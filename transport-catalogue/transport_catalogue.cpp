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
	Stop* stop_from = FindStop(dist.departure_stop_);
	Stop* stop_to = FindStop(dist.destination_);
	pair from_to = make_pair(stop_from, stop_to);
	from_to_dist[from_to] = dist.distance_;
}

void TransportCatalogue::AddBus(domain::NewBus&& new_bus)
{
	Bus n_bus{ new_bus.name_, new_bus.route_is_circular_, {} };
	BusStat bus_stat;

	busses_.push_back(move(n_bus));
	auto& bus = busses_.back();/**/

	bus_stat.existing_ = true;
	bus_stat.route_is_circular_ = bus.route_is_circular_;
	bus_stat.name_ = bus.name_;
	bus_stat.final_stop_ = FindStop(new_bus.final_stop_name_);

	deque<Stop*> stops;
	set<Stop*, StopCmp> uniq_stops;

	double straight_dist = 0;
	int route_length = 0;

	string& firststop_name = new_bus.stops_[0];
	Stop* firststop_ptr = FindStop(firststop_name);

	uniq_stops.insert(FindStop(firststop_ptr->name_));
	stops.push_back(firststop_ptr);

	auto stop_stat_it = stopname_to_stopstat_.find(firststop_name);
	(*stop_stat_it).second.busses_.insert(&bus);

	pair<double, double> from{ firststop_ptr->coordinates_.lat, firststop_ptr->coordinates_.lng };

	Stop* stop_from = firststop_ptr;
	for (size_t n = 1; n < new_bus.stops_.size(); ++n)
	{
		string& stop_name = new_bus.stops_[n];
		Stop* stop_to = FindStop(stop_name);

		uniq_stops.insert(FindStop(stop_to->name_));
		stops.push_back(stop_to);
		auto stop_stat_it = stopname_to_stopstat_.find(stop_name);
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

	bus_stat.straight_dist_ = straight_dist;
	bus_stat.route_length_ = route_length;
	bus_stat.curvature_ = route_length / straight_dist;
	bus_stat.unique_stops_ = move(uniq_stops);
	bus_stat.stops_on_route_ = stops;

	bus.stops_ = move(stops);
	busname_to_bus_[bus.name_] = &bus;
	busname_to_busstat_[bus.name_] = bus_stat;
	/*bus.straight_dist_ = straight_dist;
	bus.route_length_ = route_length;
	bus.curvature_ = route_length / straight_dist;
	bus.unique_stops_ = move(uniq_stops);*/
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
		domain::BusStat busstat;
		busstat.name_ = bus_name;
		return busstat;
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

deque<const domain::BusStat*> TransportCatalogue::GetNonemptyBussesStat() const
{
	deque<const domain::BusStat*> result;
	for (auto& bus_ref : busname_to_busstat_) {
		if (!bus_ref.second.stops_on_route_.empty()) {
			result.push_back(&(bus_ref.second));
		}
	}
	return result;
}
