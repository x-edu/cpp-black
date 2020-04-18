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
        "layers": ["bus_lines", "stop_points", "stop_labels"],
        "bus_label_font_size": 20,
        "bus_label_offset": [
            7,
            15
        ],
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

constexpr std::string_view kPartHFirstRequest = R"(
{
    "routing_settings": {
        "bus_wait_time": 2,
        "bus_velocity": 30
    },
    "render_settings": {
        "width": 1200,
        "height": 1200,
        "padding": 50,
        "stop_radius": 5,
        "line_width": 14,
        "layers": ["bus_lines", "bus_labels", "stop_points", "stop_labels"],
        "bus_label_font_size": 20,
        "bus_label_offset": [
            7,
            15
        ],
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
            "name": "14",
            "stops": [
                "Улица Лизы Чайкиной",
                "Электросети",
                "Ривьерский мост",
                "Гостиница Сочи",
                "Кубанская улица",
                "По требованию",
                "Улица Докучаева",
                "Улица Лизы Чайкиной"
            ],
            "is_roundtrip": true
        },
        {
            "type": "Bus",
            "name": "24",
            "stops": [
                "Улица Докучаева",
                "Параллельная улица",
                "Электросети",
                "Санаторий Родина"
            ],
            "is_roundtrip": false
        },
        {
            "type": "Bus",
            "name": "114",
            "stops": [
                "Морской вокзал",
                "Ривьерский мост"
            ],
            "is_roundtrip": false
        },
        {
            "type": "Stop",
            "name": "Улица Лизы Чайкиной",
            "latitude": 43.590317,
            "longitude": 39.746833,
            "road_distances": {
                "Электросети": 4300,
                "Улица Докучаева": 2000
            }
        },
        {
            "type": "Stop",
            "name": "Морской вокзал",
            "latitude": 43.581969,
            "longitude": 39.719848,
            "road_distances": {
                "Ривьерский мост": 850
            }
        },
        {
            "type": "Stop",
            "name": "Электросети",
            "latitude": 43.598701,
            "longitude": 39.730623,
            "road_distances": {
                "Санаторий Родина": 4500,
                "Параллельная улица": 1200,
                "Ривьерский мост": 1900
            }
        },
        {
            "type": "Stop",
            "name": "Ривьерский мост",
            "latitude": 43.587795,
            "longitude": 39.716901,
            "road_distances": {
                "Морской вокзал": 850,
                "Гостиница Сочи": 1740
            }
        },
        {
            "type": "Stop",
            "name": "Гостиница Сочи",
            "latitude": 43.578079,
            "longitude": 39.728068,
            "road_distances": {
                "Кубанская улица": 320
            }
        },
        {
            "type": "Stop",
            "name": "Кубанская улица",
            "latitude": 43.578509,
            "longitude": 39.730959,
            "road_distances": {
                "По требованию": 370
            }
        },
        {
            "type": "Stop",
            "name": "По требованию",
            "latitude": 43.579285,
            "longitude": 39.733742,
            "road_distances": {
                "Улица Докучаева": 600
            }
        },
        {
            "type": "Stop",
            "name": "Улица Докучаева",
            "latitude": 43.585586,
            "longitude": 39.733879,
            "road_distances": {
                "Параллельная улица": 1100
            }
        },
        {
            "type": "Stop",
            "name": "Параллельная улица",
            "latitude": 43.590041,
            "longitude": 39.732886,
            "road_distances": {}
        },
        {
            "type": "Stop",
            "name": "Санаторий Родина",
            "latitude": 43.601202,
            "longitude": 39.715498,
            "road_distances": {}
        }
    ],
    "stat_requests": [
        {
            "id": 826874078,
            "type": "Bus",
            "name": "14"
        },
        {
            "id": 1086967114,
            "type": "Route",
            "from": "Морской вокзал",
            "to": "Параллельная улица"
        },
        {
            "id": 1218663236,
            "type": "Map"
        }
    ]
}
)";
constexpr std::string_view kPartHFirstResponse = R".([{"curvature": 1.60481, "request_id": 826874078, "route_length": 11230, "stop_count": 8, "unique_stop_count": 7}, {"items": [{"stop_name": "Морской вокзал", "time": 2, "type": "Wait"}, {"bus": "114", "span_count": 1, "time": 1.7, "type": "Bus"}, {"stop_name": "Ривьерский мост", "time": 2, "type": "Wait"}, {"bus": "14", "span_count": 4, "time": 6.06, "type": "Bus"}, {"stop_name": "Улица Докучаева", "time": 2, "type": "Wait"}, {"bus": "24", "span_count": 1, "time": 2.2, "type": "Bus"}], "request_id": 1086967114, "total_time": 15.96}, {"map": "<?xml version=\"1.0\" encoding=\"UTF-8\" ?><svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\"><polyline fill=\"none\" stroke=\"green\" stroke-width=\"14\" stroke-linecap=\"round\" stroke-linejoin=\"round\" points=\"202.705,725.165 99.2516,520.646 202.705,725.165\" /><polyline fill=\"none\" stroke=\"rgb(255,160,0)\" stroke-width=\"14\" stroke-linecap=\"round\" stroke-linejoin=\"round\" points=\"1150,432.113 580.956,137.796 99.2516,520.646 491.264,861.722 592.751,846.627 690.447,819.386 695.256,598.192 1150,432.113\" /><polyline fill=\"none\" stroke=\"red\" stroke-width=\"14\" stroke-linecap=\"round\" stroke-linejoin=\"round\" points=\"695.256,598.192 660.397,441.801 580.956,137.796 50,50 580.956,137.796 660.397,441.801 695.256,598.192\" /><text fill=\"rgba(255,255,255,0.85)\" stroke=\"rgba(255,255,255,0.85)\" stroke-width=\"3\" stroke-linecap=\"round\" stroke-linejoin=\"round\" x=\"202.705\" y=\"725.165\" dx=\"7\" dy=\"15\" font-size=\"20\" font-family=\"Verdana\" font-weight=\"bold\" >114</text><text fill=\"green\" stroke=\"none\" stroke-width=\"1\" x=\"202.705\" y=\"725.165\" dx=\"7\" dy=\"15\" font-size=\"20\" font-family=\"Verdana\" font-weight=\"bold\" >114</text><text fill=\"rgba(255,255,255,0.85)\" stroke=\"rgba(255,255,255,0.85)\" stroke-width=\"3\" stroke-linecap=\"round\" stroke-linejoin=\"round\" x=\"99.2516\" y=\"520.646\" dx=\"7\" dy=\"15\" font-size=\"20\" font-family=\"Verdana\" font-weight=\"bold\" >114</text><text fill=\"green\" stroke=\"none\" stroke-width=\"1\" x=\"99.2516\" y=\"520.646\" dx=\"7\" dy=\"15\" font-size=\"20\" font-family=\"Verdana\" font-weight=\"bold\" >114</text><text fill=\"rgba(255,255,255,0.85)\" stroke=\"rgba(255,255,255,0.85)\" stroke-width=\"3\" stroke-linecap=\"round\" stroke-linejoin=\"round\" x=\"1150\" y=\"432.113\" dx=\"7\" dy=\"15\" font-size=\"20\" font-family=\"Verdana\" font-weight=\"bold\" >14</text><text fill=\"rgb(255,160,0)\" stroke=\"none\" stroke-width=\"1\" x=\"1150\" y=\"432.113\" dx=\"7\" dy=\"15\" font-size=\"20\" font-family=\"Verdana\" font-weight=\"bold\" >14</text><text fill=\"rgba(255,255,255,0.85)\" stroke=\"rgba(255,255,255,0.85)\" stroke-width=\"3\" stroke-linecap=\"round\" stroke-linejoin=\"round\" x=\"695.256\" y=\"598.192\" dx=\"7\" dy=\"15\" font-size=\"20\" font-family=\"Verdana\" font-weight=\"bold\" >24</text><text fill=\"red\" stroke=\"none\" stroke-width=\"1\" x=\"695.256\" y=\"598.192\" dx=\"7\" dy=\"15\" font-size=\"20\" font-family=\"Verdana\" font-weight=\"bold\" >24</text><text fill=\"rgba(255,255,255,0.85)\" stroke=\"rgba(255,255,255,0.85)\" stroke-width=\"3\" stroke-linecap=\"round\" stroke-linejoin=\"round\" x=\"50\" y=\"50\" dx=\"7\" dy=\"15\" font-size=\"20\" font-family=\"Verdana\" font-weight=\"bold\" >24</text><text fill=\"red\" stroke=\"none\" stroke-width=\"1\" x=\"50\" y=\"50\" dx=\"7\" dy=\"15\" font-size=\"20\" font-family=\"Verdana\" font-weight=\"bold\" >24</text><circle fill=\"white\" stroke=\"none\" stroke-width=\"1\" cx=\"491.264\" cy=\"861.722\" r=\"5\" /><circle fill=\"white\" stroke=\"none\" stroke-width=\"1\" cx=\"592.751\" cy=\"846.627\" r=\"5\" /><circle fill=\"white\" stroke=\"none\" stroke-width=\"1\" cx=\"202.705\" cy=\"725.165\" r=\"5\" /><circle fill=\"white\" stroke=\"none\" stroke-width=\"1\" cx=\"660.397\" cy=\"441.801\" r=\"5\" /><circle fill=\"white\" stroke=\"none\" stroke-width=\"1\" cx=\"690.447\" cy=\"819.386\" r=\"5\" /><circle fill=\"white\" stroke=\"none\" stroke-width=\"1\" cx=\"99.2516\" cy=\"520.646\" r=\"5\" /><circle fill=\"white\" stroke=\"none\" stroke-width=\"1\" cx=\"50\" cy=\"50\" r=\"5\" /><circle fill=\"white\" stroke=\"none\" stroke-width=\"1\" cx=\"695.256\" cy=\"598.192\" r=\"5\" /><circle fill=\"white\" stroke=\"none\" stroke-width=\"1\" cx=\"1150\" cy=\"432.113\" r=\"5\" /><circle fill=\"white\" stroke=\"none\" stroke-width=\"1\" cx=\"580.956\" cy=\"137.796\" r=\"5\" /><text fill=\"rgba(255,255,255,0.85)\" stroke=\"rgba(255,255,255,0.85)\" stroke-width=\"3\" stroke-linecap=\"round\" stroke-linejoin=\"round\" x=\"491.264\" y=\"861.722\" dx=\"7\" dy=\"-3\" font-size=\"20\" font-family=\"Verdana\" >Гостиница Сочи</text><text fill=\"black\" stroke=\"none\" stroke-width=\"1\" x=\"491.264\" y=\"861.722\" dx=\"7\" dy=\"-3\" font-size=\"20\" font-family=\"Verdana\" >Гостиница Сочи</text><text fill=\"rgba(255,255,255,0.85)\" stroke=\"rgba(255,255,255,0.85)\" stroke-width=\"3\" stroke-linecap=\"round\" stroke-linejoin=\"round\" x=\"592.751\" y=\"846.627\" dx=\"7\" dy=\"-3\" font-size=\"20\" font-family=\"Verdana\" >Кубанская улица</text><text fill=\"black\" stroke=\"none\" stroke-width=\"1\" x=\"592.751\" y=\"846.627\" dx=\"7\" dy=\"-3\" font-size=\"20\" font-family=\"Verdana\" >Кубанская улица</text><text fill=\"rgba(255,255,255,0.85)\" stroke=\"rgba(255,255,255,0.85)\" stroke-width=\"3\" stroke-linecap=\"round\" stroke-linejoin=\"round\" x=\"202.705\" y=\"725.165\" dx=\"7\" dy=\"-3\" font-size=\"20\" font-family=\"Verdana\" >Морской вокзал</text><text fill=\"black\" stroke=\"none\" stroke-width=\"1\" x=\"202.705\" y=\"725.165\" dx=\"7\" dy=\"-3\" font-size=\"20\" font-family=\"Verdana\" >Морской вокзал</text><text fill=\"rgba(255,255,255,0.85)\" stroke=\"rgba(255,255,255,0.85)\" stroke-width=\"3\" stroke-linecap=\"round\" stroke-linejoin=\"round\" x=\"660.397\" y=\"441.801\" dx=\"7\" dy=\"-3\" font-size=\"20\" font-family=\"Verdana\" >Параллельная улица</text><text fill=\"black\" stroke=\"none\" stroke-width=\"1\" x=\"660.397\" y=\"441.801\" dx=\"7\" dy=\"-3\" font-size=\"20\" font-family=\"Verdana\" >Параллельная улица</text><text fill=\"rgba(255,255,255,0.85)\" stroke=\"rgba(255,255,255,0.85)\" stroke-width=\"3\" stroke-linecap=\"round\" stroke-linejoin=\"round\" x=\"690.447\" y=\"819.386\" dx=\"7\" dy=\"-3\" font-size=\"20\" font-family=\"Verdana\" >По требованию</text><text fill=\"black\" stroke=\"none\" stroke-width=\"1\" x=\"690.447\" y=\"819.386\" dx=\"7\" dy=\"-3\" font-size=\"20\" font-family=\"Verdana\" >По требованию</text><text fill=\"rgba(255,255,255,0.85)\" stroke=\"rgba(255,255,255,0.85)\" stroke-width=\"3\" stroke-linecap=\"round\" stroke-linejoin=\"round\" x=\"99.2516\" y=\"520.646\" dx=\"7\" dy=\"-3\" font-size=\"20\" font-family=\"Verdana\" >Ривьерский мост</text><text fill=\"black\" stroke=\"none\" stroke-width=\"1\" x=\"99.2516\" y=\"520.646\" dx=\"7\" dy=\"-3\" font-size=\"20\" font-family=\"Verdana\" >Ривьерский мост</text><text fill=\"rgba(255,255,255,0.85)\" stroke=\"rgba(255,255,255,0.85)\" stroke-width=\"3\" stroke-linecap=\"round\" stroke-linejoin=\"round\" x=\"50\" y=\"50\" dx=\"7\" dy=\"-3\" font-size=\"20\" font-family=\"Verdana\" >Санаторий Родина</text><text fill=\"black\" stroke=\"none\" stroke-width=\"1\" x=\"50\" y=\"50\" dx=\"7\" dy=\"-3\" font-size=\"20\" font-family=\"Verdana\" >Санаторий Родина</text><text fill=\"rgba(255,255,255,0.85)\" stroke=\"rgba(255,255,255,0.85)\" stroke-width=\"3\" stroke-linecap=\"round\" stroke-linejoin=\"round\" x=\"695.256\" y=\"598.192\" dx=\"7\" dy=\"-3\" font-size=\"20\" font-family=\"Verdana\" >Улица Докучаева</text><text fill=\"black\" stroke=\"none\" stroke-width=\"1\" x=\"695.256\" y=\"598.192\" dx=\"7\" dy=\"-3\" font-size=\"20\" font-family=\"Verdana\" >Улица Докучаева</text><text fill=\"rgba(255,255,255,0.85)\" stroke=\"rgba(255,255,255,0.85)\" stroke-width=\"3\" stroke-linecap=\"round\" stroke-linejoin=\"round\" x=\"1150\" y=\"432.113\" dx=\"7\" dy=\"-3\" font-size=\"20\" font-family=\"Verdana\" >Улица Лизы Чайкиной</text><text fill=\"black\" stroke=\"none\" stroke-width=\"1\" x=\"1150\" y=\"432.113\" dx=\"7\" dy=\"-3\" font-size=\"20\" font-family=\"Verdana\" >Улица Лизы Чайкиной</text><text fill=\"rgba(255,255,255,0.85)\" stroke=\"rgba(255,255,255,0.85)\" stroke-width=\"3\" stroke-linecap=\"round\" stroke-linejoin=\"round\" x=\"580.956\" y=\"137.796\" dx=\"7\" dy=\"-3\" font-size=\"20\" font-family=\"Verdana\" >Электросети</text><text fill=\"black\" stroke=\"none\" stroke-width=\"1\" x=\"580.956\" y=\"137.796\" dx=\"7\" dy=\"-3\" font-size=\"20\" font-family=\"Verdana\" >Электросети</text></svg>", "request_id": 1218663236}]).";

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

void AssertCourseraTest(const std::string_view request,
                  const std::string_view response) {
  std::stringstream input{request.data()};
  std::stringstream output{};

  const std::stringstream expected = MakeExpectedFromJson(response);

  MakeRequest(input, output);
  ASSERT_EQUAL(output.str(), expected.str());
}

void CourseraPartEFirstCase() {
  AssertCourseraTest(kPartEFirstRequest, kPartEFirstResponse);
}

void CourseraPartHFirstCase() {
  std::stringstream input{kPartHFirstRequest.data()};
  std::stringstream output{};
  MakeRequest(input, output);
  ASSERT_EQUAL(output.str(), kPartHFirstResponse);
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
  RUN_TEST(tr, CourseraPartHFirstCase);
}
