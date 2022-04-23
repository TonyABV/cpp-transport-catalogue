#pragma once
#include <deque>
#include <map>
#include <string>
#include <utility>

#include "map_renderer.h"
#include "transport_catalogue.h"
#include "json.h"

// Класс RequestHandler играет роль Фасада, упрощающего взаимодействие JSON reader-а
// с другими подсистемами приложения.
// См. паттерн проектирования Фасад: https://ru.wikipedia.org/wiki/Фасад_(шаблон_проектирования)

namespace req {
    using NewStop = std::tuple<std::string, double, double>;
    using Distance = std::pair<std::string, std::vector<std::pair<std::string, int>>>;
    using NewBus = std::tuple<std::string, bool, std::vector<std::string>, std::string>;
    using StatRequests = std::tuple<std::string, int, std::string >;

    using BaseRequests = std::tuple< std::deque<NewStop>, std::deque<Distance>,
        std::deque<NewBus>>;

    using RenderSetings = json::Node;

    using Requests = std::tuple<BaseRequests, std::deque<StatRequests>, RenderSetings>;
}

class RequestHandler {
public:
    RequestHandler(TransportCatalogue& db, const renderer::MapRenderer& rend);

    std::vector<std::tuple<char, int, domain::Info>> GetStatistics(std::deque<req::StatRequests>& requests);
    
    domain::Info InfoForMap();
private:
    domain::MaxMinLatLon ComputeMaxMin(std::deque<const domain::Bus*> busses);
    //// MapRenderer понадобится в следующей части итогового проекта
    //RequestHandler(const TransportCatalogue& db, const renderer::MapRenderer& renderer);

    //// Возвращает информацию о маршруте (запрос Bus)
    //std::optional<BusStat> GetBusStat(const std::string_view& bus_name) const;

    //// Возвращает маршруты, проходящие через
    //const std::unordered_set<BusPtr>* GetBusesByStop(const std::string_view& stop_name) const;

    // Этот метод будет нужен в следующей части итогового проекта
    //svg::Document RenderMap() const;

private:
    //// RequestHandler использует агрегацию объектов "Транспортный Справочник" и "Визуализатор Карты"

    const TransportCatalogue& db_;
    const renderer::MapRenderer& renderer_;
};