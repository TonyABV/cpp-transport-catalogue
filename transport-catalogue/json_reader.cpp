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

domain::NewBus json_input::AdaptBusReq(const std::map<std::string, json::Node>& bus_req)
{
    string bus_name = bus_req.at("name").AsString();
    bool is_roundtrip = bus_req.at("is_roundtrip").AsBool();
    vector<json::Node> n_stops = bus_req.at("stops").AsArray();

    vector<string> stops;
    for (auto& node : n_stops) {
        stops.push_back(node.AsString());
    }

    std::string final_stop = stops.back();

    if (!is_roundtrip) {
        size_t old_size = stops.size();
        stops.reserve(old_size * 2 - 1);
        for (size_t n = 1; n < old_size; ++n) {
            stops.push_back(stops[old_size - 1 - n]);
        }
    }

    return domain::NewBus{bus_name, is_roundtrip, stops, final_stop};
}

tuple<deque<domain::Stop>, deque<domain::Distance>, deque<domain::NewBus>>
                        json_input::MakeBaseReq(vector<json::Node>& base_req)
{
    deque<domain::Stop> new_stops;
    deque<domain::Distance> new_distances;
    deque<domain::NewBus> new_busses;

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

deque<StatRequests> json_input::MakeStatReq(vector<json::Node>& stat_req)
{
    deque<StatRequests> result;
    for (json::Node& req : stat_req) {
        string type = req.AsMap().at("type").AsString();
        int id = req.AsMap().at("id").AsInt();
        if (type != "Map") {
            string name = req.AsMap().at("name").AsString();
            result.push_back(make_tuple(type, id, name));
        }
        else {
            result.push_back(make_tuple(type, id, ""));
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
    return make_tuple(base_req, stat_req, node.AsMap().at("render_settings"));
}

 json::Node json_input::AdaptToJson(std::vector<std::tuple<char, int, domain::Stat>>& stat)
{
    json::Array arr;
    for (const auto& st : stat) {
        json::Dict dict;

        dict["request_id"] = json::Node(get<1>(st));
        char flag = get<0>(st);
        domain::Stat stat = get<2>(st);

        if (flag == 'S') {
            if (get<domain::StopStat>(stat).existing_) {
                json::Array busses;
                for (domain::Bus* bus : get<domain::StopStat>(stat).busses_) {
                    busses.push_back(json::Node(bus->name_));
                }
                dict["buses"] = json::Node(busses);
            }
            else {
                dict["error_message"] = json::Node("not found"s);
            }
        }
        else if(flag == 'B') {
            if (get<domain::BusStat>(stat).existing_) {
                dict["curvature"] = json::Node(get<domain::BusStat>(stat).curvature_);
                dict["route_length"] = json::Node(get<domain::BusStat>(stat).route_length_);
                dict["stop_count"] = json::Node(static_cast<int>(get<domain::BusStat>(stat).stops_on_route_.size()));
                dict["unique_stop_count"] = json::Node(static_cast<int>(get<domain::BusStat>(stat).unique_stops_.size()));/**/
            }
            else {
                dict["error_message"] = json::Node("not found"s);
            }
        }
        else if (flag == 'M') {
            dict["map"] = json::Node(get<domain::Map>(stat));
        }
        arr.push_back(dict);
    }
    return json::Node(arr);
}

 void json_input::PrintInfo(std::ostream& out, vector<tuple<char, int, domain::Stat>>& stat)
 {
     json::Node node = AdaptToJson(stat);
     json::Print(json::Document(node), out);
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

 renderer::RenderSettings json_input::GetSettings(const json::Dict& render_set)
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
