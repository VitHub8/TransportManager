#pragma once

#include <iostream>
#include <cstdint>
#include <string>
#include <vector>
#include <optional>
#include <unordered_map>
#include <string_view>


namespace Svg {

  struct Point {
    double x = 0;
    double y = 0;
    std::string ToString() const;
  };

  struct Rgb {
    int red = 0;
    int green = 0;
    int blue = 0;
    std::optional<double> alpha = std::nullopt;
    std::string ToString() const;
  };

  struct Color {
    Color();
    Color(std::string&& color);
    Color(const char* c);
    Color(std::string color);
    Color(Rgb rgb);
    std::string color_;
  };

  const Color NoneColor = Color();

  class Node {
  public:
    Node();
    Node(std::string name, std::unordered_map<std::string, std::string> attrs);

    const std::vector<Node>& Children() const;
    void AddChild(Node node);

    void Out(std::ostream& os) const;

  private:
    std::string name;
    std::vector<Node> children;
    std::unordered_map<std::string, std::string> attrs;
  };


  template <class Derived>
  class Object {
  public:
    Derived& SetFillColor(const Color& color) {
      fill_color = color;
      return AsDerived();
    }
    Derived& SetStrokeColor(const Color& color) {
      stroke_color = color;
      return AsDerived();
    }
    Derived& SetStrokeWidth(double width) {
      stroke_width = width;
      return AsDerived();
    }
    Derived& SetStrokeLineCap(const std::string& cap) {
      line_cap = cap;
      return AsDerived();
    }
    Derived& SetStrokeLineJoin(const std::string& join) {
      line_join = join;
      return AsDerived();
    }

    virtual Node MakeNode() const = 0;

  protected:
    Derived& AsDerived() {
      return *static_cast<Derived*>(this);
    }

    Color stroke_color = NoneColor;
    Color fill_color = NoneColor;
    double stroke_width = 1.0;
    std::optional<std::string> line_cap = std::nullopt;
    std::optional<std::string> line_join = std::nullopt;
  };

  class Circle : public Object<Circle> {
  public:
    Circle& SetCenter(Point point);
    Circle& SetRadius(double radius);

    Node MakeNode() const override;

  private:
    Point center = Point{0,0};
    double R = 1.0;
  };

  class Polyline : public Object<Polyline> {
  public:
    Polyline& AddPoint(Point point);

    Node MakeNode() const override;

  private:
    std::vector<Point> vertices;
  };

  class Text : public Object<Text> {
  public:
    Text& SetPoint(Point p);
    Text& SetOffset(Point off);
    Text& SetFontSize(uint32_t size);
    Text& SetFontFamily(const std::string& style);
    Text& SetFontWeight(const std::string& weight);
    Text& SetData(const std::string& data_);

    Node MakeNode() const override;

  private:
    std::string data;
    Point point;
    Point offset;
    uint32_t font_size = 1;
    std::optional<std::string> font_style = std::nullopt;
    std::optional<std::string> font_weight = std::nullopt;
  };

  class Document {
  public:
    Document();
    explicit Document(Node root);

    template<class Obj>
    Document& Add(const Obj& obj) {
      root.AddChild(obj.MakeNode());
      return *this;
    }

    template<class Obj>
    Document& Add(Obj&& obj) {
      root.AddChild(obj.MakeNode());
      return *this;
    }

    void Render(std::ostream& os) const;

  private:
    Node root;
  };


}