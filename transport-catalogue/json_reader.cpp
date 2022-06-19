#include "json_reader.h"
#include "json_builder.h"
#include "svg.h"
#include "serialization.h"

namespace json_reader {

    using namespace std::literals;

    namespace detail {

        std::map<std::string, int> DistBtwStops(const json::Dict& dic) {
            std::map<std::string, int> res;
            for (const auto& elem : dic) {
                res[elem.first] = elem.second.AsInt();
            }
            return res;
        }

        RequestStop BaseStop(const json::Dict& dic)
        {
            RequestStop stop;
            stop.e_type = RequestType::BASE;
            stop.type = request_type::STOP;
            stop.name = dic.at("name"s).AsString();
            stop.latitude = dic.at("latitude"s).AsDouble();
            stop.longitude = dic.at("longitude"s).AsDouble();
            if (dic.count("road_distances"s)) {
                // прочитать road_distances как map, а затем преобразовать Node в эту map
                stop.road_distances = std::move(DistBtwStops(dic.at("road_distances"s).AsMap()));
            }
            return stop;
        }

        std::vector<std::string> StopsForBus(const json::Array& arr) {
            std::vector<std::string> res;
            for (const auto& elem : arr) {
                res.emplace_back(elem.AsString());
            }
            return res;
        }

        RequestBus BaseBus(const json::Dict& dic) {
            RequestBus bus;
            bus.e_type = RequestType::BASE;
            bus.type = request_type::BUS;
            bus.name = dic.at("name"s).AsString();
            bus.stops = std::move(StopsForBus(dic.at("stops"s).AsArray()));
            bus.is_roundtrip = dic.at("is_roundtrip"s).AsBool();
            return bus;
        }

        RequestStat Stat(const json::Dict& dic) {
            RequestStat stat;
            stat.e_type = RequestType::STAT;
            stat.id = dic.at("id"s).AsInt();
            if (dic.at("type"s).AsString() == "Bus"s) {
                stat.type = request_type::BUS;
                stat.name = dic.at("name"s).AsString();
            }
            else if (dic.at("type"s).AsString() == "Stop"s) {
                stat.type = request_type::STOP;
                stat.name = dic.at("name"s).AsString();
            }
            else if (dic.at("type"s).AsString() == "Map"s) {
                stat.type = request_type::MAP;
            }
            else if (dic.at("type"s).AsString() == "Route"s) {
                stat.type = request_type::ROUTE;
                stat.route.from = dic.at("from"s).AsString();
                stat.route.to = dic.at("to"s).AsString();
            }
            return stat;
        }

        svg::Color ColorFromNode(const json::Node& node) {
            if (node.IsArray()) {//содержит rgb/rgba
                if (node.AsArray().size() == 3) {
                    svg::Rgb rgb;
                    rgb.red = node.AsArray()[0].AsInt();
                    rgb.green = node.AsArray()[1].AsInt();
                    rgb.blue = node.AsArray()[2].AsInt();
                    return rgb;
                }
                else {
                    svg::Rgba rgba;
                    rgba.red = node.AsArray()[0].AsInt();
                    rgba.green = node.AsArray()[1].AsInt();
                    rgba.blue = node.AsArray()[2].AsInt();
                    rgba.opacity = node.AsArray()[3].AsDouble();
                    return rgba;
                }
            }
            else {//содержит string
                return node.AsString();
            }
        }

        transport_db::RenderSettings RenderMap(const json::Dict& dic) {
            using namespace detail;
            transport_db::RenderSettings res;
            res.width = dic.at("width"s).AsDouble();
            res.height = dic.at("height"s).AsDouble();
            res.padding = dic.at("padding"s).AsDouble();
            res.line_width = dic.at("line_width"s).AsDouble();
            res.stop_radius = dic.at("stop_radius"s).AsDouble();
            res.bus_label_font_size = dic.at("bus_label_font_size"s).AsInt();
            res.bus_label_offset[0] = dic.at("bus_label_offset"s).AsArray()[0].AsDouble();
            res.bus_label_offset[1] = dic.at("bus_label_offset"s).AsArray()[1].AsDouble();
            res.stop_label_font_size = dic.at("stop_label_font_size"s).AsInt();
            res.stop_label_offset[0] = dic.at("stop_label_offset"s).AsArray()[0].AsDouble();
            res.stop_label_offset[1] = dic.at("stop_label_offset"s).AsArray()[1].AsDouble();
            res.underlayer_color = ColorFromNode(dic.at("underlayer_color"s));
            res.underlayer_width = dic.at("underlayer_width"s).AsDouble();
            for (const auto& node : dic.at("color_palette"s).AsArray()) {
                res.color_palette.emplace_back(ColorFromNode(node));
            }
            return res;
        } 
    }

    transport_db::RouterSettings RouterMap(const json::Dict& dic) {
        using namespace detail;
        transport_db::RouterSettings res;
        res.bus_velocity_kmh = dic.at("bus_velocity"s).AsInt();
        res.bus_wait_time = dic.at("bus_wait_time"s).AsInt();
        return res;
    }

    serialization::SerializationSettings SerializationCatalogue(const json::Dict& dic) {           
        serialization::SerializationSettings res;
        res.file_name = dic.at("file"s).AsString();
        return res;
    }

    using namespace detail;
    void JsonReader::ReadInput(std::istream& input) {
        FillDoc(input);
        //заполнить все запросы
        for (const auto& elem : document_.GetRoot().AsMap()) {
            if (elem.first == "base_requests"s) {
                FillBase(elem.second.AsArray());
            }
            else if (elem.first == "stat_requests"s) {
                FillStat(elem.second.AsArray());
            }
            else if (elem.first == "render_settings"s) {
                FillRender(elem.second.AsMap());
            }
            else if (elem.first == "routing_settings"s) {
                FillRouting(elem.second.AsMap());
            }
            else if (elem.first == "serialization_settings"s) {
                FillSerialization(elem.second.AsMap());
            }
        }
    }

    void JsonReader::FillCatalogue() {
        using RoadDistances = std::map<std::string, int>;
        std::vector<std::pair<std::string, RoadDistances>> stops_w_dist; //вектор всех расстояний между остановками
        // 1-й этап. Добавить остановки
        for (const auto& elem : requests_) {
            if (RequestStop* s = dynamic_cast<RequestStop*>(elem.get())) {
                catalogue_.AddStop(s->name, { s->latitude, s->longitude });
                if (!s->road_distances.empty()) {
                    stops_w_dist.emplace_back(std::make_pair(s->name, s->road_distances));
                }
            }
        }
        // 2-й этап. Добавить автобусы
        for (const auto& elem : requests_) {
            if (RequestBus* b = dynamic_cast<RequestBus*>(elem.get())) {
                std::vector <std::string> string_stops = std::move(b->stops);
                std::vector<std::string_view> stops;
                for (const auto& stop : string_stops) {
                    stops.emplace_back(stop);
                }
                std::string_view end_stop = stops.back();
                if (!b->is_roundtrip) {
                    stops.reserve(2 * stops.size());
                    stops.insert(stops.end(), next(stops.rbegin()), stops.rend()); // все остановки маршрута добавляются перед добавлением в каталог
                }
                catalogue_.AddRoute(b->name, stops, b->is_roundtrip, end_stop); // Установить из запроса имя end_stop для обнаружения условия с неправильным вводом b->is_roundtrip
            }
        }
        // 3-й этап. Добавить расстояния между остановками
        for (auto& [main_stop, des_stops] : stops_w_dist) {
            for (auto& [des_stop, dist] : des_stops) {
                catalogue_.SetDistBtwStops(main_stop, des_stop, dist);
            }
        }
    }

    void JsonReader::PrintRequests(std::ostream& out, transport_router::RequestHandler& request_handler) {
        out << "["s << std::endl;
        bool first = true;
        for (const auto& elem : requests_) {
            if (json_reader::detail::RequestStat* s = dynamic_cast<json_reader::detail::RequestStat*>(elem.get())) {
                if (!first) {
                    out << ',' << std::endl;
                }
                if (s->type == request_type::STOP) {
                    Builder request{};
                    request.StartDict().Key("request_id"s).Value(s->id);
                    ProcessStopStatRequest(catalogue_.GetStop(s->name), request);
                    request.EndDict();
                    Print(Document{ request.Build() }, out);
                }
                else if (s->type == request_type::BUS) {
                    Builder request{};
                    request.StartDict().Key("request_id"s).Value(s->id);
                    ProcessBusStatRequest(catalogue_.GetRoute(s->name), request);
                    request.EndDict();
                    Print(Document{ request.Build() }, out);
                }
                else if (s->type == request_type::MAP) {
                    Builder request{};
                    request.StartDict().Key("request_id"s).Value(s->id);
                    request_handler.SetCatalogueDataToRender();
                    std::stringstream strm;
                    renderer_.Render(strm);
                    request.Key("map"s).Value(strm.str());
                    request.EndDict();
                    Print(Document{ request.Build() }, out);
                }
                else if (s->type == request_type::ROUTE) {
                    Builder request{};
                    const std::vector<transport_router::Edges>* edges_data = router_.GetEdgesData();
                    auto route_data = router_.GetRoute(s->route.from, s->route.to);
                    request.StartDict().Key("request_id"s).Value(s->id);
                    if (route_data && route_data->edges.size() > 0) {// маршрут сгенерирован
                        request.Key("total_time"s).Value(route_data->weight)
                            .Key("items")
                            .StartArray();
                        for (size_t edge_id : route_data->edges) {
                            std::string name{ edges_data->at(edge_id).name };
                            if (edges_data->at(edge_id).type == transport_router::edge_type::WAIT) {// ребро графа принадлежит stop
                                request.StartDict()
                                    .Key("stop_name"s).Value(name)
                                    .Key("time"s).Value(edges_data->at(edge_id).time)
                                    .Key("type"s).Value("Wait"s)
                                    .EndDict();
                            }
                            else {// ребро графа принадлежит bus
                                request.StartDict()
                                    .Key("bus"s).Value(name)
                                    .Key("time"s).Value(edges_data->at(edge_id).time)
                                    .Key("type"s).Value("Bus"s)
                                    .Key("span_count"s).Value(static_cast<int>(edges_data->at(edge_id).span_count))
                                    .EndDict();
                            }
                        }
                        request.EndArray();
                    }
                    else if (!route_data) {// не удалось создать маршрут
                        request.Key("error_message"s).Value("not found"s);
                    }
                    else {// остановка FROM = остановка TO
                        request.Key("total_time"s).Value(0)
                            .Key("items")
                            .StartArray()
                            .EndArray();
                    }
                    request.EndDict();
                    Print(Document{ request.Build() }, out);
                }
                first = false;
            }
        }
        out << std::endl << "]"s << std::endl;
    }

    // наполнить json-документ
    void JsonReader::FillDoc(std::istream& strm) {
        document_ = json::Load(strm);
    }

    // наполнить add-запросы
    void JsonReader::FillBase(const std::vector <Node>& vec) {
        for (const auto& elem : vec) {
            if (elem.AsMap().count("type"s)) {
                if (elem.AsMap().at("type"s).AsString() == "Stop"s) {
                    requests_.emplace_back(std::make_unique<RequestStop>(detail::BaseStop(elem.AsMap()))); // наполнить stop
                }
                else if (elem.AsMap().at("type"s).AsString() == "Bus"s) {
                    requests_.emplace_back(std::make_unique<RequestBus>(detail::BaseBus(elem.AsMap()))); // наполнить bus
                }
            }
        }
    }

    // наполнить get-запросы (статистику)
    void JsonReader::FillStat(const std::vector<Node>& vec) {
        for (const auto& elem : vec) {
            if (elem.AsMap().count("type"s)) {
                if (elem.AsMap().at("type"s).AsString() == "Bus"s
                    || elem.AsMap().at("type"s).AsString() == "Stop"s
                    || elem.AsMap().at("type"s).AsString() == "Map"s
                    || elem.AsMap().at("type"s).AsString() == "Route"s
                    ) {
                    requests_.emplace_back(std::make_unique<RequestStat>(detail::Stat(elem.AsMap())));
                }
            }
        }
    }

    // наполнить add-запрос настроек маршрутизации
    void JsonReader::FillRouting(const std::map<std::string, Node>& dic) {
        router_.SetSettings(RouterMap(dic));
    }

    void JsonReader::FillSerialization(const std::map<std::string, Node>& dic) { 
        serialization_.SetSettings(SerializationCatalogue(dic));
    }

    // наполнить dict данными bus
    void JsonReader::ProcessStopStatRequest(const transport_db::TransportCatalogue::StopOutput& request, Builder& dict) {
        auto [X, buses] = request;
        if (X[0] == '!') {
            dict.Key("error_message"s).Value("not found"s);
            return;
        }
        if (buses.size() == 0) {
            dict.Key("buses"s).StartArray().EndArray();
        }
        else {
            dict.Key("buses"s).StartArray();
            for (auto& bus : buses) {
                std::string s_bus(bus);
                dict.Value(std::move(s_bus));
            }
            dict.EndArray();
        }
    }

    // наполнить dict данными Stop
    void JsonReader::ProcessBusStatRequest(const transport_db::TransportCatalogue::RouteOutput& request, Builder& dict) {
        auto [X, R, U, L, C] = request;
        if (X[0] != '!') {
            dict.Key("curvature"s).Value(C)
                .Key("route_length"s).Value(L)
                .Key("stop_count"s).Value(static_cast<int>(R))
                .Key("unique_stop_count"s).Value(static_cast<int>(U));
        }
        else {
            dict.Key("error_message"s).Value("not found"s);
        }
    }

    // наполнить add-запрос настроек Render
    void JsonReader::FillRender(const std::map<std::string, Node>& dic) {
        renderer_.SetSettings(RenderMap(dic));
    }
}// namespace json_reader