#include <algorithm>
#include <stdexcept>

#include <iostream>

#include "transport_catalogue.h"

using namespace std;
using namespace domain;


void TransportCatalogue::UpdateStopStat()
{
	stopname_to_stop_[stops_.back().name_] = &stops_.back();

	id_to_stop_.push_back(&stops_.back());

	++graph_stat_.stops_count_;

	StopStat stop_stat;

	stop_stat.id_ = stop_id_counter++;

	stop_stat.name_ = stops_.back().name_;
	stop_stat.existing_ = true;
	stopname_to_stopstat_[stops_.back().name_] = move(stop_stat);
}

void TransportCatalogue::AddStop(domain::Stop&& new_stop)
{
	stops_.push_back(move(new_stop));

	UpdateStopStat();
}

void TransportCatalogue::SetDist(domain::Distance&& dist)
{
	Stop* stop_from = FindStop(dist.departure_);
	Stop* stop_to = FindStop(dist.destination_);
	pair from_to = make_pair(stop_from, stop_to);
	from_to_dist[from_to] = dist.distance_;
}

double TransportCatalogue::ComputeStraightDist(const Stop* stop_from, const Stop* stop_to) const
{
	pair<double, double> from{ stop_from->coordinates_.lat, stop_from->coordinates_.lng };
	pair<double, double> to{ stop_to->coordinates_.lat, stop_to->coordinates_.lng };
	return domain::Dist(from, to);
}

// Update stop stat, create edges,  return: route length, straight_dist.
std::pair<int, double> TransportCatalogue::DoLoopByStops(domain::Bus& bus)
{
	int route_length = 0;
	double straight_dist = 0;

	Stop* stop_from = bus.stops_[0];

	auto stop_stat_it = stopname_to_stopstat_.find(stop_from->name_);
	(*stop_stat_it).second.busses_.insert(&bus);

	size_t stops_count = bus.stops_.size();

	for (size_t n = 1; n < bus.stops_.size(); ++n)
	{
		Stop* stop_to = bus.stops_[n];

		straight_dist += ComputeStraightDist(stop_from, stop_to);

		auto stop_stat_it = stopname_to_stopstat_.find(stop_to->name_);
		(*stop_stat_it).second.busses_.insert(&bus);

		pair from_to = make_pair(stop_from, stop_to);

		int dist = GetDist(from_to);
		route_length = route_length + dist;

		double forward_weight(dist);
		double reverse_weight = 0.0;

		const domain::StopStat* first_stop_stat = GetStopStat(stop_from->name_);
		const domain::StopStat* second_stop_stat = GetStopStat(stop_to->name_);

		if (first_stop_stat->id_ != second_stop_stat->id_)
		{
			graph_stat_.short_edges_.push_back(move(graph::Edge<double> 
													{ bus.name_, first_stop_stat->id_, second_stop_stat->id_, 1, forward_weight }));

			if (!bus.route_is_circular_) {
				std::pair<domain::Stop*, domain::Stop*> reverse_copy(from_to.second, from_to.first);
				dist = GetDist(reverse_copy);
				reverse_weight = dist;
				graph_stat_.short_edges_.push_back(move(graph::Edge<double>
														{ bus.name_, second_stop_stat->id_, first_stop_stat->id_, 1, reverse_weight }));
			}
		}

		size_t spane_count = 2;

		auto loop = [this, &bus, n, &from_to, &dist, &forward_weight, &reverse_weight, &first_stop_stat, spane_count]
					(size_t stops_count) mutable {
			for (size_t j = n + 1; j < stops_count; ++j) {
				Stop* stop_to = bus.stops_[j];
				from_to.first = from_to.second;
				from_to.second = stop_to;
				dist = GetDist(from_to);

				forward_weight += dist;

				const domain::StopStat* t_stop_stat = GetStopStat(stop_to->name_);
				if (first_stop_stat->id_ != t_stop_stat->id_) {
					graph_stat_.short_edges_.push_back(move(graph::Edge<double> { bus.name_, first_stop_stat->id_,
																t_stop_stat->id_, spane_count, forward_weight }));
					if (!bus.route_is_circular_) {
						std::pair<domain::Stop*, domain::Stop*> reverse_copy(from_to.second, from_to.first);
						dist = GetDist(reverse_copy);
						reverse_weight += dist;
						graph_stat_.short_edges_.push_back(move(graph::Edge<double> { bus.name_, t_stop_stat->id_,
							first_stop_stat->id_, spane_count, reverse_weight }));
					}
				}
				++spane_count;
			}
		};

		if (bus.route_is_circular_) {
			loop(stops_count);
		}
		else if (n < stops_count / 2) {
			loop(stops_count / 2 + 1);
		}

		stop_from = stop_to;
	}

	return make_pair(route_length, straight_dist);
}

void TransportCatalogue::UpdateBusStat()
{
	auto& bus = busses_.back();

	BusStat bus_stat;

	bus_stat.name_ = bus.name_;
	bus_stat.existing_ = true;

	auto[route_length, straight_dist] = DoLoopByStops(bus);

	bus_stat.route_length_ = route_length;
	bus_stat.curvature_ = route_length / straight_dist;
	bus_stat.unique_stops_count_ = bus.unique_stops_.size();
	bus_stat.stops_count_ = bus.stops_.size();

	busname_to_bus_[bus.name_] = &bus;
	busname_to_busstat_[bus.name_] = bus_stat;
}

int TransportCatalogue::GetDist(const std::pair<domain::Stop*, domain::Stop*>& from_to)
{
	auto dist_iter = from_to_dist.find(from_to);

	if (dist_iter == from_to_dist.end()) {
		std::pair<domain::Stop*, domain::Stop*> reverse_copy(from_to.second, from_to.first);

		dist_iter = from_to_dist.find(reverse_copy);

		if (dist_iter == from_to_dist.end()) {
			return 0;
		}
	}
	return (*dist_iter).second;
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
	UpdateBusStat();
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

Stop* TransportCatalogue::FindStop(size_t stop_id) const
{
	try
	{
		Stop* result = id_to_stop_.at(stop_id);
		return result;
	}
	catch (const std::out_of_range&)
	{
		return nullptr;
	}
}

const BusStat* TransportCatalogue::GetBusStat(string_view bus_name) const
{
	try
	{
		return &busname_to_busstat_.at(bus_name);
	}
	catch (const std::out_of_range&)
	{
		return nullptr;
	}
}

const std::deque<domain::Bus>& TransportCatalogue::GetBuses() const
{
	return busses_;
}

const StopStat* TransportCatalogue::GetStopStat(std::string_view stop_name) const
{
	try
	{
		return &stopname_to_stopstat_.at(stop_name);
	}
	catch (const std::out_of_range&)
	{
		return nullptr;
	}
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

size_t TransportCatalogue::GetStopCount()
{
	return stops_.size();
}

StatForGraph& TransportCatalogue::GetGraphStat()
{
	return graph_stat_;
}
