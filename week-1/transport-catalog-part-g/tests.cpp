#include "tests.h"

#include "svg.h"
#include "test_runner.h"

namespace {

constexpr std::string_view kCourseraExampleSvg =
    R"(<?xml version="1.0" encoding="UTF-8" ?>)"
    R"(<svg xmlns="http://www.w3.org/2000/svg" version="1.1">)"
    R".(<polyline fill="none" stroke="rgb(140,198,63)" stroke-width="16" stroke-linecap="round" points="50,50 250,250" />)."
    R"(<circle fill="white" stroke="none" stroke-width="1" cx="50" cy="50" r="6" />)"
    R"(<circle fill="white" stroke="none" stroke-width="1" cx="250" cy="250" r="6" />)"
    R"(<text fill="black" stroke="none" stroke-width="1" x="50" y="50" dx="10" dy="-10" font-size="20" font-family="Verdana" >)"
    "C"
    "</text>"
    R"(<text fill="black" stroke="none" stroke-width="1" x="250" y="250" dx="10" dy="-10" font-size="20" font-family="Verdana" >)"
    "C++"
    "</text>"
    "</svg>";

void SimpleTest() {
  auto circle = Svg::Circle().SetCenter({.x = -2, .y = 10});
  auto doc = Svg::Document{};
  doc.Add(circle);
  std::stringstream stream{};
  doc.Render(stream);
  ASSERT_EQUAL(
      R"(<?xml version="1.0" encoding="UTF-8" ?><svg xmlns="http://www.w3.org/2000/svg" version="1.1"><circle fill="none" stroke="none" stroke-width="1" cx="-2" cy="10" r="1" /></svg>)",
      stream.str());
}

void CourseraExample() {
  Svg::Document svg;

  svg.Add(Svg::Polyline{}
              .SetStrokeColor(Svg::Rgb{140, 198, 63})  // soft green
              .SetStrokeWidth(16)
              .SetStrokeLineCap("round")
              .AddPoint({50, 50})
              .AddPoint({250, 250}));

  for (const auto point : {Svg::Point{50, 50}, Svg::Point{250, 250}}) {
    svg.Add(Svg::Circle{}.SetFillColor("white").SetRadius(6).SetCenter(point));
  }

  svg.Add(Svg::Text{}
              .SetPoint({50, 50})
              .SetOffset({10, -10})
              .SetFontSize(20)
              .SetFontFamily("Verdana")
              .SetFillColor("black")
              .SetData("C"));
  svg.Add(Svg::Text{}
              .SetPoint({250, 250})
              .SetOffset({10, -10})
              .SetFontSize(20)
              .SetFontFamily("Verdana")
              .SetFillColor("black")
              .SetData("C++"));

  std::stringstream stream{};
  svg.Render(stream);
  ASSERT_EQUAL(stream.str(), kCourseraExampleSvg);
}

}  // namespace

void RunTests() {
  TestRunner tr;
  RUN_TEST(tr, SimpleTest);
  RUN_TEST(tr, CourseraExample);
}
