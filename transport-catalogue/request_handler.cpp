#include "request_handler.h"

using namespace std;

RequestHandler::RequestHandler(TransportCatalogue& db, const renderer::MapRenderer& rend) : db_(db), renderer_(rend) {}

vector<tuple<char, int, domain::Info>> RequestHandler::GetStatistics(deque<req::StatRequests>& requests)
{
	vector<tuple<char, int, domain::Info>> statistics;
	for (auto stat_req : requests) {
		if (get<0>(stat_req) == "Bus") {
			statistics.push_back(make_tuple('B', get<1>(stat_req),
				db_.GetBusInfo(move(get<2>(stat_req)))));
		}
		else if(get<0>(stat_req) == "Stop"){
			statistics.push_back(make_tuple('S', get<1>(stat_req),
				db_.GetStopInfo(move(get<2>(stat_req)))));
		}
		else if (get<0>(stat_req) == "Map") {
			statistics.push_back(make_tuple('M', get<1>(stat_req),
				 InfoForMap()));
		}
	}
	return statistics;
}

domain::MaxMinLatLon FindMaxMinLatLon(const domain::Bus* busses) {
	double min_lat = (busses->stops_)[0]->latitude_;
	double min_lon = (busses->stops_)[0]->longitude_;
	double max_lat = min_lat;
	double max_lon = min_lon;
	for (size_t n = 1; n < (busses->stops_.size()); ++n) {
		min_lat = min(min_lat, (busses->stops_)[n]->latitude_);
		min_lon = min(min_lon, (busses->stops_)[n]->longitude_);
		max_lat = max(max_lat, (busses->stops_)[n]->latitude_);
		max_lon = max(max_lon, (busses->stops_)[n]->longitude_);
	}
	
	return make_pair(make_pair(max_lat, max_lon), make_pair(min_lat, min_lon));
}

domain::MaxMinLatLon RequestHandler::ComputeMaxMin(deque<const domain::Bus*> busses) {
	pair<double, double> max_lat_lon;
	pair<double, double> min_lat_lon;
	bool is_first = true;
	for (const domain::Bus* bus : busses) {
		auto max_min_lat_lon = FindMaxMinLatLon(bus);
		if (is_first) {
			max_lat_lon.first = max_min_lat_lon.first.first;
			max_lat_lon.second = max_min_lat_lon.first.second;
			min_lat_lon.first = max_min_lat_lon.second.first;
			min_lat_lon.second = max_min_lat_lon.second.second;
			is_first = false;
			continue;
		}
		max_lat_lon.first = max(max_lat_lon.first, max_min_lat_lon.first.first);
		max_lat_lon.second = max(max_lat_lon.second, max_min_lat_lon.first.second);
		min_lat_lon.first = min(min_lat_lon.first, max_min_lat_lon.second.first);
		min_lat_lon.second = min(min_lat_lon.second, max_min_lat_lon.second.second);
	}
	return make_pair(max_lat_lon, min_lat_lon);
}

domain::Info RequestHandler::InfoForMap()
{
	domain::Info result;

	deque<const domain::Bus*> busses = db_.NonemptyBusses();
	domain::MaxMinLatLon max_min_lat_lon = ComputeMaxMin(busses);

	result.map_ = RequestHandler::renderer_.CreateMap(max_min_lat_lon, busses);
		
	return result;
}