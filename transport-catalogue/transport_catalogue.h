#pragma once

#include <deque>
#include <set>
#include <string>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "domain.h"

namespace input {
	using NewStop = std::tuple<std::string, double, double>;
	using Distance = std::pair<std::string, std::vector<std::pair<std::string, int>>>;
	using NewBus = std::tuple<std::string, bool, std::vector<std::string>, std::string>;
}

class TransportCatalogue{
public:
	void AddStop(input::NewStop&& stop_req);
	void AddDist(input::Distance&& dist_req);
	void AddBus(input::NewBus&& bus_req);

	domain::Bus* FindBus(std::string_view bus_name) const;
	domain::Stop* FindStop(std::string_view stop_name)const;

	domain::Info GetBusInfo(std::string&& bus_name) const;
	domain::Info GetStopInfo(std::string&& stop_name) const;

	std::deque<const domain::Bus*> NonemptyBusses() const;

private:
	std::deque<domain::Stop> stops_;
	std::unordered_map<std::string_view, domain::Stop*> stopname_to_stop_;
	std::deque<domain::Bus> busses_;
	std::unordered_map<std::string_view, domain::Bus*> busname_to_bus_;
	std::unordered_map<std::pair<domain::Stop*, domain::Stop*>, int, domain::PairBusHasher> from_to_dist;
};