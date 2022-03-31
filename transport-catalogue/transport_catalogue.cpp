#include <algorithm>
#include <stdexcept>

#include "geo.h"
#include "transport_catalogue.h"

using namespace std;

//TransportCatalogue::TransportCatalogue(InputRequests&& requests) {
//	for (auto stop_req : requests.stop_requests_) {
//		AddStop(move(stop_req));
//	}
//	for (auto bus_req : requests.bus_requests_) {
//		AddBus(bus_req);
//	}
//}

void TransportCatalogue::AddStop(input::NewStop&& stop_req)
{
	Stop stop{ std::get<0>(stop_req), std::get<1>(stop_req), std::get<2>(stop_req), {} };
	stops_.push_back(move(stop));
	stopname_to_stop_[stops_.back().name] = &stops_.back();
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
	Bus empty_bus;
	busses_.push_back(move(empty_bus));
	auto& bus = busses_.back();
	bus.name_ = move(get<0>(bus_req));

	bus.route_is_circular_ = get<1>(bus_req);

	deque<Stop*> stops;
	for (auto& stop_name : get<2>(bus_req)) {
		Stop* stop_ptr= FindStop(stop_name);
		stops.push_back(stop_ptr);
		FindStop(stop_name)->bus_names.insert(busses_.back().name_);
	}

	bus.stops_ = move(stops);

	busname_to_bus_[bus.name_] = &busses_.back();
}

Bus* TransportCatalogue::FindBus(string_view bus_name)
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

Stop* TransportCatalogue::FindStop(string_view stop_name)
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

size_t TransportCatalogue::GetUniqs(const Bus* bus)
{
	std::unordered_set<string_view> stops_uniq_;
	for (const auto& stop : bus->stops_) {
		stops_uniq_.insert(move(stop->name));
	}
	return stops_uniq_.size();
}

Info TransportCatalogue::GetBusInfo(string&& bus_name)
{
	Bus* bus_ptr = FindBus(bus_name);
	Info result;

	if (bus_ptr == nullptr) {
		result.name_ = move(bus_name);
		return result;
	}

	result.existing_ = true;

	result.stops_on_route_ = bus_ptr->stops_.size();

	result.unique_stops_ = GetUniqs(FindBus(bus_name));
	result.name_ = move(bus_name);
	
	double straight_dist = 0;
	int route_length = 0;
	Coordinates from{ bus_ptr->stops_[0]->latitude, bus_ptr->stops_[0]->longitude };
	Stop* stop_from = bus_ptr->stops_[0];
	for (size_t n = 1; n < bus_ptr->stops_.size(); ++n) 
	{
		Coordinates to{ bus_ptr->stops_[n]->latitude, bus_ptr->stops_[n]->longitude };
		straight_dist += ComputeDistance(from, to);
		from = to;

		Stop* stop_to = bus_ptr->stops_[n];
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
		route_length += (*dist_iter).second;
		stop_from = stop_to;
	}
	result.curvature_ = route_length / straight_dist;
	result.straight_dist_ = move(straight_dist);
	result.route_length_ = move(route_length);
	return result;
}

Info TransportCatalogue::GetStopInfo(std::string&& stop_name)
{
	Info result;
	Stop* stop_ptr = FindStop(stop_name);
	if (stop_ptr == nullptr) {
		result.name_ = move(stop_name);
		return result;
	}
	result.existing_ = true;
	result.busses_to_stop_ = stop_ptr->bus_names;
	result.name_ = move(stop_name);
	return result;
}