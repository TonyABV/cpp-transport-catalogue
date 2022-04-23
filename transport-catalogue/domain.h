#pragma once
#include <deque>
#include <set>
#include <string>
#include <string_view>
#include <tuple>
#include <unordered_set>
#include <utility>

#include "geo.h"

namespace domain {

struct Bus;

using MaxMinLatLon = std::pair<std::pair<double, double>,
								std::pair<double, double>>;

struct Info
{
	bool existing_ = false;
	std::string name_;
	int stops_on_route_ = 0;
	int unique_stops_ = 0;
	double straight_dist_ = 0;
	int route_length_ = 0;
	double curvature_ = 0.0;
	std::set<std::string> busses_to_stop_;
	std::string map_;
};

struct Stop
{
	std::string name_;
	double latitude_;
	double longitude_;
	std::set<std::string> bus_names_;
};

struct StopCmp {
	bool operator()(const Stop* lhs, const Stop* rhs) const {
		return lhs->name_ < rhs->name_;
	}
};

struct Bus
{
	std::string name_;
	bool route_is_circular_;
	std::deque<Stop*> stops_{};
	std::set<Stop*, StopCmp> unique_stops_{};
	double straight_dist_ = 0.0;
	int route_length_ = 0;
	double curvature_ = 0.0;
	Stop* final_stop_{};
};

Info BusInfo(Bus* bus_ptr);
Info StopInfo(Stop* bus_ptr);

struct PairBusHasher {
	size_t operator()(const std::pair<Stop*, Stop*>& busses) const;
	std::hash<std::string_view> hasher1;
	std::hash<std::string_view> hasher2;
};

double Dist(std::pair<double, double> lati_long_from, std::pair<double, double> lati_long_to);
}
