#include "descriptions.h"
#include "json.h"
#include "requests.h"
#include "sphere.h"
#include "transport_catalog.h"
#include "utils.h"
//#include "test_runner.h"

#include <iostream>

using namespace std;

//void TestSVG() {
//    Svg::Document svg;
//
//    svg.Add(
//            Svg::Polyline{}
//                    .SetStrokeColor(Svg::Rgb{140, 198, 63})  // soft green
//                    .SetStrokeWidth(16)
//                    .SetStrokeLineCap("round")
//                    .AddPoint({50, 50})
//                    .AddPoint({250, 250})
//    );
//
//    for (const auto point : {Svg::Point{50, 50}, Svg::Point{250, 250}}) {
//        svg.Add(
//                Svg::Circle{}
//                        .SetFillColor("white")
//                        .SetRadius(6)
//                        .SetCenter(point)
//        );
//    }
//
//    svg.Add(
//            Svg::Text{}
//                    .SetPoint({50, 50})
//                    .SetOffset({10, -10})
//                    .SetFontSize(20)
//                    .SetFontFamily("Verdana")
//                    .SetFillColor("black")
//                    .SetData("C")
//    );
//    svg.Add(
//            Svg::Text{}
//                    .SetPoint({250, 250})
//                    .SetOffset({10, -10})
//                    .SetFontSize(20)
//                    .SetFontFamily("Verdana")
//                    .SetFillColor("black")
//                    .SetData("C++")
//    );
//
//    svg.Render(std::cout);
//
//}


int main() {
//    TestSVG();
  const auto input_doc = Json::Load(cin);
  const auto& input_map = input_doc.GetRoot().AsMap();

  const TransportCatalog db(
    Descriptions::ReadDescriptions(input_map.at("base_requests").AsArray()),
    input_map.at("routing_settings").AsMap(),
    input_map.at("render_settings").AsMap()
  );

  Json::PrintValue(
    Requests::ProcessAll(db, input_map.at("stat_requests").AsArray()), cout
  );

  return 0;
}
