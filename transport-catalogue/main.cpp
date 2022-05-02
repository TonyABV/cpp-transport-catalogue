#include <cassert>
#include <fstream> 
#include <iostream>
#include <istream>
#include <string>
#include <string_view>
#include <sstream>
#include <vector>

#include "domain.h"
#include "json_reader.h"
#include "map_renderer.h"
#include "transport_catalogue.h"

using namespace std;

domain::Bus PrepareNewBus(const TransportCatalogue& tc, request::RawBus& raw_bus) {
	domain::Bus bus{raw_bus.name_, raw_bus.route_is_circular_};
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

int main() {
	/*ifstream input("input.json");
	ofstream output("output.json");*/

	request::Requests requests = json_input::MakeRequests(cin);

	TransportCatalogue tc;

	InfillCatalog(tc, get<0>(requests));

	renderer::RenderSettings settings = json_input::GetSettings(get<2>(requests).AsMap());

	renderer::MapRenderer map_renderer(settings);

	RequestHandler handler(tc, map_renderer);

	auto stat = handler.GetStatistics(get<1>(requests));

	json_input::PrintInfo(cout, stat);

	/*ofstream output_svg("aaa.svg");
	auto map = handler.GetMap();
	output_svg << map;*/
}
