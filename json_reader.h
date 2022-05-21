#pragma once

#include <deque>
#include <iostream>
#include <map>
#include <unordered_map>
#include <string>
#include <string_view>
#include <tuple>
#include <utility>
#include <vector>

#include "domain.h"
#include "json.h"
#include "json_builder.h"
#include "map_renderer.h"
#include "request_handler.h"

namespace json_input {

std::pair<domain::Stop, std::deque<domain::Distance>> 
	AdaptStopReq(const std::map < std::string, json::Node>& stop_req);
request::RawBus AdaptBusReq(const std::map < std::string, json::Node>& bus_req);

std::tuple<std::deque<domain::Stop>, std::deque<domain::Distance>, std::deque<request::RawBus>>
	MakeBaseReq(std::vector<json::Node>& base_req);

std::deque<request::StatRequest> MakeStatReq(std::vector<json::Node>& stat_req);

request::Requests MakeRequests(std::istream& input);

json::Node AdaptToJson(std::vector<std::tuple<char, int, domain::Stat>>& stats);
void PrintInfo(std::ostream& out, std::vector<std::tuple<char, int, domain::Stat>>& stat);

renderer::RenderSettings GetRenderSettings(const json::Dict& render_set);

domain::RouteSettings GetRouteSettings(const json::Dict& node);

}