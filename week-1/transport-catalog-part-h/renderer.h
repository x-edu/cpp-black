#pragma once

#include <cstdint>
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
  };

  static RenderSettings MakeRenderSettings(const Json::Dict& json);

 private:
  RenderSettings render_settings_;
  std::string result_;
};
