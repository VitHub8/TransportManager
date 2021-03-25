#pragma once

#include "svg.h"
#include "sphere.h"
#include "descriptions.h"
#include <vector>
#include <set>
#include <deque>
#include <optional>
#include <utility>
#include <sstream>

class Render {
private:
  enum class Layer{
    BusLines,
    BusLabels,
    StopPoints,
    StopLabels
  };

  struct RenderSettings {
    double width;
    double height;
    double padding;
    double stop_radius;
    double line_width;
    int stop_label_font_size;
    Svg::Point stop_label_offset;
    double underlayer_width;
    int bus_label_font_size;
    Svg::Point bus_label_offset;
    Svg::Color underlayer_color = Svg::NoneColor;
    std::deque<Svg::Color> color_palette;
    std::vector<Layer> layers;
  };

  struct TextWithUnderlayer {
    Svg::Text underlayer;
    Svg::Text text;
  };

  struct Borders {
    double min_lat;
    double max_lat;
    double min_lon;
    double max_lon;
  };

public:
  Render(const Descriptions::StopsDict& stops_dict,
         const Descriptions::BusesDict& buses_dict,
         const Json::Dict& render_settings_json);

  std::string PrintMap();

private:
  RenderSettings render_settings;
  Borders borders;
  double zoom_coef = 0;

  std::vector<Svg::Polyline> polyline_layer;
  std::vector<TextWithUnderlayer> routes_labels;
  std::vector<Svg::Circle> circle_layer;
  std::vector<TextWithUnderlayer> stop_names;

  std::optional<Svg::Document> doc_ = std::nullopt;

private:
  static const std::unordered_map<std::string, Layer> ConvertStrToLayer;
  void RenderBusLines();
  void RenderBusLabels();
  void RenderStopPoints();
  void RenderStopLabels();
  void FillSvgDocWithLayers();
  static Svg::Color ReadColorFromSettings(const Json::Node& node);
  static std::deque<Svg::Color> ReadColorPaletteFromSettings(const Json::Node& node);
  static std::vector<Layer> FillLayersSettings(const Json::Node& node);
  static RenderSettings MakeRenderSettings(const Json::Dict& json);
  static Borders FindBorders(const Descriptions::StopsDict& stops_dict);
  Svg::Point ConvertSpherePointToSvg(Sphere::Point point) const;
  TextWithUnderlayer MakeStopText(const Descriptions::Stop& stop);
  TextWithUnderlayer MakeBusLabel(const Descriptions::Stop& stop, const Svg::Color& fill_color, const std::string& bus_name);
  Svg::Circle MakeStopCircle(const Descriptions::Stop& stop);
  void FillBusRoutes(const Descriptions::BusesDict& buses_dict, const Descriptions::StopsDict& stops_dict);
  void FillStops(const Descriptions::StopsDict& stops_dict);
};