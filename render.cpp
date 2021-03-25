#include "render.h"



Render::Render(const Descriptions::StopsDict& stops_dict,
       const Descriptions::BusesDict& buses_dict,
       const Json::Dict& render_settings_json)
    : render_settings(MakeRenderSettings(render_settings_json)),
      borders(FindBorders(stops_dict)) {
  if (borders.min_lat == borders.max_lat && borders.min_lon == borders.max_lon) {
    zoom_coef = 0;
  } else if (borders.min_lat != borders.max_lat && borders.min_lon != borders.max_lon){
    zoom_coef = std::min((render_settings.width - 2 * render_settings.padding) / (borders.max_lon - borders.min_lon),
                         (render_settings.height - 2 * render_settings.padding) / (borders.max_lat - borders.min_lat));
  } else if (borders.min_lat == borders.max_lat) {
    zoom_coef = (render_settings.width - 2 * render_settings.padding) / (borders.max_lon - borders.min_lon);
  } else {
    zoom_coef = (render_settings.height - 2 * render_settings.padding) / (borders.max_lat - borders.min_lat);
  }
  FillBusRoutes(buses_dict, stops_dict);
  FillStops(stops_dict);
}

const std::unordered_map<std::string, Render::Layer> Render::ConvertStrToLayer = {
    {"bus_lines", Render::Layer::BusLines},
    {"bus_labels", Render::Layer::BusLabels},
    {"stop_points", Render::Layer::StopPoints},
    {"stop_labels", Render::Layer::StopLabels}
};

  void  Render::RenderBusLines() {
    for (auto& polyline : polyline_layer) {
      doc_->Add(polyline);
    }
  }
  void  Render::RenderBusLabels() {
    for (auto& [underlayer, text] : routes_labels) {
      doc_->Add(underlayer);
      doc_->Add(text);
    }
  }
  void  Render::RenderStopPoints() {
    for (auto& circle : circle_layer) {
      doc_->Add(circle);
    }
  }
  void  Render::RenderStopLabels() {
    for (auto& [underlayer, text] : stop_names) {
      doc_->Add(underlayer);
      doc_->Add(text);
    }
  }

  void Render::FillSvgDocWithLayers() {
    for (auto layer : render_settings.layers) {
      if (layer == Layer::BusLines) {
        RenderBusLines();
      } else if (layer == Layer::BusLabels) {
        RenderBusLabels();
      } else if (layer == Layer::StopPoints) {
        RenderStopPoints();
      } else {
        RenderStopLabels();
      }
    }
  }


std::string Render::PrintMap() {
  if (!doc_) {
    doc_ = Svg::Document{};
    FillSvgDocWithLayers();
  }
  std::ostringstream os;
  doc_->Render(os);
  return os.str();
}

Svg::Color Render::ReadColorFromSettings(const Json::Node& node) {
  if (std::holds_alternative<std::vector<Json::Node>>(node.GetBase())) {
    Svg::Rgb rgb;
    auto& array = node.AsArray();
    if (array.size() == 4) {
      rgb = Svg::Rgb{array.at(0).AsInt(),
                     array.at(1).AsInt(),
                     array.at(2).AsInt(),
                     array.at(3).AsDouble()};
    } else {
      rgb = Svg::Rgb{array.at(0).AsInt(),
                     array.at(1).AsInt(),
                     array.at(2).AsInt()};
    }
    return Svg::Color(rgb);
  }
  return Svg::Color(node.AsString());
}


std::deque<Svg::Color> Render::ReadColorPaletteFromSettings(const Json::Node& node) {
  std::deque<Svg::Color> result;
  for (auto& color : node.AsArray()) {
    result.push_back(ReadColorFromSettings(color));
  }
  return result;
}

std::vector<Render::Layer> Render::FillLayersSettings(const Json::Node& node) {
  std::vector<Render::Layer> result;
  result.reserve(node.AsArray().size());
  for (auto& layer : node.AsArray()) {
    result.push_back(ConvertStrToLayer.at(layer.AsString()));
  }
  return result;
}

Render::RenderSettings Render::MakeRenderSettings(const Json::Dict& json) {
  return {
      json.at("width").AsDouble(),
      json.at("height").AsDouble(),
      json.at("padding").AsDouble(),
      json.at("stop_radius").AsDouble(),
      json.at("line_width").AsDouble(),
      json.at("stop_label_font_size").AsInt(),
      {json.at("stop_label_offset").AsArray()[0].AsDouble(),
       json.at("stop_label_offset").AsArray()[1].AsDouble()},
      json.at("underlayer_width").AsDouble(),
      json.at("bus_label_font_size").AsInt(),
      {json.at("bus_label_offset").AsArray()[0].AsDouble(),
       json.at("bus_label_offset").AsArray()[1].AsDouble()},
      ReadColorFromSettings(json.at("underlayer_color")),
      ReadColorPaletteFromSettings(json.at("color_palette")),
      FillLayersSettings(json.at("layers")),
  };
}

Render::Borders Render::FindBorders(const Descriptions::StopsDict& stops_dict) {
  Sphere::Point point = stops_dict.begin()->second->position;
  Borders result{point.latitude, point.latitude, point.longitude, point.longitude};
  for (auto& [stop_name, stop] : stops_dict) {
    auto stop_pos = stop->position;
    if (stop_pos.latitude < result.min_lat) { result.min_lat = stop_pos.latitude; }
    if (stop_pos.latitude > result.max_lat) { result.max_lat = stop_pos.latitude; }
    if (stop_pos.longitude < result.min_lon) { result.min_lon = stop_pos.longitude; }
    if (stop_pos.longitude > result.max_lon) { result.max_lon = stop_pos.longitude; }
  }
  return result;
}

Svg::Point Render::ConvertSpherePointToSvg(Sphere::Point point) const {
  return {(point.longitude - borders.min_lon) * zoom_coef + render_settings.padding,
          (borders.max_lat - point.latitude) * zoom_coef + render_settings.padding };
}

Render::TextWithUnderlayer Render::MakeStopText(const Descriptions::Stop& stop) {
  return {
      Svg::Text{}
          .SetPoint(ConvertSpherePointToSvg(stop.position))
          .SetOffset(render_settings.stop_label_offset)
          .SetFontSize(render_settings.stop_label_font_size)
          .SetFontFamily("Verdana")
          .SetFillColor(render_settings.underlayer_color)
          .SetStrokeColor(render_settings.underlayer_color)
          .SetStrokeWidth(render_settings.underlayer_width)
          .SetData(stop.name)
          .SetStrokeLineCap("round")
          .SetStrokeLineJoin("round"),
      Svg::Text{}
          .SetPoint(ConvertSpherePointToSvg(stop.position))
          .SetOffset(render_settings.stop_label_offset)
          .SetFontSize(render_settings.stop_label_font_size)
          .SetFontFamily("Verdana")
          .SetFillColor("black")
          .SetData(stop.name)
  };
}

Render::TextWithUnderlayer Render::MakeBusLabel(const Descriptions::Stop& stop,
                                                const Svg::Color& fill_color,
                                                const std::string& bus_name) {
  return {
      Svg::Text{}
          .SetPoint(ConvertSpherePointToSvg(stop.position))
          .SetOffset(render_settings.bus_label_offset)
          .SetFontSize(render_settings.bus_label_font_size)
          .SetFontFamily("Verdana")
          .SetFillColor(render_settings.underlayer_color)
          .SetStrokeColor(render_settings.underlayer_color)
          .SetStrokeWidth(render_settings.underlayer_width)
          .SetData(bus_name)
          .SetStrokeLineCap("round")
          .SetStrokeLineJoin("round")
          .SetFontWeight("bold"),
      Svg::Text{}
          .SetPoint(ConvertSpherePointToSvg(stop.position))
          .SetOffset(render_settings.bus_label_offset)
          .SetFontSize(render_settings.bus_label_font_size)
          .SetFontFamily("Verdana")
          .SetFillColor(fill_color)
          .SetData(bus_name)
          .SetFontWeight("bold")
  };
}

Svg::Circle Render::MakeStopCircle(const Descriptions::Stop& stop) {
  return Svg::Circle{}
      .SetFillColor("white")
      .SetRadius(render_settings.stop_radius)
      .SetCenter(ConvertSpherePointToSvg(stop.position));
}

void Render::FillStops(const Descriptions::StopsDict& stops_dict) {
  std::set<std::string> stops;
  for (auto& [name, stop] : stops_dict) {
    stops.insert(name);
  }
  stop_names.reserve(stops.size());
  circle_layer.reserve(stops.size());
  for (auto& stop_name : stops) {
    stop_names.push_back(MakeStopText(*stops_dict.at(stop_name)));
    circle_layer.push_back(MakeStopCircle(*stops_dict.at(stop_name)));
  }
}

void Render::FillBusRoutes(const Descriptions::BusesDict& buses_dict, const Descriptions::StopsDict& stops_dict) {
  std::set<std::string> buses;
  for (auto& [name, bus] : buses_dict) {
    buses.insert(name);
  }
  polyline_layer.reserve(buses_dict.size());
  routes_labels.reserve(buses_dict.size()*2);
  for (auto& bus : buses) {
    auto color = render_settings.color_palette.front();
    Svg::Polyline temp = Svg::Polyline{}
        .SetStrokeColor(color)
        .SetStrokeWidth(render_settings.line_width)
        .SetStrokeLineCap("round")
        .SetStrokeLineJoin("round");
    auto bus_ = buses_dict.at(bus);
    for (auto it = bus_->stops.begin(); it != bus_->stops.end(); ++it) {
      temp.AddPoint(ConvertSpherePointToSvg(stops_dict.at(*it)->position));
      if (it == bus_->stops.begin()) {
        routes_labels.push_back(MakeBusLabel(*stops_dict.at(*it), color, bus));
        if (*it != bus_->end) {
          routes_labels.push_back(MakeBusLabel(*stops_dict.at(bus_->end), color, bus));
        }
      }
    }
    render_settings.color_palette.pop_front();
    render_settings.color_palette.push_back(std::move(color));
    polyline_layer.push_back(std::move(temp));
  }
}
