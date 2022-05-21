#include "domain.h"

using namespace std;

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

size_t domain::PairBusHasher::operator()(const std::pair<Stop*, Stop*>& busses) const {
	return haser(busses.first) + 42 * haser(busses.second);
}