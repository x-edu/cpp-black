#include "tests.h"

#include "svg.h"
#include "test_runner.h"

namespace {

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

}  // namespace

void RunTests() {
  TestRunner tr;
  RUN_TEST(tr, SimpleTest);
}
