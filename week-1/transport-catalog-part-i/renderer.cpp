#include "renderer.h"

#include <algorithm>
#include <map>
#include <stdexcept>

namespace {

Svg::Point PointFromJsonArray(const Json::Array& nodes) {
  return {
      .x = nodes.at(0).AsDouble(),
      .y = nodes.at(1).AsDouble(),
  };
}

template <typename T>
Svg::Color ColorFrom(const T& raw_color) {
  throw std::invalid_argument("invalid color type");
}

template <>
Svg::Color ColorFrom<std::string>(const std::string& raw_color) {
  return raw_color;
}

template <>
Svg::Color ColorFrom<Json::Array>(const Json::Array& raw_color) {
  Svg::Rgb rgb{.red = raw_color.at(0).AsInt(),
               .green = raw_color.at(1).AsInt(),
               .blue = raw_color.at(2).AsInt()};
  if (raw_color.size() == 4) {
    return Svg::Rgba{.red = rgb.red,
                     .green = rgb.green,
                     .blue = rgb.blue,
                     .alpha = raw_color.at(3).AsDouble()};
  }
  return rgb;
}

Svg::Color ColorFromJsonNode(const Json::Node& node) {
  return std::visit([](const auto& raw_color) { return ColorFrom(raw_color); },
                    node.GetBase());
}

std::vector<Svg::Color> ColorPaletteFromJsonArray(const Json::Array& nodes) {
  std::vector<Svg::Color> palette;
  palette.reserve(nodes.size());
  for (const auto& node : nodes) {
    palette.push_back(ColorFromJsonNode(node));
  }
  return palette;
}

template <typename T>
[[maybe_unused]] std::vector<T> TypifyJsonArray(const Json::Array& arr);

template <>
std::vector<std::string> TypifyJsonArray<std::string>(const Json::Array& arr) {
  std::vector<std::string> res;
  res.reserve(arr.size());
  for (const auto& node : arr) {
    res.push_back(node.AsString());
  }
  return res;
}

}  // namespace

Renderer::Renderer(const Descriptions::StopsDict& stops_dict,
                   const Descriptions::BusesDict& buses_dict,
                   const Json::Dict& json)
    : render_settings_(MakeRenderSettings(json)) {
  double min_lat = std::numeric_limits<double>::max();
  double min_lon = std::numeric_limits<double>::max();
  double max_lat = std::numeric_limits<double>::min();
  double max_lon = std::numeric_limits<double>::min();
  for (const auto& [stop, stop_info] : stops_dict) {
    min_lat = std::min(min_lat, stop_info->position.latitude);
    max_lat = std::max(max_lat, stop_info->position.latitude);
    min_lon = std::min(min_lon, stop_info->position.longitude);
    max_lon = std::max(max_lon, stop_info->position.longitude);
  }
  const double zoom_coef = [&] {
    std::vector<double> coefs;
    if (const auto delta = max_lon - min_lon; std::abs(delta) > 1e-9) {
      coefs.push_back((render_settings_.width - 2 * render_settings_.padding) /
                      delta);
    }
    if (const auto delta = max_lat - min_lat; std::abs(delta) > 1e-9) {
      coefs.push_back((render_settings_.height - 2 * render_settings_.padding) /
                      delta);
    }
    if (coefs.empty()) {
      return 0.0;
    }
    std::sort(coefs.begin(), coefs.end());
    return coefs.front();
  }();

  std::map<std::string, Svg::Point> stop_to_point{};
  for (const auto& [stop, stop_info] : stops_dict) {
    stop_to_point[stop] =
        Svg::Point{.x = (stop_info->position.longitude - min_lon) * zoom_coef +
                        render_settings_.padding,
                   .y = (max_lat - stop_info->position.latitude) * zoom_coef +
                        render_settings_.padding};
  }

  std::vector<std::string> buses;
  buses.reserve(buses_dict.size());
  for (const auto& [bus, bus_info] : buses_dict) {
    buses.push_back(bus);
  }
  std::sort(buses.begin(), buses.end());
  std::unordered_map<std::string, Svg::Color> bus_to_color;
  bus_to_color.reserve(buses.size());
  for (int i = 0; i < buses.size(); ++i) {
    bus_to_color[buses[i]] = render_settings_.color_palette.at(
        i % render_settings_.color_palette.size());
  }

  Svg::Document document{};
  for (const auto& layer : render_settings_.layers) {
    if (layer == "bus_lines") {
      RenderBusLines(buses_dict, stop_to_point, buses, bus_to_color, document);
    } else if (layer == "bus_labels") {
      RenderBusLabels(buses_dict, stop_to_point, buses, bus_to_color, document);
    } else if (layer == "stop_points") {
      RenderStopPoints(stop_to_point, document);
    } else if (layer == "stop_labels") {
      RenderStopLabels(stop_to_point, document);
    } else {
      throw std::invalid_argument("invalid layer: " + layer);
    }
  }

  std::stringstream stream;
  document.Render(stream);
  result_ = stream.str();
}
void Renderer::RenderStopLabels(
    const std::map<std::string, Svg::Point>& stop_to_point,
    Svg::Document& document) const {
  for (const auto& [stop, point] : stop_to_point) {
    const auto& pt = point;
    const auto& st = stop;
    const auto makeBaseText = [&] {
      return Svg::Text{}
          .SetPoint(pt)
          .SetOffset(render_settings_.stop_label_offset)
          .SetFontSize(render_settings_.stop_label_font_size)
          .SetFontFamily("Verdana")
          .SetData(st);
    };
    document.Add(makeBaseText()
                     .SetFillColor(render_settings_.underlayer_color)
                     .SetStrokeColor(render_settings_.underlayer_color)
                     .SetStrokeWidth(render_settings_.underlayer_width)
                     .SetStrokeLineCap("round")
                     .SetStrokeLineJoin("round"));
    document.Add(makeBaseText().SetFillColor("black"));
  }
}
void Renderer::RenderStopPoints(
    const std::map<std::string, Svg::Point>& stop_to_point,
    Svg::Document& document) const {
  for (const auto& [stop, point] : stop_to_point) {
    document.Add(Svg::Circle{}
                     .SetCenter(point)
                     .SetRadius(render_settings_.stop_radius)
                     .SetFillColor("white"));
  }
}

void Renderer::RenderBusLines(
    const Descriptions::BusesDict& buses_dict,
    const std::map<std::string, Svg::Point>& stop_to_point,
    const std::vector<std::string>& buses,
    const std::unordered_map<std::string, Svg::Color>& bus_to_color,
    Svg::Document& document) const {
  for (const auto& bus : buses) {
    const auto& info = buses_dict.at(bus);
    Svg::Polyline polyline{};
    for (const auto& stop : info->stops) {
      polyline.AddPoint(stop_to_point.at(stop));
    }
    document.Add(polyline.SetStrokeColor(bus_to_color.at(bus))
                     .SetStrokeWidth(render_settings_.line_width)
                     .SetStrokeLineCap("round")
                     .SetStrokeLineJoin("round"));
  }
}

void Renderer::RenderBusLabels(
    const Descriptions::BusesDict& buses_dict,
    const std::map<std::string, Svg::Point>& stop_to_point,
    const std::vector<std::string>& buses,
    const std::unordered_map<std::string, Svg::Color>& bus_to_color,
    Svg::Document& document) const {
  for (const auto& bus : buses) {
    const auto& info = buses_dict.at(bus);
    const auto makeTextBase = [&](const std::string& stop) {
      return Svg::Text{}
          .SetPoint(stop_to_point.at(stop))
          .SetOffset(render_settings_.bus_label_offset)
          .SetFontSize(render_settings_.bus_label_font_size)
          .SetFontFamily("Verdana")
          .SetFontWeight("bold")
          .SetData(bus);
    };
    const auto makeTextUnderlayer = [&](const std::string& stop) {
      return makeTextBase(stop)
          .SetFillColor(render_settings_.underlayer_color)
          .SetStrokeColor(render_settings_.underlayer_color)
          .SetStrokeWidth(render_settings_.underlayer_width)
          .SetStrokeLineCap("round")
          .SetStrokeLineJoin("round");
    };
    const auto makeText = [&](const std::string& stop) {
      return makeTextBase(stop).SetFillColor(bus_to_color.at(bus));
    };
    document.Add(makeTextUnderlayer(info->stops.front()));
    document.Add(makeText(info->stops.front()));
    const auto& mid = info->stops.at(info->stops.size() / 2);
    if (!info->is_roundtrip && mid != info->stops.front()) {
      document.Add(makeTextUnderlayer(mid));
      document.Add(makeText(mid));
    }
  }
}

// static
Renderer::RenderSettings Renderer::MakeRenderSettings(const Json::Dict& json) {
  return {
      .width = json.at("width").AsDouble(),
      .height = json.at("height").AsDouble(),
      .padding = json.at("padding").AsDouble(),
      .stop_radius = json.at("stop_radius").AsDouble(),
      .line_width = json.at("line_width").AsDouble(),
      .stop_label_font_size =
          static_cast<uint32_t>(json.at("stop_label_font_size").AsInt()),
      .stop_label_offset =
          PointFromJsonArray(json.at("stop_label_offset").AsArray()),
      .underlayer_color = ColorFromJsonNode(json.at("underlayer_color")),
      .underlayer_width = json.at("underlayer_width").AsDouble(),
      .color_palette =
          ColorPaletteFromJsonArray(json.at("color_palette").AsArray()),
      .bus_label_font_size =
          static_cast<uint32_t>(json.at("bus_label_font_size").AsInt()),
      .bus_label_offset =
          PointFromJsonArray(json.at("bus_label_offset").AsArray()),
      .layers = TypifyJsonArray<std::string>(json.at("layers").AsArray()),
  };
}
