///////////////////////////////////////////////////////////////////////////
//Визуализация карты маршрутов в формате SVG.

#pragma once

#include "domain.h"
#include "geo.h"
#include "svg.h"

#include <algorithm>
#include <map>

namespace transport_db {
    using namespace std::literals;
    struct RenderSettings {
        using Color = std::variant<std::monostate, std::string, svg::Rgb, svg::Rgba>;
        double width = 0.;
        double height = 0.;
        double padding = 0.;
        double line_width = 0.;
        double stop_radius = 0.;
        int bus_label_font_size = 0;
        double bus_label_offset[2] = { };
        int stop_label_font_size = 0;
        double stop_label_offset[2] = { };
        Color underlayer_color = {};
        double underlayer_width = 0.;
        std::vector<Color> color_palette;
    };
}

namespace map_renderer {

    using namespace std::literals;
    using namespace transport_db;
    using namespace domain;

    inline const double EPSILON = 1e-6;

    bool IsZero(double value);

    class SphereProjector {
    public:
        template <typename PointInputIt>
        SphereProjector(PointInputIt points_begin, PointInputIt points_end, double max_width, double max_height, double padding);

        svg::Point operator()(geo::Coordinates coordinates) const;
    private:
        double padding_;
        double min_lon_ = 0;
        double max_lat_ = 0;
        double zoom_coeff_ = 0;
    };

    class MapRenderer {
    public:
        MapRenderer() = default;

        void SetSettings(const transport_db::RenderSettings& settings);

        void SetStops(const std::map<std::string_view, const Stop*> stops);

        void SetRoutes(const std::map<std::string_view, const Bus*> routes);

        transport_db::RenderSettings GetRenderSettings() const;

        void Render(std::ostream& out_stream);

    private:
        svg::Document doc_;
        std::map<std::string_view, const Stop*> stops_;
        std::map<std::string_view, const Bus*> routes_;
        transport_db::RenderSettings settings_;

        struct BusSort {
            bool operator()(const Bus* lhs, const Bus* rhs) const
            {
                return std::lexicographical_compare(lhs->name.begin(), lhs->name.end(),
                    rhs->name.begin(), rhs->name.end());
            }
        };

        void RenderBusRoutes(SphereProjector& projector, const std::set<const Bus*, BusSort>& routes_to_render);
        void RenderRoutesNames(const SphereProjector& projector, const std::set<const Bus*, BusSort>& routes_to_render);
        void RenderStops(const SphereProjector& projector);
        void RenderStopsNames(const SphereProjector& projector);
    };


    template <typename PointInputIt>
    map_renderer::SphereProjector::SphereProjector(PointInputIt points_begin, PointInputIt points_end, double max_width, double max_height, double padding)
        : padding_(padding)
    {
        if (points_begin == points_end) {
            return;
        }

        const auto [left_it, right_it]
            = std::minmax_element(points_begin, points_end, [](auto lhs, auto rhs) {
            return lhs.lng < rhs.lng;
                });
        min_lon_ = left_it->lng;
        const double max_lon = right_it->lng;

        const auto [bottom_it, top_it]
            = std::minmax_element(points_begin, points_end, [](auto lhs, auto rhs) {
            return lhs.lat < rhs.lat;
                });
        const double min_lat = bottom_it->lat;
        max_lat_ = top_it->lat;

        std::optional<double> width_zoom;
        if (!IsZero(max_lon - min_lon_)) {
            width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
        }

        std::optional<double> height_zoom;
        if (!IsZero(max_lat_ - min_lat)) {
            height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
        }

        if (width_zoom && height_zoom) {
            zoom_coeff_ = std::min(*width_zoom, *height_zoom);
        }
        else if (width_zoom) {
            zoom_coeff_ = *width_zoom;
        }
        else if (height_zoom) {
            zoom_coeff_ = *height_zoom;
        }
    }
} // namespace map_renderer