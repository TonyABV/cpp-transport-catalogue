#include "transport_catalogue.h"

#include <iomanip>

namespace transport_db {
	size_t SvSvHasher::operator()(const std::pair<std::string_view, std::string_view>& stop_to_stop) const {
		return sv_hasher(stop_to_stop.first) + 37 * sv_hasher(stop_to_stop.second);
	}
	void TransportCatalogue::AddRoute(std::string name, const std::vector<std::string_view>& data, bool is_round, std::string_view end_stop) {
		std::string& ref_to_name = buses_names_.emplace_back(std::move(name)); // добавить строку в deque
		std::string_view sv_name{ ref_to_name }; // получить ссылку на значение в deque
		std::vector<std::string_view> v_stops;
		for (std::string_view stop : data) {
			v_stops.emplace_back(stops_[stop].name);
			stops_[stop].buses.insert(sv_name); // добавить этот автобус в контейнер всех остановок маршрута
		}
		routes_[sv_name] = { sv_name, v_stops, is_round, stops_.at(end_stop).name };
	}

	const domain::Bus* TransportCatalogue::SearchRoute(std::string_view name) const {
		if (!routes_.count(name)) {
			return nullptr;
		}
		return &routes_.at(name);
	}

	TransportCatalogue::RouteOutput TransportCatalogue::GetRoute(std::string_view name) {
		RouteOutput result;
		result.name = name.substr(0, name.size());
		if (!routes_.count(name)) {// если маршрут не существует - добавить "!" для обработки в ostream (overload)
			result.name.insert(0, "!");
			return result;
		}
		else {
			const domain::Bus tmp = *SearchRoute(name);
			result.real_stops_count = tmp.stops.size();
			// получить уникальные остановки через set-контейнер
			std::set<std::string_view> unique_stops{ tmp.stops.begin(), tmp.stops.end() };
			result.unique_stops_count = unique_stops.size();
			double road_distance = 0;
			double geo_distance = 0;
			for (auto it = tmp.stops.begin(); it < prev(tmp.stops.end()); ++it) { // цикл от begin() до (end()-1)
				road_distance += GetDistBtwStops((*it), *next(it));
				geo_distance += ComputeDistance(SearchStop(*it)->coordinates, SearchStop(*(it + 1))->coordinates);
			}
			result.route_length = road_distance;
			result.curvature = road_distance / geo_distance;
			return result;
		}
	}

	void  TransportCatalogue::AddStop(std::string name, const geo::Coordinates& coordinates) {
		std::string& ref_to_name = stops_names_.emplace_back(std::move(name)); // добавить строку в deque
		std::string_view sv_name{ ref_to_name }; // получить ссылку на значение в deque
		geo::Coordinates geo = { coordinates.lat, coordinates.lng };
		stops_[sv_name] = { sv_name, geo, {} }; // создать уникальную остановку без расстояний до других остановок (1-й этап добавления Остановок)
	}

	const  domain::Stop* TransportCatalogue::SearchStop(std::string_view name) const {
		if (!stops_.count(name)) {
			return nullptr;
		}
		return &stops_.at(name);
	}

	TransportCatalogue::StopOutput  TransportCatalogue::GetStop(std::string_view name) {
		StopOutput result;
		result.name = name.substr(0, name.size());
		if (!stops_.count(name)) {// если остановка не существует - добавить "!" для обработки в ostream (overload)
			result.name.insert(0, "!");
			return result;
		}
		else {
			const domain::Stop tmp = *SearchStop(name);
			result.buses = stops_[name].buses;
			return result;
		}
	}

	void  TransportCatalogue::SetDistBtwStops(std::string_view name, std::string_view name_to, const int dist) {
		const domain::Stop stop = *SearchStop(name); // получить данные string_view через указатели
		const domain::Stop stop_to = *SearchStop(name_to);
		dist_btw_stops_[std::pair(stop.name, stop_to.name)] = dist;
	}

	int  TransportCatalogue::GetDistBtwStops(std::string_view name, std::string_view name_to) {
		if (dist_btw_stops_.count(std::pair(name, name_to))) {
			return dist_btw_stops_.at(std::pair(name, name_to));
		}
		else {
			// Проверить обратную комбинацию имен остановок
			return dist_btw_stops_.at(std::pair(name_to, name));
		}
	}
}
