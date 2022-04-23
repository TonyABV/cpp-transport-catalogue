#pragma once
#include <algorithm>
#include <deque>
#include <optional>
#include <set>
#include <sstream>
#include <string>
#include <tuple>
#include <utility>
#include <variant>
#include <vector>

#include "domain.h"
#include "svg.h"

namespace renderer {
struct RenderSettings
{
	double width;
	double height;
	double padding;
	double line_width;
	double stop_radius;
	size_t bus_label_font_size;
	std::pair<double, double> bus_label_offset;
	std::pair<double, double> stop_label_offset;
	size_t stop_label_font_size;
	std::string underlayer_color;
	double underlayer_width;
	std::vector<std::string> color_palette;
};


using j_Color = std::variant<std::vector<uint8_t>, std::tuple<uint8_t, uint8_t, uint8_t, double>>;

std::string ColorConvertToString(j_Color c);


class MapRenderer
{
public:
	MapRenderer(const RenderSettings& set);
	std::string CreateMap(domain::MaxMinLatLon info, std::deque<const domain::Bus*> busses) const;
private:
	double ComputeZoomCoef(const domain::MaxMinLatLon& max_min_lat_lot) const;

	svg::Polyline CreateRouteLine(const domain::Bus* bus, double zoom_co, double max_lat, double min_lon)const;
	void AddLines(svg::Document& doc, const std::deque<const domain::Bus*>& busses,
														double zoom_co, double max_lat, double min_lon)const;

	svg::Text CreateBusName(const domain::Stop* stop, double zoom_co,
		double max_lat, double min_lon) const;
	void AddBusNames(svg::Document& doc, const std::deque<const domain::Bus*>& busses, double zoom_co,
		double max_lat, double min_lon) const;

	svg::Circle CreateToken(const domain::Stop* stop, double zoom_co,
		double max_lat, double min_lon) const;
	void AddTokens (svg::Document& doc, const std::deque<const domain::Bus*>& busses, double zoom_co,
		double max_lat, double min_lon) const;

	svg::Text CreateStopName(const domain::Stop* stop, double zoom_co,
		double max_lat, double min_lon) const;
	void AddStopNames(svg::Document& doc, const std::deque<const domain::Bus*>& busses, double zoom_co,
		double max_lat, double min_lon) const;

private:
	const RenderSettings& settings_;
};
}