#include <cassert>
#include <iostream>
#include <istream>
#include <string>
#include <string_view>
#include <sstream>
#include <vector>

#include "input_reader.h"
#include "stat_reader.h"
#include "transport_catalogue.h"

using namespace std;

void InfillCatalog(TransportCatalogue& tc, input::Requests&& requests)
{
	for (input::NewStop stop_req : requests.stop_requests_) {
		tc.AddStop(move(stop_req));
	}
	for (input::Distance dist_req : requests.distance_requests_) {
		tc.AddDist(move(dist_req));
	}
	for (input::NewBus bus_req : requests.bus_requests_) {
		tc.AddBus(move(bus_req));
	}
}

vector<pair<char, Info>> GetStatistics(TransportCatalogue& tc,
								statistic::Requests&& requests) 
{
	vector<pair<char, Info>> statistics;
	for (auto request : requests.requests) {
		if (request.first == 'B') {
			statistics.push_back(make_pair('B',
				tc.GetBusInfo(move(request.second))));
		}
		else {
			statistics.push_back(make_pair('S',
				tc.GetStopInfo(move(request.second))));
		}
	}
	return statistics;
}

int main() {
	input::Requests input_requests = input::MakeRequests(cin);
	TransportCatalogue tc;
	InfillCatalog(tc, move(input_requests));
	statistic::Requests stat_requests = statistic::MakeRequests(cin);
	vector<pair<char, Info>> information = GetStatistics(tc, move(stat_requests));
	statistic::PrinStat(cout, move(information));
}