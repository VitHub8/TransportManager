#include "svg.h"
#include <sstream>
#include <iomanip>


namespace Svg {


    std::string Point::ToString() const {
      std::ostringstream os;
      os << std::setprecision(15) << x << "," << std::setprecision(15) << y;
      return os.str();
    }


    std::string Rgb::ToString() const {
        return (alpha ? "rgba(" : "rgb(")
                      + std::to_string(red) + ","
                      + std::to_string(green) + ","
                      + std::to_string(blue)
                      + (alpha ? "," + std::to_string(*alpha) : "")
                + ")";
    }


  Color::Color() : color_("none") {}
  Color::Color(std::string&& color) : color_(color) {}
  Color::Color(const char* c) : color_(c) {}
  Color::Color(std::string color) : color_(move(color)){}
  Color::Color(Rgb rgb) : color_(rgb.ToString()) {}


  Node::Node() : name("svg"), attrs({}) {}
  Node::Node(std::string name, std::unordered_map<std::string, std::string> attrs)
      : name(move(name)), attrs(move(attrs)) {}

    const std::vector<Node>& Node::Children() const {
        return children;
    }
    void Node::AddChild(Node node) {
        children.push_back(std::move(node));
    }

    void Node::Node::Out(std::ostream& os) const {
        os << "<" << name << " ";
        for (auto& [attr, value] : attrs) {
            if (attr != "data") {
                os << attr << "=" << value << " ";
            }
        }
        if (name == "text") {
            os << ">" << attrs.at("data") << "</text>";
        } else {
            os << "/>";
        }
    }



    Circle& Circle::SetCenter(Point point) {
          center = point;
          return *this;
      }

      Circle& Circle::SetRadius(double radius) {
          R = radius;
          return *this;
      }

      Node Circle::MakeNode() const {
        std::ostringstream  os;
        os << std::setprecision(15) << center.x << " "
           << std::setprecision(15) << center.y << " "
           << std::setprecision(15) << R;
        std::istringstream is(os.str());
        std::string cx,cy,r;
        is >> cx >> cy >> r;
          std::unordered_map<std::string, std::string> attrs = {
                  {"cx", "\\\"" + cx + "\\\""},
                  {"cy", "\\\"" + cy + "\\\""},
                  {"r", "\\\"" + r + "\\\""},
                  {"fill", "\\\"" + fill_color.color_ + "\\\""},
                  {"stroke", "\\\"" + stroke_color.color_ + "\\\""},
                  {"stroke-width", "\\\"" + std::to_string(stroke_width) + "\\\""}
          };
          if (line_join) { attrs["stroke-linejoin"] = "\\\"" + *line_join + "\\\""; }
          if (line_cap) { attrs["stroke-linecap"] = "\\\"" + *line_cap + "\\\""; }
          return Node("circle", std::move(attrs));
      }


      Polyline& Polyline::AddPoint(Point point) {
          vertices.push_back(point);
          return *this;
      }

      Node Polyline::MakeNode() const {
          std::unordered_map<std::string, std::string> attrs = {
                  {"fill", "\\\"" + fill_color.color_ + "\\\""},
                  {"stroke", "\\\"" + stroke_color.color_ + "\\\""},
                  {"stroke-width", "\\\"" + std::to_string(stroke_width) + "\\\""}
          };
          if (line_join) { attrs["stroke-linejoin"] = "\\\"" + *line_join + "\\\""; }
          if (line_cap) { attrs["stroke-linecap"] = "\\\"" + *line_cap + "\\\""; }
          attrs["points"] += "\\\"";
          for (auto& p : vertices) {
              attrs["points"] +=p.ToString() + " ";
          }
          attrs["points"] += "\\\"";
          return Node("polyline", std::move(attrs));
      }


      Text& Text::SetPoint(Point p) {
          this->point = p;
          return *this;
      }

      Text& Text::SetOffset(Point off) {
          this->offset = off;
          return *this;
      }

      Text& Text::SetFontSize(uint32_t size) {
          font_size = size;
          return *this;
      }

      Text& Text::SetFontFamily(const std::string& style) {
          font_style = style;
          return *this;
      }

      Text& Text::SetFontWeight(const std::string& weight) {
        font_weight = weight;
        return *this;
      }

      Text& Text::SetData(const std::string& data_) {
          this->data = data_;
          return *this;
      }

      Node Text::MakeNode() const {
        std::ostringstream  os;
        os << std::setprecision(15) << point.x << " "
           << std::setprecision(15) << point.y << " "
           << std::setprecision(15) << offset.x << " "
           << std::setprecision(15) << offset.y;
        std::istringstream is(os.str());
        std::string x,y,dx,dy;
        is >> x >> y >> dx >> dy;
        std::unordered_map<std::string, std::string> attrs = {
                  {"x", "\\\"" + x + "\\\""},
                  {"y", "\\\"" + y + "\\\""},
                  {"dx", "\\\"" + dx + "\\\""},
                  {"dy", "\\\"" + dy + "\\\""},
                  {"fill", "\\\"" + fill_color.color_ + "\\\""},
                  {"stroke", "\\\"" + stroke_color.color_ + "\\\""},
                  {"stroke-width", "\\\"" + std::to_string(stroke_width) + "\\\""},
                  {"data", data},
                  {"font-size", "\\\"" + std::to_string(font_size) +  + "\\\""}
          };
          if (line_join) { attrs["stroke-linejoin"] = "\\\"" + *line_join + "\\\""; }
          if (line_cap) { attrs["stroke-linecap"] = "\\\"" + *line_cap + "\\\""; }
          if (font_style) { attrs["font-family"] = "\\\"" + *font_style + "\\\""; }
          if (font_weight) { attrs["font-weight"] = "\\\"" + *font_weight + "\\\""; }
          return Node("text", std::move(attrs));
      }


    Document::Document() : root(Node()) {}
    Document::Document(Node root) : root(std::move(root)) {}

    void Document::Render(std::ostream& os) const {
        os << "<?xml version=\\\"1.0\\\" encoding=\\\"UTF-8\\\" ?>"
           << "<svg xmlns=\\\"http://www.w3.org/2000/svg\\\" version=\\\"1.1\\\">";
        for (auto& node : root.Children()) {
            node.Out(os);
        }
        os << "</svg>";
    }

}