using namespace renderer;
using namespace json_input;
using namespace req;
using namespace std;

#include "json_reader.h"


pair<NewStop, Distance> json_input::AdaptStopReq(const map<string, json::Node>& bus_req)
{
    string stop_name = bus_req.at("name").AsString();
    NewStop stop = make_tuple(stop_name, bus_req.at("latitude").AsDouble(),
        bus_req.at("longitude").AsDouble());

    vector<pair<string, int>> to_stop_dist;
    for (auto& dist : bus_req.at("road_distances").AsMap()) {
        to_stop_dist.push_back(make_pair(dist.first, dist.second.AsInt()));
    }
    Distance dist = make_pair(stop_name, to_stop_dist);
    return make_pair(stop, dist);
}

NewBus json_input::AdaptBusReq(const std::map<std::string, json::Node>& bus_req)
{
    NewBus bus;
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
    return make_tuple(bus_name, is_roundtrip, stops, final_stop);
}

tuple<deque<NewStop>, deque<Distance>, deque<NewBus>> json_input::MakeBaseReq(vector<json::Node>& base_req)
{
    deque<NewStop> stop_req;
    deque<Distance> dist_req;
    deque<NewBus> bus_req;

    for (json::Node& req : base_req) {
        auto& as_map = req.AsMap();
        string_view req_type = as_map.at("type").AsString();
        if (req_type == "Stop") {
            auto adapted_req = AdaptStopReq(as_map);
            stop_req.push_back(adapted_req.first);
            dist_req.push_back(adapted_req.second);
        }
        else {
            bus_req.push_back(AdaptBusReq(as_map));
        }
    }
    return make_tuple(stop_req, dist_req, bus_req);
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

Requests json_input::MakeRequests(std::istream& input)
{
    json::Node node = json::Load(input).GetRoot();
    vector<json::Node> array_base_req(node.AsMap().at("base_requests").AsArray());
    vector<json::Node> array_stat_req(node.AsMap().at("stat_requests").AsArray());

    auto base_req = MakeBaseReq(array_base_req);
    auto stat_req = MakeStatReq(array_stat_req);
    return make_tuple(base_req, stat_req, node.AsMap().at("render_settings"));
}



 json::Node json_input::AdaptToJson(std::vector<std::tuple<char, int, domain::Info>>& stat)
{
    json::Array arr;
    for (auto& st : stat) {
        json::Dict dict;
        dict["request_id"] = json::Node(get<1>(st));
        domain::Info info = get<2>(st);
        if (get<0>(st) == 'S') {
            if (info.existing_) {
                json::Array busses;
                for (string stop : info.busses_to_stop_) {
                    busses.push_back(json::Node(stop));
                }
                dict["buses"] = json::Node(busses);
            }
            else {
                dict["error_message"] = json::Node("not found"s);
            }
        }
        else if(get<0>(st) == 'B') {
            if (info.existing_) {
                dict["curvature"] = json::Node(info.curvature_);
                dict["route_length"] = json::Node(info.route_length_);
                dict["stop_count"] = json::Node(info.stops_on_route_);
                dict["unique_stop_count"] = json::Node(info.unique_stops_);/**/
            }
            else {
                dict["error_message"] = json::Node("not found"s);
            }
        }
        else if (get<0>(st) == 'M') {
            dict["map"] = json::Node(info.map_);
        }
        arr.push_back(dict);
    }
    return json::Node(arr);
}

 void json_input::PrintInfo(std::ostream& out, vector<tuple<char, int, domain::Info>>& stat)
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
         return ColorConvertToString(rgb_color);
     }
     else {
         tuple<uint8_t, uint8_t, uint8_t, double> rgba_color{ color[0].AsInt(), color[1].AsInt(),
                                                    color[2].AsInt(), color[3].AsDouble() };
         return ColorConvertToString(rgba_color);
     }
 }

 RenderSettings json_input::GetSettings(const json::Dict& render_set)
 {
     RenderSettings result;
     
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
