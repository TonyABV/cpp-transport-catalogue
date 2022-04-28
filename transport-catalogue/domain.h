#pragma once
#include <deque>
#include <set>
#include <string>
#include <string_view>
#include <tuple>
#include <unordered_set>
#include <utility>
#include <variant>

#include "geo.h"

namespace domain {

struct StopStat;
struct BusStat;
using Map = std::string;

using Stat = std::variant<std::monostate, StopStat, BusStat, Map>;

using MaxMinLatLon = std::pair<std::pair<double, double>,
								std::pair<double, double>>;

struct Stop
{
	std::string name_;
	geo::Coordinates coordinates_;
};

struct Distance {
	std::string departure_stop_;
	std::string destination_;
	int distance_;
};

struct StopCmp {
	bool operator()(const Stop* lhs, const Stop* rhs) const {
		return lhs->name_ < rhs->name_;
	}
};

struct NewBus
{
	std::string name_{};
	bool route_is_circular_ = false;
	std::vector<std::string> stops_{};
	std::string final_stop_name_;
};

struct Bus
{
	std::string name_{};
	bool route_is_circular_ = false;
	std::deque<Stop*> stops_{};
};

struct BusHasher {
	size_t operator()(const Bus* bus)const;
	std::hash<std::string_view> string_haser;
};

struct BusCmp {
	bool operator()(const Bus* lhs, const Bus* rhs) const {
		return lhs->name_ < rhs->name_;
	}
};

struct StopStat
{
	bool existing_ = false;
	std::string_view name_{};
	std::set<Bus*, BusCmp> busses_{};
};

struct BusStat
{
	bool existing_ = false;
	bool route_is_circular_ = false;
	std::deque<Stop*> stops_on_route_{};
	std::set<Stop*, StopCmp> unique_stops_{};
	std::string_view name_{};
	double straight_dist_ = 0.0;
	int route_length_ = 0;
	double curvature_ = 0.0;
	Stop* final_stop_{};
};

struct PairBusHasher {
	size_t operator()(const std::pair<Stop*, Stop*>& busses) const;
	std::hash<std::string_view> string_haser1;
	std::hash<std::string_view> string_haser2;
};

double Dist(std::pair<double, double> lati_long_from, std::pair<double, double> lati_long_to);
}
