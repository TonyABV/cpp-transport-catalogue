#include "serialization.h"

#include <filesystem>
#include <fstream>

namespace serialization {

	using namespace transport_db;

	void Serialization::SetSettings(const  SerializationSettings& settings) {
		settings_ = settings;
	}

	void Serialization::Serialize() {
		serialization::TransportCatalogue tcs;
		*tcs.mutable_catalogue() = std::move(SerializeCatalogueData());
		*tcs.mutable_map_renderer() = std::move(SerializeMapRendererData());
		*tcs.mutable_transport_router() = std::move(SerializeTransportRouterData());

		std::filesystem::path path = settings_.file_name;
		std::ofstream out_file(path, std::ios::binary);
		tcs.SerializeToOstream(&out_file);
	}

	serialization::Catalogue Serialization::SerializeCatalogueData() {
		serialization::Catalogue tmp;
		int s = 0; // индекс для сериализации в proto serialization::Stop.buses_index
		for (auto& [stop_name, stop] : catalogue_.GetStopsForRender()) {
			ser_stops_ind[stop_name] = s;
			tmp.add_stops_base(); // добавить элемент в массив proto
			*tmp.mutable_stops_base(s) = std::move(SerializeStopData(stop, ser_stops_ind)); // изменить новый элемент
			++s;
		}
		int b = 0; // индекс для сериализации в proto serialization::Bus.stops_index
		for (auto& [bus_name, bus] : catalogue_.GetRoutesForRender()) {
			ser_buses_ind[bus_name] = b;
			tmp.add_buses_base(); // добавить элемент в массив proto
			*tmp.mutable_buses_base(b) = std::move(SerializeBusData(bus, ser_stops_ind)); // изменить новый элемент
			++b;
		}
		int d = 0; // dist counter
		for (auto& [stops, distance] : catalogue_.GetDistForRouter()) {
			tmp.add_dist_btw_stops(); // добавить элемент в массив proto   
			*tmp.mutable_dist_btw_stops(d) = std::move(SerializeDistanceData(stops, distance, ser_stops_ind)); // изменить новый элемент
			++d;
		}
		return tmp;
	}

	serialization::Stop Serialization::SerializeStopData(const domain::Stop& stop, const std::map<std::string_view, int>& buses_index) {
		serialization::Stop tmp;
		std::string s_name{ stop.name }; //name как string
		tmp.set_name(s_name);
		tmp.mutable_coordinates()->set_geo_lat(stop.coordinates.lat);
		tmp.mutable_coordinates()->set_geo_lng(stop.coordinates.lng);
		return tmp;
	}

	serialization::Bus Serialization::SerializeBusData(const domain::Bus& bus, const std::map<std::string_view, int>& stops_index) {
		serialization::Bus tmp;
		std::string s_name{ bus.name }; //name как string
		tmp.set_name(s_name);
		for (std::string_view stop : bus.stops) {
			tmp.add_stops_index(stops_index.at(stop));
		}
		tmp.set_round_trip(bus.is_round_trip);
		tmp.set_end_stop_index(stops_index.at(bus.end_stop));
		return tmp;
	}

	serialization::DistBtwStops Serialization::SerializeDistanceData(const std::pair<std::string_view, std::string_view> stops, int length, const std::map<std::string_view, int>& stops_ind) {
		serialization::DistBtwStops tmp;
		tmp.set_from(stops_ind.at(stops.first));
		tmp.set_to(stops_ind.at(stops.second));
		tmp.set_distance(length);
		return tmp;
	}

	/// Сериализация MapRenderer
	serialization::MapRenderer Serialization::SerializeMapRendererData() {
		serialization::MapRenderer tmp;
		*tmp.mutable_settings() = std::move(SerializeRenderSettingsData());
		return tmp;
	}

	serialization::RenderSettings Serialization::SerializeRenderSettingsData() {
		serialization::RenderSettings tmp;
		transport_db::RenderSettings cat_rend_set = renderer_.GetRenderSettings();
		tmp.set_width(cat_rend_set.width);
		tmp.set_height(cat_rend_set.height);
		tmp.set_padding(cat_rend_set.padding);
		tmp.set_line_width(cat_rend_set.line_width);
		tmp.set_stop_radius(cat_rend_set.stop_radius);
		tmp.set_bus_label_font_size(cat_rend_set.bus_label_font_size);
		tmp.add_bus_label_offset(cat_rend_set.bus_label_offset[0]);
		tmp.add_bus_label_offset(cat_rend_set.bus_label_offset[1]);
		tmp.set_stop_label_font_size(cat_rend_set.stop_label_font_size);
		tmp.add_stop_label_offset(cat_rend_set.stop_label_offset[0]);
		tmp.add_stop_label_offset(cat_rend_set.stop_label_offset[1]);
		*tmp.mutable_underlayer_color() = SerializeColorData(cat_rend_set.underlayer_color);
		tmp.set_underlayer_width(cat_rend_set.underlayer_width);
		for (int i = 0; i < cat_rend_set.color_palette.size(); ++i) {
			tmp.add_color_palette();
			*tmp.mutable_color_palette(i) = SerializeColorData(cat_rend_set.color_palette[i]);
		}
		return tmp;
	}

	serialization::Color Serialization::SerializeColorData(const svg::Color& catalogue_color) {
		serialization::Color tmp;
		if (std::holds_alternative<std::monostate>(catalogue_color)) {
			return {};
		}
		else if (std::holds_alternative<std::string>(catalogue_color)) {
			tmp.set_string_name(std::get<std::string>(catalogue_color));
		}
		else if (std::holds_alternative<svg::Rgb>(catalogue_color)) {
			svg::Rgb tmp_rgb_color = std::get<svg::Rgb>(catalogue_color);
			tmp.mutable_rgb()->set_r(tmp_rgb_color.red);
			tmp.mutable_rgb()->set_g(tmp_rgb_color.green);
			tmp.mutable_rgb()->set_b(tmp_rgb_color.blue);
		}
		else {
			svg::Rgba tmp_rgba_color = std::get<svg::Rgba>(catalogue_color);
			tmp.mutable_rgba()->set_r(tmp_rgba_color.red);
			tmp.mutable_rgba()->set_g(tmp_rgba_color.green);
			tmp.mutable_rgba()->set_b(tmp_rgba_color.blue);
			tmp.mutable_rgba()->set_opacity(tmp_rgba_color.opacity);
		}
		return tmp;
	}

	/// TransportRouter
	serialization::TransportRouter Serialization::SerializeTransportRouterData() {
		serialization::TransportRouter tmp;
		*tmp.mutable_settings() = std::move(SerializeRouterSettingsData());
		*tmp.mutable_transport_router() = std::move(SerializeTransportRouterClassData());
		*tmp.mutable_router() = std::move(SerializeRouterData());
		*tmp.mutable_graph() = std::move(SerializeGraphData());
		return tmp;
	}

	serialization::RouterSettings Serialization::SerializeRouterSettingsData() {
		serialization::RouterSettings tmp;
		transport_db::RouterSettings cat_router_set = router_.GetSettings();
		tmp.set_bus_velocity_kmh(cat_router_set.bus_velocity_kmh);
		tmp.set_bus_wait_time(cat_router_set.bus_wait_time);
		return tmp;
	}

	serialization::TransportRouterData Serialization::SerializeTransportRouterClassData() {
		serialization::TransportRouterData tmp;
		int i = 0;
		for (const auto& [name, data] : *router_.GetVertexes()) { //Конвертация Vertexes. name и data.in.id - необходимые данные для функции BuildRoute
			tmp.add_vertexes(); // добавить элемент в массив proto
			tmp.mutable_vertexes(i)->set_stop_id(ser_stops_ind.at(name));
			tmp.mutable_vertexes(i)->set_id(data.in.id);
			++i;
		}
		int j = 0;
		for (const auto& edge : *router_.GetEdgesData()) { //Конвертация Edges. 
			tmp.add_edges();
			tmp.mutable_edges(j)->set_type(static_cast<int>(edge.type));
			if (edge.type == transport_router::edge_type::WAIT) { // WAIT равно STOP
				tmp.mutable_edges(j)->set_name_id(ser_stops_ind.at(edge.name));
			}
			else {
				tmp.mutable_edges(j)->set_name_id(ser_buses_ind.at(edge.name));
			}
			tmp.mutable_edges(j)->set_span_count(edge.span_count);
			tmp.mutable_edges(j)->set_time(edge.time);
			++j;
		}
		return tmp;
	}

	serialization::Router Serialization::SerializeRouterData() {
		serialization::Router tmp;
		int i = 0;
		for (const auto& data_vector : router_.GetRouterData()) {
			int j = 0;
			tmp.add_routes_internal_data();
			for (const auto& data : data_vector) {
				tmp.mutable_routes_internal_data(i)->add_route_internal_data_elem();
				if (data) {
					serialization::RouteInternalData& elem_data = *tmp.mutable_routes_internal_data(i)->mutable_route_internal_data_elem(j)->mutable_data();
					elem_data.set_weight(data.value().weight); // weight всегда определен
					if (data.value().prev_edge) { // проверяем, есть ли у элемента prev_edge
						elem_data.set_edgeid(data.value().prev_edge.value()); // назначаем идентификатор prev_edge
					}
				}
				++j;
			}
			++i;
		}
		return tmp;
	}

	serialization::Graph Serialization::SerializeGraphData() {
		serialization::Graph tmp;
		for (int i = 0; i < router_.GetGraph().GetEdgeCount(); ++i) {          
			graph::Edge tmp_cat_edge = router_.GetGraph().GetEdge(i);
			tmp.add_edges(); // добавить элемент в массив proto
			serialization::Edge& tmp_base_edge = *tmp.mutable_edges(i);
			tmp_base_edge.set_from(tmp_cat_edge.from);
			tmp_base_edge.set_to(tmp_cat_edge.to);
			tmp_base_edge.set_weight(tmp_cat_edge.weight);
		}
		for (size_t i = 0; i < router_.GetGraph().GetVertexCount(); ++i) {
			tmp.add_incidence_lists();
			serialization::IncidenceList& tmp_base_inc_list = *tmp.mutable_incidence_lists(i);
			int j = 0;
			for (const auto inc_edge : router_.GetGraph().GetIncidentEdges(i)) {// конвертация incedence_list из ребра X
				tmp_base_inc_list.add_edges(inc_edge);
				++j;
			}
		}
		return tmp;
	}

	void Serialization::Deserialize() {
		std::filesystem::path path = settings_.file_name;
		std::ifstream in_file(path, std::ios::binary);		
		
		serialization::TransportCatalogue tcs;
		tcs.ParseFromIstream(&in_file);

		DeserializeCatalogueData(tcs.catalogue());
		DeserializeMapRendererData(tcs.map_renderer());
		router_.GenerateEmptyRouter(); // создаем связь между графом и классами маршрутизатора
		DeserializeTransportRouterData(tcs.transport_router());
		DeserializeRouterData(tcs.transport_router().router());
		DeserializeGraphData(tcs.transport_router().graph());
	}
	
	void Serialization::DeserializeCatalogueData(const serialization::Catalogue& tcs) {
		for (int i = 0; i < tcs.stops_base_size(); ++i) {
			std::string name = tcs.stops_base(i).name();
			const geo::Coordinates& geo_coordinates{ tcs.stops_base(i).coordinates().geo_lat(), tcs.stops_base(i).coordinates().geo_lng() };
			catalogue_.AddStop(name, geo_coordinates);
			deser_stops_ind[i] = catalogue_.SearchStop(tcs.stops_base(i).name())->name;
		}
		for (int i = 0; i < tcs.buses_base_size(); ++i) {
			std::string name = tcs.buses_base(i).name();
			std::vector<std::string_view> stops;
			for (int j = 0; j < tcs.buses_base(i).stops_index_size(); ++j) { // подготовить маршрут автобуса
				stops.push_back(deser_stops_ind.at(tcs.buses_base(i).stops_index(j)));
			}
			bool is_round = tcs.buses_base(i).round_trip();
			std::string_view end_stop = deser_stops_ind.at(tcs.buses_base(i).end_stop_index());
			catalogue_.AddRoute(name, stops, is_round, end_stop);
			deser_buses_ind[i] = catalogue_.SearchRoute(tcs.buses_base(i).name())->name;
		}
		for (int i = 0; i < tcs.dist_btw_stops_size(); ++i) {
			catalogue_.SetDistBtwStops(deser_stops_ind.at(tcs.dist_btw_stops(i).from()), deser_stops_ind.at(tcs.dist_btw_stops(i).to()), tcs.dist_btw_stops(i).distance());
		}
		//for (int i = 0; i < tcs.buses_base_size(); ++i) {
		//	catalogue_.GetRoute(tcs.buses_base(i).name());
		//}
	}

	/// Десериализация MapRenderer
	void Serialization::DeserializeMapRendererData(const serialization::MapRenderer& base_map_renderer) {
		renderer_.SetSettings(DeserializeRenderSettingsData(base_map_renderer.settings()));
	}

	transport_db::RenderSettings Serialization::DeserializeRenderSettingsData(const serialization::RenderSettings& base_render_settings) {
		transport_db::RenderSettings tmp;
		tmp.width = base_render_settings.width();
		tmp.height = base_render_settings.height();
		tmp.padding = base_render_settings.padding();
		tmp.line_width = base_render_settings.line_width();
		tmp.stop_radius = base_render_settings.stop_radius();
		tmp.bus_label_font_size = base_render_settings.bus_label_font_size();
		tmp.bus_label_offset[0] = base_render_settings.bus_label_offset(0);
		tmp.bus_label_offset[1] = base_render_settings.bus_label_offset(1);
		tmp.stop_label_font_size = base_render_settings.stop_label_font_size();
		tmp.stop_label_offset[0] = base_render_settings.stop_label_offset(0);
		tmp.stop_label_offset[1] = base_render_settings.stop_label_offset(1);
		tmp.underlayer_color = DeserializeColorData(base_render_settings.underlayer_color());
		tmp.underlayer_width = base_render_settings.underlayer_width();
		tmp.color_palette.reserve(base_render_settings.color_palette_size());
		for (int i = 0; i < base_render_settings.color_palette_size(); ++i) {
			tmp.color_palette.emplace_back(std::move(DeserializeColorData(base_render_settings.color_palette(i))));
		}
		return tmp;
	}

	svg::Color Serialization::DeserializeColorData(const serialization::Color& base_color) {
		svg::Color empty_color{};
		switch (base_color.color_case()) {
		case serialization::Color::ColorCase::COLOR_NOT_SET:
			return  empty_color;
			break;
		case serialization::Color::ColorCase::kStringName:
			return base_color.string_name();
			break;
		case serialization::Color::ColorCase::kRgb:
			return svg::Rgb(base_color.rgb().r(), base_color.rgb().g(), base_color.rgb().b());
			break;
		case serialization::Color::ColorCase::kRgba:
			return svg::Rgba(base_color.rgba().r(), base_color.rgba().g(), base_color.rgba().b(), base_color.rgba().opacity());
			break;
		}
		return  empty_color;
	}

	///Десериализатор TransportRouter
	void Serialization::DeserializeTransportRouterData(const serialization::TransportRouter& base_transport_router) {
		router_.SetSettings(DeserializeTrasnportRouterSettingsData(base_transport_router.settings()));
		DeserializeTransportRouterClassData(base_transport_router.transport_router());
		DeserializeGraphData(base_transport_router.graph());
	}
	// Десериализатор Transport router class
	transport_router::RouterSettings Serialization::DeserializeTrasnportRouterSettingsData(const serialization::RouterSettings& base_router_settings) {
		transport_router::RouterSettings tmp_settings;
		tmp_settings.bus_velocity_kmh = base_router_settings.bus_velocity_kmh();
		tmp_settings.bus_wait_time = base_router_settings.bus_wait_time();
		return tmp_settings;
	}

	void Serialization::DeserializeTransportRouterClassData(const serialization::TransportRouterData& base_transport_router_data) {
		router_.ModifyVertexes() = std::move(DeserializeTranspRouterVertexesData(base_transport_router_data));
		router_.ModifyEdgesData() = DeserializeTranspRouterEdgesData(base_transport_router_data);
	}

	Serialization::Vertexes Serialization::DeserializeTranspRouterVertexesData(const serialization::TransportRouterData& base_transport_router_data) {
		Vertexes tmp_vertexes;
		for (int i = 0; i < base_transport_router_data.vertexes_size(); ++i) {
			tmp_vertexes[deser_stops_ind.at(base_transport_router_data.vertexes(i).stop_id())].in.id = base_transport_router_data.vertexes(i).id();
		}
		return tmp_vertexes;
	}

	Serialization::Edges Serialization::DeserializeTranspRouterEdgesData(const serialization::TransportRouterData& base_transport_router_data) {
		Edges tmp_edges;
		tmp_edges.reserve(base_transport_router_data.edges_size());
		for (int i = 0; i < base_transport_router_data.edges_size(); ++i) {
			transport_router::Edges tmp_edge;
			switch (base_transport_router_data.edges(i).type()) {
			case 0:
				tmp_edge.type = transport_router::edge_type::WAIT;
				tmp_edge.name = deser_stops_ind.at(base_transport_router_data.edges(i).name_id());
				break;
			case 1:
				tmp_edge.type = transport_router::edge_type::BUS;
				tmp_edge.name = deser_buses_ind.at(base_transport_router_data.edges(i).name_id());
				break;
			}
			tmp_edge.time = base_transport_router_data.edges(i).time();
			tmp_edge.span_count = base_transport_router_data.edges(i).span_count();
			tmp_edges.emplace_back(std::move(tmp_edge));
		}
		return tmp_edges;
	}
	
	// Десериализатор Graph
	void Serialization::DeserializeGraphData(const serialization::Graph& base_graph_data) {
		router_.ModifyGraph().ModifyEdges() = std::move(DeserializeGraphEdgesData(base_graph_data));
		router_.ModifyGraph().ModifyIncidenceLists() = std::move(DeserializeGraphIncidenceListsData(base_graph_data));
	}

	std::vector<graph::Edge<double>> Serialization::DeserializeGraphEdgesData(const serialization::Graph& base_graph_data) {
		std::vector<graph::Edge<double>> tmp_edges;;
		tmp_edges.reserve(base_graph_data.edges_size());
		for (int i = 0; i < base_graph_data.edges_size(); ++i) {
			graph::Edge<double> tmp_edge;
			tmp_edge.from = base_graph_data.edges(i).from();
			tmp_edge.to = base_graph_data.edges(i).to();
			tmp_edge.weight = base_graph_data.edges(i).weight();
			tmp_edges.emplace_back(std::move(tmp_edge));
		}
		return tmp_edges;
	}

	std::vector<graph::IncidenceList> Serialization::DeserializeGraphIncidenceListsData(const serialization::Graph& base_graph_data) {
		std::vector<graph::IncidenceList> tmp_inc_lists;
		tmp_inc_lists.reserve(base_graph_data.incidence_lists_size());
		for (int i = 0; i < base_graph_data.incidence_lists_size(); ++i) {
			graph::IncidenceList tmp_list;
			tmp_list.reserve(base_graph_data.incidence_lists(i).edges_size());
			for (int j = 0; j < base_graph_data.incidence_lists(i).edges_size(); ++j) {
				tmp_list.emplace_back(base_graph_data.incidence_lists(i).edges(j));
			}
			tmp_inc_lists.emplace_back(std::move(tmp_list));
		}
		return tmp_inc_lists;
	}

	// Десериализатор Router
	void Serialization::DeserializeRouterData(const serialization::Router& base_router) {
		std::vector<std::vector<std::optional<graph::Router<double>::RouteInternalData>>>& routes_internal_data = router_.ModifyRouter().get()->ModifyRoutesInternalData();
		routes_internal_data.resize(base_router.routes_internal_data_size());
		for (int i = 0; i < base_router.routes_internal_data_size(); ++i) {
			routes_internal_data[i].reserve(base_router.routes_internal_data(i).route_internal_data_elem_size());
			for (int j = 0; j < base_router.routes_internal_data(i).route_internal_data_elem_size(); ++j) {
				serialization::RouteInternalDataVectorElem base_elem = base_router.routes_internal_data(i).route_internal_data_elem(j);
				routes_internal_data[i].emplace_back(std::move(DeserializeRouteInternalData(base_elem)));
			}
		}
	}

	std::optional<graph::Router<double>::RouteInternalData> Serialization::DeserializeRouteInternalData(serialization::RouteInternalDataVectorElem& base) {
		graph::Router<double>::RouteInternalData res{};
		switch (base.elem_case()) {
		case serialization::RouteInternalDataVectorElem::ElemCase::ELEM_NOT_SET:
			return std::nullopt;
			break;
		case serialization::RouteInternalDataVectorElem::ElemCase::kData:
			res.weight = base.data().weight();
			switch (base.data().prev_edge_case()) {
			case serialization::RouteInternalData::PrevEdgeCase::kEdgeid:
				res.prev_edge = std::make_optional(base.data().edgeid());
				break;
			case serialization::RouteInternalData::PrevEdgeCase::PREV_EDGE_NOT_SET:
				res.prev_edge = std::nullopt;
				break;
			}
		}
		return res;
	}
}