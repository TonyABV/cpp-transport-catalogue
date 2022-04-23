#include <algorithm>
#include <stdexcept>

#include "transport_catalogue.h"

using namespace std;
using namespace domain;

void TransportCatalogue::AddStop(input::NewStop&& stop_req)
{
	Stop stop{ std::get<0>(stop_req), std::get<1>(stop_req), std::get<2>(stop_req), {} };

	stops_.push_back(move(stop));
	stopname_to_stop_[stops_.back().name_] = &stops_.back();
}

void TransportCatalogue::AddDist(input::Distance&& dist_req)
{
	Stop* stop_from = FindStop(dist_req.first);

	for (auto stopto_dist : dist_req.second) {
		Stop* stop_to = FindStop(stopto_dist.first);
		pair from_to = make_pair(stop_from, stop_to);
		from_to_dist[from_to] = stopto_dist.second;
	}
}

void TransportCatalogue::AddBus(input::NewBus&& bus_req)
{
	Bus new_bus{ get<0>(bus_req), get<1>(bus_req) };

	busses_.push_back(move(new_bus));
	auto& bus = busses_.back();

	bus.final_stop_ = FindStop(get<3>(bus_req));

	deque<Stop*> stops;
	set<Stop*, StopCmp> uniq_stops;

	double straight_dist = 0;
	int route_length = 0;

	string& firststop_name = get<2>(bus_req)[0];
	Stop* firststop_ptr = FindStop(firststop_name);

	uniq_stops.insert(FindStop(firststop_ptr->name_));
	stops.push_back(firststop_ptr);
	firststop_ptr->bus_names_.insert(busses_.back().name_);

	pair<double, double> from{ firststop_ptr->latitude_, firststop_ptr->longitude_ };

	Stop* stop_from = firststop_ptr;
	for (size_t n = 1; n < get<2>(bus_req).size(); ++n)
	{
		string& stop_name = get<2>(bus_req)[n];
		Stop* stop_to = FindStop(stop_name);

		uniq_stops.insert(FindStop(stop_to->name_));
		stops.push_back(stop_to);
		stop_to->bus_names_.insert(busses_.back().name_);

		pair<double, double> to{ stop_to->latitude_, stop_to->longitude_ };
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

	bus.straight_dist_ = straight_dist;
	bus.route_length_ = route_length;
	bus.curvature_ = route_length / straight_dist;
	bus.stops_ = move(stops);
	busname_to_bus_[bus.name_] = &busses_.back();
	bus.unique_stops_ = move(uniq_stops);
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

Info TransportCatalogue::GetBusInfo(string&& bus_name) const
{
	Bus* bus_ptr = FindBus(bus_name);
	if (bus_ptr == nullptr) {
		Info result;
		result.name_ = move(bus_name);
		return result;
	}
	return BusInfo(bus_ptr);
}

Info TransportCatalogue::GetStopInfo(std::string&& stop_name) const
{
	Stop* stop_ptr = FindStop(stop_name);
	if (stop_ptr == nullptr) {
		Info result;
		result.name_ = move(stop_name);
		return result;
	}
	return StopInfo(stop_ptr);
}

deque<const domain::Bus*> TransportCatalogue::NonemptyBusses() const
{
	deque<const domain::Bus*> result;
	for (auto& bus_ref : busses_) {
		if (!bus_ref.stops_.empty()) {
			result.push_back(&bus_ref);
		}
	}
	return result;
}
