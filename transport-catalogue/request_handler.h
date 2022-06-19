#pragma once

#include "transport_catalogue.h"
#include "map_renderer.h"
#include "transport_router.h"
#include "serialization.h"

#include <optional>
#include <iostream>

namespace transport_db {

    class RequestHandler {
    public:
        explicit RequestHandler(const transport_db::TransportCatalogue& db, map_renderer::MapRenderer& renderer, transport_router::TransportRouter& router, serialization::Serialization& serialization)
            :db_(db), renderer_(renderer), router_(router), serialization_(serialization) {
        }

        std::optional<const domain::Bus*> GetBusStat(std::string_view bus_name);

        const std::set<std::string_view>* GetBusesByStop(const std::string_view& stop_name) const;

        void RenderMap() const;

        void SetCatalogueDataToRender() const;

        void GenerateRouter() const;

        void SerializeBase() const; 

        void DeserializeBase() const; 

    private:

        const TransportCatalogue& db_;
        map_renderer::MapRenderer& renderer_;
        transport_router::TransportRouter& router_;
        serialization::Serialization& serialization_; 

        void SetStopsForRender() const;
        void SetRoutesForRender() const;
    };
} // namespace transport_db