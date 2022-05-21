#pragma once
#include <deque>
#include <map>
#include <string>
#include <utility>
#include <variant>

#include "domain.h"
#include "graph.h"
#include "json.h"
#include "map_renderer.h"
#include "transport_catalogue.h"
#include "router.h"


// Класс RequestHandler играет роль Фасада, упрощающего взаимодействие JSON reader-а
// с другими подсистемами приложения.
// См. паттерн проектирования Фасад: https://ru.wikipedia.org/wiki/Фасад_(шаблон_проектирования)

namespace request {
    struct RawBus {
        std::string name_;
        bool route_is_circular_ = false;
        std::deque<std::string> stops_{};
    };

    namespace type
    {
        using BusOrStop = std::tuple<std::string, int, std::string >;
        using Map = std::tuple<std::string, int>;
        using Route = std::tuple<std::string, int, std::string, std::string>;
    }

    using StatRequest = std::variant<type::BusOrStop, type::Map, type::Route>;
    //using StatRequests = std::tuple<std::string, int, std::string >;

    using BaseRequests = std::tuple< std::deque<domain::Stop>, std::deque<domain::Distance>,
        std::deque<RawBus>>;

    using RenderSetings = json::Node;

    using RoutingSettings = json::Node;

    using Requests = std::tuple<BaseRequests, std::deque<StatRequest>, std::optional<RenderSetings>,
                                std::optional<RoutingSettings>>;
}

class RequestHandler {
public:
    RequestHandler(TransportCatalogue& db, const renderer::MapRenderer& renderer,
                    const domain::RouteSettings& route_set, const graph::DirectedWeightedGraph<double>& graph,
                    const graph::Router<double>& router);

    std::vector<std::tuple<char, int, domain::Stat>> GetStatistics(std::deque<request::StatRequest>& requests);

    const domain::BusStat* GetBusStat(const std::string& bus_name) const;
    const domain::StopStat* GetStopStat(const std::string& stop_name) const;
    domain::Map GetMap();
    std::optional<domain::Route> GetRoute(std::string from, std::string to);

    
private:
    const TransportCatalogue& db_;
    const renderer::MapRenderer& renderer_;
    const domain::RouteSettings& route_set_;
    const graph::DirectedWeightedGraph<double>& graph_;
    const graph::Router<double>& router_;
};

struct RequestProcessor: public RequestHandler {
    std::tuple<char, int, domain::Stat> operator()(request::type::BusOrStop stat_req);
    std::tuple<char, int, domain::Stat> operator()(request::type::Map stat_req);
    std::tuple<char, int, domain::Stat> operator()(request::type::Route);
};