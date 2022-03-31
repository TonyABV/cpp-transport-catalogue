#pragma once
#include <iostream>
#include <istream>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

namespace input{

	using NewStop = std::tuple<std::string, double, double>;
	using Distance = std::pair<std::string, std::vector<std::pair<std::string, int>>>;
	using NewBus = std::tuple<std::string, bool, std::vector<std::string>>;

struct Requests
{
	std::vector<NewStop> stop_requests_;
	std::vector<Distance> distance_requests_;
	std::vector<NewBus> bus_requests_;
};

Requests MakeRequests(std::istream& input);

std::pair<NewStop, Distance> MakeStopRequest(std::string&& text);

NewBus MakeBusRequest(std::string&& text);

std::pair<size_t, size_t> FindNamePos(std::string_view text, 
				size_t pos_find_begin = 0, size_t pos_find_end = std::string::npos);
}