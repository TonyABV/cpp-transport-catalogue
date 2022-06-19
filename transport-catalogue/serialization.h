 #pragma once

#include <transport_catalogue.pb.h> 

#include "transport_catalogue.h"
#include "map_renderer.h"
#include "transport_router.h"

#include <string_view>
#include <map>


namespace serialization {

	using namespace std::literals;
	using namespace transport_db;

	struct SerializationSettings {
		std::string file_name = ""s;
	};

	class Serialization {
	public:
		Serialization(transport_db::TransportCatalogue& catalogue, map_renderer::MapRenderer& renderer, transport_router::TransportRouter& router)
			: catalogue_(catalogue), renderer_(renderer), router_(router)
		{}

		void SetSettings(const SerializationSettings& settings);

	public:
		void Serialize();
		void Deserialize();

	private:
		serialization::Catalogue SerializeCatalogueData();
		void DeserializeCatalogueData(const serialization::Catalogue& base);

		serialization::Stop SerializeStopData(const domain::Stop& stop, const std::map<std::string_view, int>& stops_ind);
		serialization::Bus SerializeBusData(const domain::Bus& bus, const std::map<std::string_view, int>& stops_ind);
		serialization::DistBtwStops SerializeDistanceData(const std::pair<std::string_view, std::string_view> stops, int length, const std::map<std::string_view, int>& stops_ind);
		
		/// Cериализация MapRenderer
		serialization::MapRenderer SerializeMapRendererData();
		serialization::RenderSettings SerializeRenderSettingsData();
		serialization::Color SerializeColorData(const svg::Color& catalogue_color);

		/// Десериализация MapRenderer
		void DeserializeMapRendererData(const serialization::MapRenderer& base_map_renderer);
		transport_db::RenderSettings DeserializeRenderSettingsData(const serialization::RenderSettings& base_render_settings);
		svg::Color DeserializeColorData(const serialization::Color& base_color);

		// Сериализация TransportRouter
		serialization::TransportRouter SerializeTransportRouterData();
		serialization::RouterSettings SerializeRouterSettingsData();
		serialization::TransportRouterData SerializeTransportRouterClassData();
		serialization::Router SerializeRouterData();
		serialization::Graph SerializeGraphData();

		//Десериализатор TransportRouter
		void DeserializeTransportRouterData(const serialization::TransportRouter& base_transport_router);
		transport_router::RouterSettings DeserializeTrasnportRouterSettingsData(const serialization::RouterSettings& base_router_settings);
		
		// Десериализатор Transport router class
		void DeserializeTransportRouterClassData(const serialization::TransportRouterData& base);
		using Vertexes = std::map<std::string_view, transport_router::StopAsVertexes>;
		Vertexes DeserializeTranspRouterVertexesData(const serialization::TransportRouterData& base_transport_router_data);
		using Edges = std::vector<transport_router::Edges>;
		Edges DeserializeTranspRouterEdgesData(const serialization::TransportRouterData& base_transport_router_data);
		
		// Десериализатор Graph
		void DeserializeGraphData(const serialization::Graph& base_graph_data);
		std::vector<graph::Edge<double>> DeserializeGraphEdgesData(const serialization::Graph& base_graph_data);
		std::vector<graph::IncidenceList> DeserializeGraphIncidenceListsData(const serialization::Graph& base_graph_data);
		
		// Десериализатор Router
		void DeserializeRouterData(const serialization::Router& base_router);
		std::optional<graph::Router<double>::RouteInternalData> DeserializeRouteInternalData(serialization::RouteInternalDataVectorElem& base);

	private:
		SerializationSettings settings_;

		transport_db::TransportCatalogue& catalogue_;
		map_renderer::MapRenderer& renderer_;
		transport_router::TransportRouter& router_;

		std::map<std::string_view, int> ser_stops_ind;		// индексы для сериализации
		std::map<std::string_view, int> ser_buses_ind;		// индексы для сериализации

		std::map<int, std::string_view> deser_stops_ind;	// индексы для десериализации
		std::map<int, std::string_view> deser_buses_ind;	// индексы для десериализации
	};
}