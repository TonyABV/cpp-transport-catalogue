#include "domain.h"

using namespace std;

//domain::Info domain::BusInfo(Bus* bus_ptr)
//{
//	Info result;
//	result.existing_ = true;
//	result.stops_on_route_ = bus_ptr->stops_.size();
//	result.unique_stops_ = bus_ptr->unique_stops_.size();
//	result.name_ = bus_ptr->name_;
//	result.straight_dist_ = bus_ptr->straight_dist_;
//	result.route_length_ = bus_ptr->route_length_;
//	result.curvature_ = bus_ptr->curvature_;
//	return result;
//}
//
//domain::Info domain::StopInfo(Stop* stop_ptr)
//{
//	Info result;
//	result.existing_ = true;
//	result.busses_to_stop_ = stop_ptr->bus_names_;
//	result.name_ = stop_ptr->name_;
//	return result;
//}

double domain::Dist(std::pair<double, double> lati_long_from, std::pair<double, double> lati_long_to)
{
	geo::Coordinates coord1{ lati_long_from.first, lati_long_from.second };
	geo::Coordinates coord2{ lati_long_to.first, lati_long_to.second };
	return geo::ComputeDistance(coord1, coord2);
}

size_t domain::BusHasher::operator()(const Bus* bus) const
{
	return string_haser(bus->name_);
}

size_t domain::PairBusHasher::operator()(const std::pair<Stop*, Stop*>&busses) const {
	return string_haser1(busses.first->name_) + 42 * string_haser2(busses.second->name_);
}