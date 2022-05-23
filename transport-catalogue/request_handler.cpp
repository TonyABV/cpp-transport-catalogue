#include "request_handler.h"

using namespace std;
using namespace request;

RequestHandler::RequestHandler(TransportCatalogue& db, const renderer::MapRenderer& renderer,
								const domain::RouteSettings& route_set, const TransportRouter& router)
	: db_(db), renderer_(renderer), route_set_(route_set), router_(router){}

tuple<char, int, domain::Stat> RequestProcessor::operator()(request::type::BusOrStop stat_req)
{
	if (get<0>(stat_req) == "Bus") {
		//domain::BusStat
		return make_tuple('B', get<1>(stat_req), GetBusStat(move(get<2>(stat_req))));
	}

	return make_tuple('S', get<1>(stat_req), GetStopStat(move(get<2>(stat_req))));
}

tuple<char, int, domain::Stat> RequestProcessor::operator()(request::type::Map stat_req)
{
	return make_tuple('M', get<1>(stat_req), GetMap());
}

tuple<char, int, domain::Stat> RequestProcessor::operator()(request::type::Route stat_req)
{
	optional<domain::Route> route_stat = GetRoute(get<2>(stat_req), get<3>(stat_req));
	if (route_stat.has_value()) {
		return make_tuple('R', get<1>(stat_req), route_stat.value());
	}
	return make_tuple('R', get<1>(stat_req), domain::Stat());
}

vector<tuple<char, int, domain::Stat>> RequestHandler::GetStatistics(deque<request::StatRequest>& requests)
{
	vector<tuple<char, int, domain::Stat>> statistics;
	statistics.reserve(requests.size());
	RequestProcessor proc{ *this };
	for (auto stat_req : requests) {
		statistics.push_back(visit(proc, stat_req));
	}
	return statistics;
}

const domain::BusStat* RequestHandler::GetBusStat(const string& bus_name) const
{
	return db_.GetBusStat(bus_name);
}

const domain::StopStat* RequestHandler::GetStopStat(const std::string& stop_name) const
{
	return db_.GetStopStat(stop_name);
}

domain::Map RequestHandler::GetMap()
{
	deque<const domain::Bus*> busses = db_.GetNonemptyBusses();
	return RequestHandler::renderer_.CreateMap(busses);
}

domain::Route RequestHandler::ConvertToItems(const std::optional<graph::Router<double>::RouteInfo>& route,
																						std::string_view from)
{
	domain::Route result;

	result.total_time = route.value().weight;

	auto* stop = db_.FindStop(from);

	result.items.push_back(domain::item::Wait{ stop->name_, int(route_set_.wait_time) });

	const auto& graph = router_.GetGraph();

	const auto& eges = route.value().edges;
	for (size_t n = 0; n < eges.size(); ++n) {
		const auto& edge = graph.GetEdge(eges[n]);
		if (n == eges.size() - 1) {
			result.items.push_back(domain::item::Bus{ edge.bus_name, int(edge.span_count),
				edge.weight - route_set_.wait_time });
			break;
		}
		result.items.push_back(domain::item::Bus{ edge.bus_name, int(edge.span_count), edge.weight - route_set_.wait_time });

		stop = db_.FindStop(edge.to);
		result.items.push_back(domain::item::Wait{ stop->name_, int(route_set_.wait_time) });
	}

	return result;
}

optional<domain::Route> RequestHandler::GetRoute(std::string_view from, std::string_view to)
{
	const domain::StopStat* f_stop_stat = db_.GetStopStat(from);
	const domain::StopStat* t_stop_stat = db_.GetStopStat(to);

	std::optional<graph::Router<double>::RouteInfo> route = router_.BuildRoute(f_stop_stat->id_, t_stop_stat->id_);

	if (!route.has_value()) {
		return std::nullopt;
	}
	else if (route.value().edges.empty()) {
		return domain::Route();
	}

	domain::Route result(ConvertToItems(route, from));	

	return result;
}
