#pragma once

#include <deque>
#include <iostream>
#include <map>
#include <string>
#include <string_view>
#include <tuple>
#include <utility>
#include <vector>

#include "json.h"
#include "map_renderer.h"
#include "request_handler.h"

namespace json_input {
	std::pair<req::NewStop, req::Distance> AdaptStopReq(const std::map < std::string, json::Node>& stop_req);
	req::NewBus AdaptBusReq(const std::map < std::string, json::Node>& bus_req);

	std::tuple<std::deque<req::NewStop>, std::deque<req::Distance>, std::deque<req::NewBus>>
		MakeBaseReq(std::vector<json::Node>& base_req);
	std::deque<req::StatRequests> MakeStatReq(std::vector<json::Node>& stat_req);

	req::Requests MakeRequests(std::istream& input);

	json::Node AdaptToJson(std::vector<std::tuple<char, int, domain::Info>>& stat);
	void PrintInfo(std::ostream& out, std::vector<std::tuple<char, int, domain::Info>>& stat);

	renderer::RenderSettings GetSettings(const json::Dict& render_set);
}