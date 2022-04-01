#pragma once

#include <deque>
#include <set>
#include <string>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "input_reader.h"

struct Stop
{
	std::string name;
	double latitude;
	double longitude;
	std::set<std::string_view> bus_names;
};

struct Bus
{
	bool route_is_circular_;
	std::string name_;
	std::deque<Stop*> stops_;
	std::unordered_set<std::string_view> unique_stops_;
	double straight_dist_;
	int route_length_;
	double curvature_;
};

struct Info
{
	bool existing_ = false;

	std::string name_;

	size_t stops_on_route_ = 0;

	size_t unique_stops_ = 0;

	double straight_dist_ = 0;

	int route_length_ = 0;

	double curvature_ = 0.0;

	std::set<std::string_view> busses_to_stop_;
};

class TransportCatalogue{
public:
	//TransportCatalogue(InputRequests&& requests);

	void AddStop(input::NewStop&& stop_req);
	void AddDist(input::Distance&& dist_req);
	void AddBus(input::NewBus&& bus_req);

	Bus* FindBus(std::string_view bus_name);
	Stop* FindStop(std::string_view stop_name);

	//size_t GetUniqs(const Bus* bus);
	Info GetBusInfo(std::string&& bus_name);
	Info GetStopInfo(std::string&& stop_name);

private:
	std::deque<Stop> stops_;
	std::unordered_map<std::string_view, Stop*> stopname_to_stop_;
	std::deque<Bus> busses_;
	std::unordered_map<std::string_view, Bus*> busname_to_bus_;

	struct PairBusHasher {
		size_t operator()(const std::pair<Stop*, Stop*>& busses) const {
			return hasher1(busses.first->name) + 42 * hasher2(busses.second->name);
		}
		std::hash<std::string_view> hasher1;
		std::hash<std::string_view> hasher2;
	};

	std::unordered_map<std::pair<Stop*, Stop*>, int, PairBusHasher> from_to_dist;
	//std::unordered_map<std::string_view, std::unordered_set<std::string_view>> stopname_tp_busnames_;
};