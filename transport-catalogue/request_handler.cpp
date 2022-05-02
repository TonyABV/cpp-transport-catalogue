#include "request_handler.h"

using namespace std;
using namespace request;

RequestHandler::RequestHandler(TransportCatalogue& db, const renderer::MapRenderer& renderer)
	: db_(db), renderer_(renderer) {}

vector<tuple<char, int, domain::Stat>> RequestHandler::GetStatistics(deque<request::StatRequests>& requests)
{
	vector<tuple<char, int, domain::Stat>> statistics;
	for (auto stat_req : requests) {
		if (get<0>(stat_req) == "Bus") {
			statistics.push_back(make_tuple('B', get<1>(stat_req),
				db_.GetBusStat(move(get<2>(stat_req)))));
		}
		else if(get<0>(stat_req) == "Stop"){
			statistics.push_back(make_tuple('S', get<1>(stat_req),
				db_.GetStopStat(move(get<2>(stat_req)))));
		}
		else if (get<0>(stat_req) == "Map") {
			statistics.push_back(make_tuple('M', get<1>(stat_req),
				 GetMap()));
		}/**/
	}
	return statistics;
}

domain::Map RequestHandler::GetMap()
{
	deque<const domain::Bus*> busses = db_.GetNonemptyBusses();
	return RequestHandler::renderer_.CreateMap(busses);
}