#include "svg.h"

namespace svg {

using namespace std::literals;

std::ostream& operator<<(std::ostream& out, const svg::StrokeLineCap& linecap_type) {
    switch (linecap_type)
    {
    case svg::StrokeLineCap::BUTT:
        out << "butt";
        return out;
    case svg::StrokeLineCap::ROUND:
        out << "round";
        return out;
    case svg::StrokeLineCap::SQUARE:
        out << "square";
        return out;
    default:
        return out;
    }
}

std::ostream& operator<<(std::ostream& out, const svg::StrokeLineJoin& linejoin_type)
{
    switch (linejoin_type)
    {
    case svg::StrokeLineJoin::ARCS:
        out << "arcs";
        return out;
    case svg::StrokeLineJoin::BEVEL:
        out << "bevel";
        return out;
    case svg::StrokeLineJoin::MITER:
        out << "miter";
        return out;
    case svg::StrokeLineJoin::MITER_CLIP:
        out << "miter-clip";
        return out;
    case svg::StrokeLineJoin::ROUND:
        out << "round";
        return out;
    default:
        return out;;
    }
}


void Object::Render(const RenderContext& context) const {
    context.RenderIndent();

    // Делегируем вывод тега своим подклассам
    RenderObject(context);

    context.out << std::endl;
}

// ---------- Circle ------------------

Circle& Circle::SetCenter(Point center)  {
    center_ = center;
    return *this;
}

Circle& Circle::SetRadius(double radius)  {
    radius_ = radius;
    return *this;
}

void Circle::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << " <circle";
    out << " cx = \""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
    out << "r=\""sv << radius_ << "\""sv;
    RenderAttrs(out);
    out << "/>"sv;
}

// ---------- Polyline ------------------
Polyline& Polyline::AddPoint(Point point)
{
    points_.push_back(point);
    return *this;
}

void Polyline::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << " <polyline";
    out << " points = \""sv;
    bool is_first = true;
    for (auto& point : points_) {
        if (is_first) {
            out << point.x << ","sv << point.y ;
            is_first = false;
            continue;
        }
        out << " "sv << point.x << ","sv << point.y;
    }
    out << "\" "sv;
    RenderAttrs(out);
    out << "/>"sv;
}

// ---------- Text ------------------
Text& Text::SetPosition(Point pos)
{
    position_ = pos;
    return *this;
}

Text& Text::SetOffset(Point offset)
{
    offset_ = offset;
    return *this;
}

Text& Text::SetFontSize(uint32_t size)
{
    font_size_ = size;
    return *this;
}

Text& Text::SetFontFamily(std::string font_family)
{
    font_family_ = font_family;
    return *this;
}

Text& Text::SetFontWeight(std::string font_weight)
{
    font_weight_ = font_weight;
    return *this;
}

//!!
Text& Text::SetData(std::string data)
{
    text_ = data;
    return *this;
}

Text& Text::SetData(std::string_view data)
{
    text_ = data;
    return *this;
}

void Text::RenderObject(const RenderContext& context) const
{
    auto& out = context.out;
    out << " <text";
    RenderAttrs(out);
    out << " x=\""sv << position_.x << "\" y=\""sv << position_.y << "\""sv;
    out << " dx=\""sv << offset_.x << "\" dy=\""sv << offset_.y << "\""sv;
    out << " font-size=\""sv << font_size_ << "\""sv;
    if (font_family_) {
        out << " font-family=\""sv << font_family_.value() << "\""sv;
    }
    if (font_weight_) {
        out << " font-weight=\""sv << font_weight_.value() << "\""sv;
    }
    out << ">";
    for (auto cha : text_) {
        auto find_res = dif_.find(cha);
        if (find_res != dif_.end()) {
            out << (*find_res).second;
            continue;
        }
        out << cha;
    }
    out << "</text>"sv;
}

void Document::AddPtr(std::unique_ptr<Object>&& obj)
{
    objects_.emplace_back(move(obj));
}

void Document::Render(std::ostream& out) const
{
    out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << std::endl;
    out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv << std::endl;
    for (auto& obj : objects_) {
        if (obj) {
            obj.get()->Render(out);
        }
    }
    out << "</svg>"sv;
}



void ColorConvert::operator()(std::monostate) const
{
    out << "none";
}

void ColorConvert::operator()(std::string color)const
{
    out << color;
}

void ColorConvert::operator()(svg::Rgb color)const
{
    out << "rgb("s << std::to_string(color.red) << ","s << std::to_string(color.green) << ","s << std::to_string(color.blue) << ")"s;
}

void ColorConvert::operator()(svg::Rgba color)const
{
    out << "rgba("s << std::to_string(color.red) << ","s << std::to_string(color.green) << ","s << std::to_string(color.blue) << ","s << color.opacity << ")"s;
}
std::ostream& operator<<(std::ostream& out, const svg::Color& color)
{
    std::visit(svg::ColorConvert{ out }, color);
    return out;
}
}  // namespace svg