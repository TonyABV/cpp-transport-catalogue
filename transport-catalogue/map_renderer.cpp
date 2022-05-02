#include "map_renderer.h"

using namespace std;

using namespace renderer;

struct RgbRgbaConvert {
    optional<svg::Rgb> operator()(vector<uint8_t> rgb) noexcept {
        return svg::Rgb{ rgb[0], rgb[1], rgb[2] };
    }
    optional<svg::Rgba> operator()(tuple<uint8_t, uint8_t, uint8_t, double> rgba)noexcept {
        return svg::Rgba{ get<0>(rgba), get<1>(rgba), get<2>(rgba), get<3>(rgba) };
    }
};

std::string renderer::ColorConvertToString(JsonColor c)
{
    stringstream out;
    if (holds_alternative<vector<uint8_t>>(c)) {
        out << svg::Rgb{ get<0>(c)[0], get<0>(c)[1], get<0>(c)[2] };
    }
    else {
        out << svg::Rgba{ get<0>(get<1>(c)), get<1>(get<1>(c)), get<2>(get<1>(c)), get<3>(get<1>(c)) };
    }
    return out.str();
}

domain::MaxMinLatLon FindMaxMinLatLon(const domain::Bus* busses) {
	double min_lat = (busses->stops_)[0]->coordinates_.lat;
	double min_lon = (busses->stops_)[0]->coordinates_.lng;
	double max_lat = min_lat;
	double max_lon = min_lon;
	for (size_t n = 1; n < (busses->stops_.size()); ++n) {
		min_lat = min(min_lat, (busses->stops_)[n]->coordinates_.lat);
		min_lon = min(min_lon, (busses->stops_)[n]->coordinates_.lng);
		max_lat = max(max_lat, (busses->stops_)[n]->coordinates_.lat);
		max_lon = max(max_lon, (busses->stops_)[n]->coordinates_.lng);
	}
	
	return make_pair(make_pair(max_lat, max_lon), make_pair(min_lat, min_lon));
}

domain::MaxMinLatLon ComputeMaxMin(std::deque<const domain::Bus*> busses)
{
    pair<double, double> max_lat_lon;
    pair<double, double> min_lat_lon;
    bool is_first = true;
    for (const domain::Bus* bus : busses) {
        auto max_min_lat_lon = FindMaxMinLatLon(bus);
        if (is_first) {
            max_lat_lon.first = max_min_lat_lon.first.first;
            max_lat_lon.second = max_min_lat_lon.first.second;
            min_lat_lon.first = max_min_lat_lon.second.first;
            min_lat_lon.second = max_min_lat_lon.second.second;
            is_first = false;
            continue;
        }
        max_lat_lon.first = max(max_lat_lon.first, max_min_lat_lon.first.first);
        max_lat_lon.second = max(max_lat_lon.second, max_min_lat_lon.first.second);
        min_lat_lon.first = min(min_lat_lon.first, max_min_lat_lon.second.first);
        min_lat_lon.second = min(min_lat_lon.second, max_min_lat_lon.second.second);
    }
    return make_pair(max_lat_lon, min_lat_lon);
}


double MapRenderer::ComputeZoomCoef(const domain::MaxMinLatLon& max_min_lat_lot) const {

    double height_max_zoom = (settings_.height - 2 * settings_.padding) / (max_min_lat_lot.first.first - max_min_lat_lot.second.first);
    double width_max_zoom = (settings_.width - 2 * settings_.padding) / (max_min_lat_lot.first.second - max_min_lat_lot.second.second);

    if (height_max_zoom == 0) {
        return width_max_zoom;
    }
    else if (width_max_zoom == 0) {
        return height_max_zoom;
    }
    else {
        return min(width_max_zoom, height_max_zoom);
    }
}

svg::Polyline renderer::MapRenderer::CreateRouteLine(const domain::Bus* bus, double zoom_co,
                                                                   double max_lat, double min_lon) const
{
    svg::Polyline route;
    for (const domain::Stop* stop : (*bus).stops_) {
        double x = (stop->coordinates_.lng - min_lon) * zoom_co + settings_.padding;
        double y = (max_lat - stop->coordinates_.lat) * zoom_co + settings_.padding;
        svg::Point point{ x, y };
        route.AddPoint(point);
    }
    return route;
}

void renderer::MapRenderer::AddLines(svg::Document& doc, const std::deque<const domain::Bus*>& busses,
                                                                double zoom_co, double max_lat, double min_lon) const
{
    size_t color_index = 0;
    for (const domain::Bus* bus : busses) {
        doc.Add(CreateRouteLine(bus, zoom_co, max_lat, min_lon)
            .SetStrokeColor(settings_.color_palette[color_index++]).SetFillColor("none").SetStrokeWidth(settings_.line_width)
            .SetStrokeLineCap(svg::StrokeLineCap::ROUND).SetStrokeLineJoin(svg::StrokeLineJoin::ROUND));

        if (color_index == settings_.color_palette.size()) {
            color_index = 0;
        }
    }
}

svg::Text renderer::MapRenderer::CreateBusName(const domain::Stop* stop, double zoom_co,
                                                                double max_lat, double min_lon) const
{
    svg::Text name;
    double x = (stop->coordinates_.lng - min_lon) * zoom_co + settings_.padding;
    double y = (max_lat - stop->coordinates_.lat) * zoom_co + settings_.padding;
    svg::Point point{ x, y };
    name.SetPosition(point);
    return name;
}

void renderer::MapRenderer::AddBusNames(svg::Document& doc, const std::deque<const domain::Bus*>& busses, double zoom_co,
                                                                                      double max_lat, double min_lon) const
{
    size_t color_index = 0;
    for (const domain::Bus* bus : busses) {
        doc.Add(CreateBusName((*bus).stops_[0], zoom_co, max_lat, min_lon)
            .SetOffset(svg::Point{ settings_.bus_label_offset.first, settings_.bus_label_offset.second })
            .SetFontSize(settings_.bus_label_font_size).SetFontFamily("Verdana").SetFontWeight("bold")
            .SetStrokeColor(settings_.underlayer_color).SetFillColor(settings_.underlayer_color)
            .SetStrokeWidth(settings_.underlayer_width)
            .SetStrokeLineCap(svg::StrokeLineCap::ROUND).SetStrokeLineJoin(svg::StrokeLineJoin::ROUND).SetData(bus->name_));

        doc.Add(CreateBusName((*bus).stops_[0], zoom_co, max_lat, min_lon)
            .SetOffset(svg::Point{ settings_.bus_label_offset.first, settings_.bus_label_offset.second })
            .SetFontSize(settings_.bus_label_font_size).SetFontFamily("Verdana").SetFontWeight("bold")
            .SetFillColor(settings_.color_palette[color_index]).SetData(bus->name_));

        if (!bus->route_is_circular_) {
            const domain::Stop* final_stop = (bus->stops_[bus->stops_.size() / 2]);
            if (final_stop->name_ != bus->stops_.front()->name_) {
            doc.Add(CreateBusName(final_stop, zoom_co, max_lat, min_lon)
                .SetOffset(svg::Point{ settings_.bus_label_offset.first, settings_.bus_label_offset.second })
                .SetFontSize(settings_.bus_label_font_size).SetFontFamily("Verdana").SetFontWeight("bold")
                .SetStrokeColor(settings_.underlayer_color).SetFillColor(settings_.underlayer_color)
                .SetStrokeWidth(settings_.underlayer_width)
                .SetStrokeLineCap(svg::StrokeLineCap::ROUND).SetStrokeLineJoin(svg::StrokeLineJoin::ROUND).SetData(bus->name_));

            doc.Add(CreateBusName(final_stop, zoom_co, max_lat, min_lon)
                .SetOffset(svg::Point{ settings_.bus_label_offset.first, settings_.bus_label_offset.second })
                .SetFontSize(settings_.bus_label_font_size).SetFontFamily("Verdana").SetFontWeight("bold")
                .SetFillColor(settings_.color_palette[color_index]).SetData(bus->name_));
            }
        }
        ++color_index;
        if (color_index == settings_.color_palette.size()) {
            color_index = 0;
        }
    }
}

svg::Circle renderer::MapRenderer::CreateToken(const domain::Stop* stop, double zoom_co, double max_lat, double min_lon) const
{

    svg::Circle token;
    double x = (stop->coordinates_.lng - min_lon) * zoom_co + settings_.padding;
    double y = (max_lat - stop->coordinates_.lat) * zoom_co + settings_.padding;
    svg::Point point{ x, y };
    token.SetCenter(point);
    return token;
}

void renderer::MapRenderer::AddTokens(svg::Document& doc, const std::deque<const domain::Bus*>& busses, double zoom_co, double max_lat, double min_lon) const
{
    set<domain::Stop*, domain::StopCmp> stops;
    for (const domain::Bus* bus : busses) {
        stops.insert(bus->unique_stops_.begin(), bus->unique_stops_.end());
    }
    for (domain::Stop* stop : stops) {
        doc.Add(CreateToken(stop, zoom_co, max_lat, min_lon)
            .SetRadius(settings_.stop_radius)
            .SetFillColor("white"));
    }
}

svg::Text renderer::MapRenderer::CreateStopName(const domain::Stop* stop, double zoom_co, double max_lat, double min_lon) const
{
    svg::Text stop_name;
    double x = (stop->coordinates_.lng - min_lon) * zoom_co + settings_.padding;
    double y = (max_lat - stop->coordinates_.lat) * zoom_co + settings_.padding;
    svg::Point point{ x, y };
    stop_name.SetPosition(point);
    return stop_name;
}

void renderer::MapRenderer::AddStopNames(svg::Document& doc, const std::deque<const domain::Bus*>& busses, double zoom_co, double max_lat, double min_lon) const
{
    set<domain::Stop*, domain::StopCmp> stops;
    for (const domain::Bus* bus : busses) {
        stops.insert(bus->unique_stops_.begin(), bus->unique_stops_.end());
    }
    for (domain::Stop* stop : stops) {
        doc.Add(CreateStopName(stop, zoom_co, max_lat, min_lon)
            .SetStrokeWidth(settings_.underlayer_width)
            .SetOffset(svg::Point{ settings_.stop_label_offset.first, settings_.stop_label_offset.second })
            .SetFontSize(settings_.stop_label_font_size)
            .SetFontFamily("Verdana")
            .SetData(stop->name_)
            .SetFillColor(settings_.underlayer_color)
            .SetStrokeColor(settings_.underlayer_color)
            .SetStrokeLineCap(svg::StrokeLineCap::ROUND).SetStrokeLineJoin(svg::StrokeLineJoin::ROUND));

        doc.Add(CreateStopName(stop, zoom_co, max_lat, min_lon)
            .SetOffset(svg::Point{ settings_.stop_label_offset.first, settings_.stop_label_offset.second })
            .SetFontSize(settings_.stop_label_font_size)
            .SetFontFamily("Verdana")
            .SetFillColor("black")
            .SetData(stop->name_));
    }
}

std::string renderer::MapRenderer::CreateMap(deque<const domain::Bus*> busses) const
{
    domain::MaxMinLatLon max_min_lat_lon = ComputeMaxMin(busses);
    double zoom_co = MapRenderer::ComputeZoomCoef(max_min_lat_lon);

    sort(busses.begin(), busses.end(), 
        [](const domain::Bus* lhs, const domain::Bus* rhs) {return (*lhs).name_ < (*rhs).name_;});

    svg::Document doc;
    AddLines(doc, busses, zoom_co, max_min_lat_lon.first.first, max_min_lat_lon.second.second);    
    AddBusNames(doc, busses, zoom_co, max_min_lat_lon.first.first, max_min_lat_lon.second.second);
    AddTokens(doc, busses, zoom_co, max_min_lat_lon.first.first, max_min_lat_lon.second.second);
    AddStopNames(doc, busses, zoom_co, max_min_lat_lon.first.first, max_min_lat_lon.second.second);

    stringstream out;
    doc.Render(out);

    return out.str();
}

renderer::MapRenderer::MapRenderer(const renderer::RenderSettings& set):settings_(set){}
