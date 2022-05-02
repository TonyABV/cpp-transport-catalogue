#include <algorithm>
#include <stdexcept>

#include "transport_catalogue.h"

using namespace std;
using namespace domain;

void TransportCatalogue::AddStop(domain::Stop&& new_stop)
{
	StopStat stop_stat;

	stops_.push_back(move(new_stop));
	stopname_to_stop_[stops_.back().name_] = &stops_.back();

	stop_stat.name_ = stops_.back().name_;
	stop_stat.existing_ = true;
	stopname_to_stopstat_[stops_.back().name_] = move(stop_stat);
}

void TransportCatalogue::SetDist(domain::Distance&& dist)
{
	Stop* stop_from = FindStop(dist.departure_);
	Stop* stop_to = FindStop(dist.destination_);
	pair from_to = make_pair(stop_from, stop_to);
	from_to_dist[from_to] = dist.distance_;
}

void TransportCatalogue::UpdateStat()
{
	auto& bus = busses_.back();

	BusStat bus_stat;

	bus_stat.name_ = bus.name_;
	bus_stat.existing_ = true;

	double straight_dist = 0;
	int route_length = 0;

	Stop* stop_from = bus.stops_[0];

	auto stop_stat_it = stopname_to_stopstat_.find(stop_from->name_);
	(*stop_stat_it).second.busses_.insert(&bus);

	pair<double, double> from{ stop_from->coordinates_.lat, stop_from->coordinates_.lng };

	for (size_t n = 1; n < bus.stops_.size(); ++n)
	{
		Stop* stop_to = bus.stops_[n];
		auto stop_stat_it = stopname_to_stopstat_.find(stop_to->name_);
		(*stop_stat_it).second.busses_.insert(&bus);

		pair<double, double> to{ stop_to->coordinates_.lat, stop_to->coordinates_.lng };
		straight_dist += domain::Dist(from, to);
		from = to;

		pair from_to = make_pair(stop_from, stop_to);
		auto dist_iter = from_to_dist.find(from_to);
		if (dist_iter == from_to_dist.end()) {
			from_to = make_pair(stop_to, stop_from);
			dist_iter = from_to_dist.find(from_to);
			if (dist_iter == from_to_dist.end()) {
				stop_to = stop_from;
				continue;
			}
		}
		int dist = (*dist_iter).second;
		route_length = route_length + dist;
		stop_from = stop_to;
	}

	bus_stat.route_length_ = route_length;
	bus_stat.curvature_ = route_length / straight_dist;
	bus_stat.unique_stops_count_ = bus.unique_stops_.size();
	bus_stat.stops_count_ = bus.stops_.size();

	busname_to_bus_[bus.name_] = &bus;
	busname_to_busstat_[bus.name_] = bus_stat;
}

void TransportCatalogue::AddBus(domain::Bus&& new_bus)
{
	if (!new_bus.route_is_circular_) {
		size_t old_size = new_bus.stops_.size();
		for (size_t n = 1; n < old_size; ++n) {
			new_bus.stops_.push_back(new_bus.stops_[old_size - 1 - n]);
		}
	}

	busses_.push_back(move(new_bus));
	UpdateStat();
}

Bus* TransportCatalogue::FindBus(string_view bus_name) const
{
	try
	{
		Bus* result = busname_to_bus_.at(bus_name);
		return result;
	}
	catch (const std::out_of_range&)
	{
		return nullptr;
	}
}

Stop* TransportCatalogue::FindStop(string_view stop_name) const
{
	try
	{
		Stop* result = stopname_to_stop_.at(stop_name);
		return result;
	}
	catch (const std::out_of_range&)
	{
		return nullptr;
	}
}

domain::BusStat TransportCatalogue::GetBusStat(const string& bus_name) const
{
	auto busstat_it = busname_to_busstat_.find(bus_name);
	if (busstat_it == busname_to_busstat_.end()) {
		
		return domain::BusStat{};
	}
	return (*busstat_it).second;
}

domain::StopStat TransportCatalogue::GetStopStat(const std::string& stop_name) const
{
	auto stopstat_it = stopname_to_stopstat_.find(stop_name);
	if (stopstat_it == stopname_to_stopstat_.end()) {
		domain::StopStat stopstat;
		stopstat.name_ = stop_name;
		return stopstat;
	}
	return (*stopstat_it).second;
}

std::deque<const domain::Bus*> TransportCatalogue::GetNonemptyBusses() const
{
	deque<const domain::Bus*> result;
	for (auto& bus_ref : busses_) {
		if (!bus_ref.stops_.empty()) {
			result.push_back(&(bus_ref));
		}
	}
	return result;
}

deque<const domain::BusStat*> TransportCatalogue::GetNonemptyBussesStat() const
{
	deque<const domain::BusStat*> result;
	for (auto& bus_ref : busname_to_busstat_) {
		if (!bus_ref.second.stops_count_ == 0) {
			result.push_back(&(bus_ref.second));
		}
	}
	return result;
}
