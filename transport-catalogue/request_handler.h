#pragma once
#include <deque>
#include <map>
#include <string>
#include <string_view>
#include <utility>
#include <variant>

#include "domain.h"
#include "graph.h"
#include "json.h"
#include "map_renderer.h"
#include "transport_catalogue.h"
#include "transport_router.h"

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
                    const domain::RouteSettings& route_set, const TransportRouter& router);

    std::vector<std::tuple<char, int, domain::Stat>> GetStatistics(std::deque<request::StatRequest>& requests);

    const domain::BusStat* GetBusStat(const std::string& bus_name) const;
    const domain::StopStat* GetStopStat(const std::string& stop_name) const;
    domain::Map GetMap();
    std::optional<domain::Route> GetRoute(std::string_view from, std::string_view to);
    
private:
    domain::Route ConvertToItems(const std::optional<graph::Router<double>::RouteInfo>& route,
                                                                                         std::string_view from);

    const TransportCatalogue& db_;
    const renderer::MapRenderer& renderer_;
    const domain::RouteSettings& route_set_;
    const TransportRouter& router_;
};

struct RequestProcessor: public RequestHandler {
    std::tuple<char, int, domain::Stat> operator()(request::type::BusOrStop stat_req);
    std::tuple<char, int, domain::Stat> operator()(request::type::Map stat_req);
    std::tuple<char, int, domain::Stat> operator()(request::type::Route);
};