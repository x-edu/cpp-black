#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "descriptions.h"
#include "json.h"
#include "svg.h"

class Renderer {
 public:
  explicit Renderer(const Descriptions::StopsDict& stops_dict,
                    const Descriptions::BusesDict& buses_dict,
                    const Json::Dict& json);

  [[nodiscard]] const std::string& GetResult() {
#ifdef TESTS
    std::clog << result_ << std::endl;
#endif
    return result_;
  }

 private:
  struct RenderSettings {
    double width;
    double height;
    double padding;
    double stop_radius;
    double line_width;
    uint32_t stop_label_font_size;
    Svg::Point stop_label_offset;
    Svg::Color underlayer_color;
    double underlayer_width;
    std::vector<Svg::Color> color_palette;
    uint32_t bus_label_font_size;
    Svg::Point bus_label_offset;
    std::vector<std::string> layers;
  };

 private:
  static RenderSettings MakeRenderSettings(const Json::Dict& json);

  void RenderBusLines(
      const Descriptions::BusesDict& buses_dict,
      const std::map<std::string, Svg::Point>& stop_to_point,
      const std::vector<std::string>& buses,
      const std::unordered_map<std::string, Svg::Color>& bus_to_color,
      Svg::Document& document) const;

  void RenderBusLabels(
      const Descriptions::BusesDict& buses_dict,
      const std::map<std::string, Svg::Point>& stop_to_point,
      const std::vector<std::string>& buses,
      const std::unordered_map<std::string, Svg::Color>& bus_to_color,
      Svg::Document& document) const;

  void RenderStopPoints(const std::map<std::string, Svg::Point>& stop_to_point,
                        Svg::Document& document) const;

  void RenderStopLabels(const std::map<std::string, Svg::Point>& stop_to_point,
                        Svg::Document& document) const;

 private:
  RenderSettings render_settings_;
  std::string result_;
};
