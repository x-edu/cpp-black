#include "tests.h"

#include "json.h"
#include "requests.h"
#include "svg.h"
#include "test_runner.h"
#include "transport_catalog.h"

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

constexpr std::string_view kPartEFirstRequest = R"(
{
  "routing_settings": {
    "bus_wait_time": 6,
    "bus_velocity": 40
  },
    "render_settings": {
        "width": 1200,
        "height": 1200,
        "padding": 50,
        "stop_radius": 5,
        "line_width": 14,
        "stop_label_font_size": 20,
        "stop_label_offset": [
            7,
            -3
        ],
        "underlayer_color": [
            255,
            255,
            255,
            0.85
        ],
        "underlayer_width": 3,
        "color_palette": [
            "green",
            [
                255,
                160,
                0
            ],
            "red"
        ]
    },
  "base_requests": [
    {
      "type": "Bus",
      "name": "297",
      "stops": [
        "Biryulyovo Zapadnoye",
        "Biryulyovo Tovarnaya",
        "Universam",
        "Biryulyovo Zapadnoye"
      ],
      "is_roundtrip": true
    },
    {
      "type": "Bus",
      "name": "635",
      "stops": [
        "Biryulyovo Tovarnaya",
        "Universam",
        "Prazhskaya"
      ],
      "is_roundtrip": false
    },
    {
      "type": "Stop",
      "road_distances": {
        "Biryulyovo Tovarnaya": 2600
      },
      "longitude": 37.6517,
      "name": "Biryulyovo Zapadnoye",
      "latitude": 55.574371
    },
    {
      "type": "Stop",
      "road_distances": {
        "Prazhskaya": 4650,
        "Biryulyovo Tovarnaya": 1380,
        "Biryulyovo Zapadnoye": 2500
      },
      "longitude": 37.645687,
      "name": "Universam",
      "latitude": 55.587655
    },
    {
      "type": "Stop",
      "road_distances": {
        "Universam": 890
      },
      "longitude": 37.653656,
      "name": "Biryulyovo Tovarnaya",
      "latitude": 55.592028
    },
    {
      "type": "Stop",
      "road_distances": {},
      "longitude": 37.603938,
      "name": "Prazhskaya",
      "latitude": 55.611717
    }
  ],
  "stat_requests": [
    {
      "type": "Bus",
      "name": "297",
      "id": 1
    },
    {
      "type": "Bus",
      "name": "635",
      "id": 2
    },
    {
      "type": "Stop",
      "name": "Universam",
      "id": 3
    },
    {
      "type": "Route",
      "from": "Biryulyovo Zapadnoye",
      "to": "Universam",
      "id": 4
    },
    {
      "type": "Route",
      "from": "Biryulyovo Zapadnoye",
      "to": "Prazhskaya",
      "id": 5
    }
  ]
}
)";
constexpr std::string_view kPartEFirstResponse = R"(
[
    {
        "curvature": 1.42963,
        "unique_stop_count": 3,
        "stop_count": 4,
        "request_id": 1,
        "route_length": 5990
    },
    {
        "curvature": 1.30156,
        "unique_stop_count": 3,
        "stop_count": 5,
        "request_id": 2,
        "route_length": 11570
    },
    {
        "request_id": 3,
        "buses": [
            "297",
            "635"
        ]
    },
    {
        "total_time": 11.235,
        "items": [
            {
                "time": 6,
                "type": "Wait",
                "stop_name": "Biryulyovo Zapadnoye"
            },
            {
                "span_count": 2,
                "bus": "297",
                "type": "Bus",
                "time": 5.235
            }
        ],
        "request_id": 4
    },
    {
        "total_time": 24.21,
        "items": [
            {
                "time": 6,
                "type": "Wait",
                "stop_name": "Biryulyovo Zapadnoye"
            },
            {
                "span_count": 2,
                "bus": "297",
                "type": "Bus",
                "time": 5.235
            },
            {
                "time": 6,
                "type": "Wait",
                "stop_name": "Universam"
            },
            {
                "span_count": 1,
                "bus": "635",
                "type": "Bus",
                "time": 6.975
            }
        ],
        "request_id": 5
    }
]
)";

void SimpleSvgTest() {
  auto circle = Svg::Circle().SetCenter({.x = -2, .y = 10});
  auto doc = Svg::Document{};
  doc.Add(circle);
  std::stringstream stream{};
  doc.Render(stream);
  ASSERT_EQUAL(
      R"(<?xml version="1.0" encoding="UTF-8" ?><svg xmlns="http://www.w3.org/2000/svg" version="1.1"><circle fill="none" stroke="none" stroke-width="1" cx="-2" cy="10" r="1" /></svg>)",
      stream.str());
}

void CourseraSvgExample() {
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

void MakeRequest(std::istream& input, std::ostream& output) {
  const auto input_doc = Json::Load(input);
  const auto& input_map = input_doc.GetRoot().AsMap();

  const TransportCatalog db(
      Descriptions::ReadDescriptions(input_map.at("base_requests").AsArray()),
      input_map.at("routing_settings").AsMap(),
      input_map.at("render_settings").AsMap());

  Json::PrintValue(
      Requests::ProcessAll(db, input_map.at("stat_requests").AsArray()),
      output);
}

std::stringstream MakeExpectedFromJson(const std::string_view view) {
  std::stringstream input{view.data()};
  std::stringstream output{};
  Json::Print(Json::Load(input), output);
  return output;
}

void CourseraPartEFirstCase() {
  std::stringstream input{kPartEFirstRequest.data()};
  std::stringstream output{};

  const std::stringstream expected = MakeExpectedFromJson(kPartEFirstResponse);

  MakeRequest(input, output);
  ASSERT_EQUAL(output.str(), expected.str());
}

void TestJsonEscape() {
  const std::string value = "a\"d";
  const std::string expected = R"("a\"d")";
  const Json::Node node{value};
  std::stringstream output{};
  Json::PrintNode(node, output);
  ASSERT_EQUAL(output.str(), expected);
}

}  // namespace

void RunTests() {
  TestRunner tr;
  RUN_TEST(tr, SimpleSvgTest);
  RUN_TEST(tr, CourseraSvgExample);
  RUN_TEST(tr, CourseraPartEFirstCase);
  RUN_TEST(tr, TestJsonEscape);
}
