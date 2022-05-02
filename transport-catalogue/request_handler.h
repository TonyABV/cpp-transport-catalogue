#pragma once
#include <deque>
#include <map>
#include <string>
#include <utility>

#include "domain.h"
#include "map_renderer.h"
#include "transport_catalogue.h"
#include "json.h"

// Класс RequestHandler играет роль Фасада, упрощающего взаимодействие JSON reader-а
// с другими подсистемами приложения.
// См. паттерн проектирования Фасад: https://ru.wikipedia.org/wiki/Фасад_(шаблон_проектирования)

namespace request {
    struct RawBus {
        std::string name_;
        bool route_is_circular_ = false;
        std::deque<std::string> stops_{};
    };

    using StatRequests = std::tuple<std::string, int, std::string >;

    using BaseRequests = std::tuple< std::deque<domain::Stop>, std::deque<domain::Distance>,
        std::deque<RawBus>>;

    using RenderSetings = json::Node;

    using Requests = std::tuple<BaseRequests, std::deque<StatRequests>, RenderSetings>;
}

class RequestHandler {
public:
    RequestHandler(TransportCatalogue& db, const renderer::MapRenderer& renderer);
    std::vector<std::tuple<char, int, domain::Stat>> GetStatistics(std::deque<request::StatRequests>& requests);
    domain::Map GetMap();
private:
    const TransportCatalogue& db_;
    const renderer::MapRenderer& renderer_;
};