#pragma once

#include "json.h"
#include "json_builder.h"
#include "request_handler.h"
#include "map_renderer.h"
#include "transport_catalogue.h"
#include "transport_router.h"
#include "serialization.h"

#include <sstream>
#include <variant>
#include <iomanip>
#include <iostream>

namespace json_reader {

    using namespace json;

    namespace detail {

        enum class RequestType {
            BASE = 0,   // добавление
            STAT,       // получение
            RENDER,     // генерация карты
            EMPTY
        };

        enum class request_type { BUS, STOP, MAP, ROUTE, EMPTY };

        // структура запроса
        struct Request {
            RequestType e_type = RequestType::EMPTY;
            request_type type = request_type::EMPTY;
            virtual ~Request() = default;
        };

        // структура запроса для Route
        struct RequestStatRoute {
            std::string from;
            std::string to;
        };

        // структура запроса статистики
        struct RequestStat : public Request {
            std::string name; // требуется для поиска по названию автобуса или остановки 
            int id = 0;
            RequestStatRoute route;
        };

        // структура запроса для Stop
        struct RequestStop : public Request {
            std::string name;
            double latitude = 0;
            double longitude = 0;
            std::map<std::string, int> road_distances;
        };

        // структура запроса для Bus
        struct RequestBus : public Request {
            std::string name;
            bool is_roundtrip = false;
            std::vector<std::string> stops;
        };

        std::map<std::string, int> DistBtwStops(const Dict& dic); // Обработать расстояния до остановок для BaseStop()
        RequestStop BaseStop(const Dict& dic);
        std::vector<std::string> StopsForBus(const Array& arr); // Обработать расстояния до остановок для BaseBus()
        RequestBus BaseBus(const Dict& dic);

        RequestStat Stat(const Dict& dic);

        svg::Color ColorFromNode(const json::Node& node);
    } // detail
    using namespace detail;

    class JsonReader {
    public:
        JsonReader(transport_db::TransportCatalogue& catalogue, map_renderer::MapRenderer& renderer, transport_router::TransportRouter& router,
            serialization::Serialization& serialization)
            :catalogue_(catalogue), renderer_(renderer), router_(router), serialization_(serialization) {
        }

        void ReadInput(std::istream& input);

        void FillCatalogue();

        void PrintRequests(std::ostream& out, transport_router::RequestHandler& request_handler);

    private:
        transport_db::TransportCatalogue& catalogue_;
        Document document_;
        std::vector<std::unique_ptr<Request>> requests_;
        map_renderer::MapRenderer& renderer_;
        transport_router::TransportRouter& router_;
        serialization::Serialization& serialization_; 

        void FillDoc(std::istream& strm);
        void FillBase(const std::vector <Node>& vec);
        void FillStat(const std::vector<Node>& vec);
        void FillRender(const std::map<std::string, json::Node>& dic);
        void FillRouting(const std::map<std::string, Node>& dic);
        void FillSerialization(const std::map<std::string, Node>& dic); 

        void ProcessStopStatRequest(const transport_db::TransportCatalogue::StopOutput& request, Builder& dict);
        void ProcessBusStatRequest(const transport_db::TransportCatalogue::RouteOutput& request, Builder& dict);
    };
} // json_reader

