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
//#include "stat_reader.h"

using namespace std;

void InfillCatalog(TransportCatalogue& tc, req::BaseRequests& req)
{
	for (auto& new_stop : std::get<0>(req)) {
		tc.AddStop(move(new_stop));
	}
	for (auto& dist : std::get<1>(req)) {
		tc.AddDist(move(dist));
	}
	for (auto& new_bus : std::get<2>(req)) {
		tc.AddBus(move(new_bus));
	}
}

int main() {
	/*ifstream input("input.json");
	ofstream output("output.json");*/

	req::Requests requests = json_input::MakeRequests(cin);

	TransportCatalogue tc;

	InfillCatalog(tc, get<0>(requests));

	renderer::RenderSettings settings = json_input::GetSettings(get<2>(requests).AsMap());

	renderer::MapRenderer map_renderer(settings);

	RequestHandler handler(tc, map_renderer);

	auto stat = handler.GetStatistics(get<1>(requests));

	json_input::PrintInfo(cout, stat);

	//ofstream output_svg("aaa.svg");
	/*auto map = handler.InfoForMap();
	cout << map.map_;*/
}
