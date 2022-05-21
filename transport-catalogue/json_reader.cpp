#include "json_reader.h"

using namespace renderer;
using namespace json_input;
using namespace request;
using namespace std;


pair<domain::Stop, deque<domain::Distance>>
        json_input::AdaptStopReq(const map<string, json::Node>& bus_req)
{
    string stop_name = bus_req.at("name").AsString();
    double latitude = bus_req.at("latitude").AsDouble();
    double longitude = bus_req.at("longitude").AsDouble();
    domain::Stop new_stop{ stop_name, {latitude,longitude} };
    deque<domain::Distance> new_distances;
    for (auto& dist : bus_req.at("road_distances").AsMap()) {
        domain::Distance new_distance{stop_name, dist.first, dist.second.AsInt()};
        new_distances.push_back(move(new_distance));
    }
    return make_pair(new_stop, new_distances);
}

request::RawBus json_input::AdaptBusReq(const std::map<std::string, json::Node>& bus_req)
{
    string bus_name = bus_req.at("name").AsString();
    bool is_roundtrip = bus_req.at("is_roundtrip").AsBool();
    vector<json::Node> n_stops = bus_req.at("stops").AsArray();

    deque<string> stops;
    for (auto& node : n_stops) {
        stops.push_back(node.AsString());
    }

    return request::RawBus{bus_name, is_roundtrip, stops};
}

tuple<deque<domain::Stop>, deque<domain::Distance>, deque<request::RawBus>>
                        json_input::MakeBaseReq(vector<json::Node>& base_req)
{
    deque<domain::Stop> new_stops;
    deque<domain::Distance> new_distances;
    deque<request::RawBus> new_busses;

    for (json::Node& req : base_req) {
        auto& as_map = req.AsMap();
        string_view req_type = as_map.at("type").AsString();
        if (req_type == "Stop") {
            auto adapted_req = json_input::AdaptStopReq(as_map);
            new_stops.push_back(move(adapted_req.first));
            new_distances.insert(new_distances.end(), adapted_req.second.begin(), adapted_req.second.end());
        }
        else {
            new_busses.push_back(json_input::AdaptBusReq(as_map));
        }
    }
    return make_tuple(new_stops, new_distances, new_busses);
}

deque<StatRequest> json_input::MakeStatReq(vector<json::Node>& stat_req)
{
    deque<StatRequest> result;
    for (json::Node& req : stat_req) {
        string type = req.AsMap().at("type").AsString();
        int id = req.AsMap().at("id").AsInt();

        if (type == "Route") {
            string from = req.AsMap().at("from").AsString();
            string to = req.AsMap().at("to").AsString();
            result.push_back(request::type::Route{type, id, from, to});
        } 
        else if (type != "Map") {
            string name = req.AsMap().at("name").AsString();

            result.push_back(request::type::BusOrStop{ type, id, name });
        }
        else {
            result.push_back(request::type::Map{ type, id });
        }
    }
    return result;
}

request::Requests json_input::MakeRequests(std::istream& input)
{
    json::Node node = json::Load(input).GetRoot();
    vector<json::Node> array_base_req(node.AsMap().at("base_requests").AsArray());
    vector<json::Node> array_stat_req(node.AsMap().at("stat_requests").AsArray());

    auto base_req = MakeBaseReq(array_base_req);
    auto stat_req = MakeStatReq(array_stat_req);

    std::optional<RenderSetings> rend_set;
    std::optional<RoutingSettings> route_set;

    auto set_iter = node.AsMap().find("render_settings");
    if (set_iter != node.AsMap().end()) {
        rend_set = (*set_iter).second;
    }

    set_iter = node.AsMap().find("routing_settings");
    if (set_iter != node.AsMap().end()) {
        route_set = (*set_iter).second;
    }

    return make_tuple(base_req, stat_req, rend_set, route_set);
}

json::Node json_input::AdaptToJson(std::vector<std::tuple<char, int, domain::Stat>>& statistics)
{
    json::Builder builder{};
    builder.StartArray(statistics.size());
    for (const auto& stat : statistics) {
        builder.StartDict().Key(move("request_id"s)).Value(get<1>(stat));

        char flag = get<0>(stat);
        domain::Stat variant_stat = get<2>(stat);

        if (flag == 'S') {
            const domain::StopStat* stop_stat = get<const domain::StopStat*>(variant_stat);
            if (stop_stat != nullptr) {

                builder.Key(move("buses"s)).StartArray(stop_stat->busses_.size());
                for (domain::Bus* bus : stop_stat->busses_) {
                    builder.Value(bus->name_);
                }
                builder.EndArray();
            }
            else {
                builder.Key(move("error_message"s)).Value("not found"s);
            }
        }
        else if(flag == 'B') {
            const domain::BusStat* bus_stat = get<const domain::BusStat*>(variant_stat);
            if (bus_stat != nullptr) {
                 builder.Key(move("curvature"s)).Value(bus_stat->curvature_)
                        .Key(move("route_length"s)).Value(bus_stat->route_length_)
                        .Key(move("stop_count"s)).Value(static_cast<int>(bus_stat->stops_count_))
                        .Key(move("unique_stop_count"s)).Value(static_cast<int>(bus_stat->unique_stops_count_));
            }
            else {
                builder.Key(move("error_message"s)).Value("not found"s);
            }
        }
        else if (flag == 'M') {
            builder.Key(move("map"s)).Value(get<domain::Map>(variant_stat));
        }
        else if (flag == 'R') {
            if (holds_alternative<monostate>(variant_stat)) {
                builder.Key(move("error_message"s)).Value("not found"s);
            }
            else {
                domain::Route route_stat = get<domain::Route>(variant_stat);
                builder.Key(move("total_time"s)).Value(route_stat.total_time)
                    .Key(move("items"s)).StartArray(route_stat.items.size());
                for (auto& item : route_stat.items) {
                    if (holds_alternative<domain::item::Bus>(item)) {
                        auto& bus = get<domain::item::Bus>(item);
                        builder.StartDict()
                            .Key(move("type"s)).Value("Bus"s)
                            .Key(move("bus"s)).Value(string(bus.bus_name_))
                            .Key(move("span_count"s)).Value(bus.span_count_)
                            .Key(move("time"s)).Value(bus.time_)
                            .EndDict();
                    }
                    else {
                        auto& wait = get<domain::item::Wait>(item);
                        builder.StartDict()
                            .Key(move("type"s)).Value("Wait"s)
                            .Key(move("stop_name"s)).Value(string(wait.stop_name_))
                            .Key(move("time"s)).Value(wait.time_)
                            .EndDict();
                    }
                }
                builder.EndArray();
            }
        }
        builder.EndDict();
    }
    return move(builder.EndArray().Build());
}

 void json_input::PrintInfo(std::ostream& out, vector<tuple<char, int, domain::Stat>>& stat)
 {
     json::Node node = AdaptToJson(stat);
     json::Print(node, out);
 }

 string ColorConvert(vector<json::Node> color) {
     if (color.size() == 3) {
         vector<uint8_t> rgb_color;
         rgb_color.reserve(3);
         rgb_color.push_back(color[0].AsInt());
         rgb_color.push_back(color[1].AsInt());
         rgb_color.push_back(color[2].AsInt());
         return  renderer::ColorConvertToString(rgb_color);
     }
     else {
         tuple<uint8_t, uint8_t, uint8_t, double> rgba_color{ color[0].AsInt(), color[1].AsInt(),
                                                    color[2].AsInt(), color[3].AsDouble() };
         return  renderer::ColorConvertToString(rgba_color);
     }
 }

 renderer::RenderSettings json_input::GetRenderSettings(const json::Dict& render_set)
 {
     renderer::RenderSettings result;
     
     result.width = (*render_set.find("width")).second.AsDouble();
     result.height = (*render_set.find("height")).second.AsDouble();
     result.padding = (*render_set.find("padding")).second.AsDouble();
     result.line_width = (*render_set.find("line_width")).second.AsDouble();
     result.stop_radius = (*render_set.find("stop_radius")).second.AsDouble();
     result.underlayer_width = (*render_set.find("underlayer_width")).second.AsDouble();

     result.bus_label_font_size = (*render_set.find("bus_label_font_size")).second.AsInt();
     result.stop_label_font_size = (*render_set.find("stop_label_font_size")).second.AsInt();

     auto& b_offset = (*render_set.find("bus_label_offset")).second.AsArray();
     result.bus_label_offset = make_pair(b_offset[0].AsDouble(), b_offset[1].AsDouble());

     auto& s_offset = (*render_set.find("stop_label_offset")).second.AsArray();
     result.stop_label_offset = make_pair(s_offset[0].AsDouble(), s_offset[1].AsDouble());
     
     auto& node_color = *render_set.find("underlayer_color");
     try{
         result.underlayer_color = node_color.second.AsString();
     }
     catch (const logic_error&) {
         result.underlayer_color = ColorConvert(node_color.second.AsArray());
     }

     auto& arr_color = (*render_set.find("color_palette")).second.AsArray();
     for (auto& node_color : arr_color) {
         try {
             result.color_palette.push_back(node_color.AsString());
         }
         catch (const logic_error&) {
             result.color_palette.push_back(ColorConvert(node_color.AsArray()));
         }
     }

     return result;
 }

 domain::RouteSettings json_input::GetRouteSettings(const json::Dict& node)
 {

     domain::RouteSettings result;

     result.bus_velocity = node.at("bus_velocity").AsDouble() * 1000 / 60; // m/min
     result.wait_time = node.at("bus_wait_time").AsInt();

     return result;
 }

 domain::RouteSettings GetRouteSettings(const json::Dict& node)
 {
     domain::RouteSettings result;
     result.bus_velocity = ((*node.find("bus_velocity"s)).second.AsInt());
     result.wait_time = ((*node.find("bus_wait_time"s)).second.AsInt());
     return result;
 }
