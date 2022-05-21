#pragma once
#include <deque>
#include <set>
#include <string>
#include <string_view>
#include <tuple>
#include <unordered_set>
#include <utility>
#include <variant>
#include <vector>

#include "geo.h"
#include "graph.h"

namespace domain {

using Map = std::string;

namespace route {

struct Bus
{
	std::string_view name_;
	size_t span_count_;
	double time;
};

struct Wait
{
	std::string_view stop_name_;
	size_t time_;
};

using RoutesItem = std::variant<Bus, Wait>;

struct Route
{
	double total_time_;
	std::vector<RoutesItem> items_;
};

} // route

using MaxMinLatLon = std::pair<std::pair<double, double>,
								std::pair<double, double>>;

struct Stop
{
	std::string name_;
	geo::Coordinates coordinates_;
};

struct Distance {
	std::string departure_;
	std::string destination_;
	int distance_;
};

struct StopCmp {
	bool operator()(const Stop* lhs, const Stop* rhs) const {
		return lhs->name_ < rhs->name_;
	}
};

struct Bus
{
	std::string name_{};
	bool route_is_circular_ = false;
	std::deque<domain::Stop*> stops_{};
	std::set<Stop*, StopCmp> unique_stops_{};
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
	size_t id_ = 0;
};

struct BusStat
{
	bool existing_ = false;
	std::string_view name_{};
	size_t stops_count_{};
	size_t unique_stops_count_{};
	int route_length_ = 0;
	double curvature_ = 0.0;
};

struct PairBusHasher {
	size_t operator()(const std::pair<Stop*, Stop*>& busses) const;
	std::hash<const void*> haser;
};

double Dist(std::pair<double, double> lati_long_from, std::pair<double, double> lati_long_to);

struct RouteSettings {
	size_t wait_time;
	double bus_velocity;
};

namespace item{

struct Wait {
	std::string_view stop_name_;
	int time_;
};

struct Bus {
	std::string_view bus_name_;
	int span_count_;
	double time_;
};

} // item

struct Route {
	double total_time = 0;
	std::deque<std::variant<item::Bus, item::Wait>> items;
};

using Stat = std::variant<std::monostate, const StopStat*, const BusStat*, Map, Route>;

struct StatForGraph
{
	size_t stops_count_ = 0;
	std::deque<graph::Edge<double>> short_edges_;
};

} // domain
