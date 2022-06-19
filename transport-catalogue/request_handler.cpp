#include "request_handler.h"

#include <map>

namespace transport_db {

    std::optional<const domain::Bus*> RequestHandler::GetBusStat(std::string_view bus_name) {
        const domain::Bus* bus = db_.SearchRoute(bus_name);
        if (bus != nullptr) {
            return bus;
        }
        else {
            return std::nullopt;
        }
    }

    const std::set<std::string_view>* RequestHandler::GetBusesByStop(const std::string_view& stop_name) const {
        const domain::Stop* stop = db_.SearchStop(stop_name);
        if (!stop->buses.empty()) {
            return &(stop->buses);
        }
        else {
            return nullptr;
        }
    }

    void RequestHandler::RenderMap() const {
    }

    void RequestHandler::SetCatalogueDataToRender() const {
        SetStopsForRender();
        SetRoutesForRender();
    }

    void RequestHandler::SetStopsForRender() const {
        std::map<std::string_view, const domain::Stop*> stops;
        for (const auto& stop : db_.GetStopsForRender()) {
            stops[stop.first] = &stop.second;
        }
        renderer_.SetStops(stops);
    }

    void RequestHandler::SetRoutesForRender() const {
        std::map<std::string_view, const domain::Bus*> routes;
        for (const auto& route : db_.GetRoutesForRender()) {
            routes[route.first] = &route.second;
        }
        renderer_.SetRoutes(routes);
    }

    void RequestHandler::GenerateRouter() const {
        router_.GenerateRouter();
    }

    void RequestHandler::SerializeBase() const { 
        GenerateRouter();
        serialization_.Serialize();
    }

    void RequestHandler::DeserializeBase() const {         
        serialization_.Deserialize();
    }
}