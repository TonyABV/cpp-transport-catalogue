#include <cassert>
#include <fstream> 
#include <iostream>
#include <istream>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "domain.h"
#include "graph.h"
#include "json_reader.h"
#include "map_renderer.h"
#include "transport_catalogue.h"
#include "transport_router.h"

using namespace std;
using namespace domain;

domain::Bus PrepareNewBus(const TransportCatalogue& tc, request::RawBus& raw_bus);

void InfillCatalog(TransportCatalogue& tc, request::BaseRequests& req);

graph::DirectedWeightedGraph<double> BuildGraph(TransportCatalogue& tc, const domain::RouteSettings& settings);

int main() {
	/*ifstream input("input.json");
	ofstream output("output.json");*/

	request::Requests requests = json_input::MakeRequests(cin);

	TransportCatalogue tc;

	InfillCatalog(tc, get<0>(requests));

	renderer::RenderSettings rend_settings;

	if (get<2>(requests).has_value()) {
		rend_settings = json_input::GetRenderSettings(get<2>(requests).value().AsMap());
	}

	renderer::MapRenderer map_renderer(rend_settings);

	domain::RouteSettings rout_set;
	if (get<3>(requests).has_value()) {
		rout_set = json_input::GetRouteSettings(get<3>(requests).value().AsMap());
	}
	graph::DirectedWeightedGraph<double> graph(BuildGraph(tc, rout_set));

	TransportRouter router(graph);

	RequestHandler handler(tc, map_renderer, rout_set, router);
	
	auto stat = handler.GetStatistics(get<1>(requests));

	json_input::PrintInfo(cout, stat);
}

domain::Bus PrepareNewBus(const TransportCatalogue& tc, request::RawBus& raw_bus) {
	domain::Bus bus{ raw_bus.name_, raw_bus.route_is_circular_ };
	for (auto& stop_name : raw_bus.stops_) {
		domain::Stop* stop = tc.FindStop(stop_name);
		bus.stops_.push_back(stop);
		bus.unique_stops_.insert(stop);
	}
	return bus;
}

void InfillCatalog(TransportCatalogue& tc, request::BaseRequests& req)
{
	for (auto& new_stop : std::get<0>(req)) {
		tc.AddStop(move(new_stop));
	}
	for (auto& dist : std::get<1>(req)) {
		tc.SetDist(move(dist));
	}
	for (auto& raw_bus : std::get<2>(req)) {
		tc.AddBus(move(PrepareNewBus(tc, raw_bus)));
	}

}

graph::DirectedWeightedGraph<double> BuildGraph(TransportCatalogue& tc, const domain::RouteSettings& settings)
{
	StatForGraph& stat = tc.GetGraphStat();
	graph::DirectedWeightedGraph<double> graph(stat.stops_count_);
	for (auto& edge : stat.short_edges_) {
		edge.weight /= settings.bus_velocity;
		edge.weight += settings.wait_time;
		graph.AddEdge(edge);
	}
	return graph;
}