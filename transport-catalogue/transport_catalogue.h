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

class TransportCatalogue{
public:
	void AddStop(domain::Stop&& new_stop);
	void SetDist(domain::Distance&& dist);
	void AddBus(domain::Bus&& new_bus);

	domain::Bus* FindBus(std::string_view bus_name) const;
	domain::Stop* FindStop(std::string_view stop_name)const;

	
	domain::BusStat GetBusStat(const std::string& bus_name) const;
	domain::StopStat GetStopStat(const std::string& stop_name) const;

	std::deque<const domain::Bus*> GetNonemptyBusses() const;
	std::deque<const domain::BusStat*> GetNonemptyBussesStat() const;

private:
	void UpdateStat();

	std::deque<domain::Stop> stops_;
	std::unordered_map<std::string_view, domain::Stop*> stopname_to_stop_;
	std::unordered_map<std::string_view, domain::StopStat> stopname_to_stopstat_;

	std::unordered_map < std::string_view, 
		std::unordered_set < domain::Bus*, domain::BusHasher >> stopname_to_busses_;

	std::deque<domain::Bus> busses_;
	std::unordered_map<std::string_view, domain::Bus*> busname_to_bus_;
	std::unordered_map <std::string_view, domain::BusStat> busname_to_busstat_;
	std::unordered_map<std::pair<domain::Stop*, domain::Stop*>, int, domain::PairBusHasher> from_to_dist;
};