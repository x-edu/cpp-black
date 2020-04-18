#include <iostream>

#include "descriptions.h"
#include "json.h"
#include "renderer.h"
#include "requests.h"
#include "transport_catalog.h"

#ifdef TESTS
#include "tests.h"
#endif  // TESTS

using namespace std;

int main() {
#ifdef TESTS
  RunTests();
#endif  // TESTS

  const auto input_doc = Json::Load(cin);
  const auto& input_map = input_doc.GetRoot().AsMap();

  const TransportCatalog db(
      Descriptions::ReadDescriptions(input_map.at("base_requests").AsArray()),
      input_map.at("routing_settings").AsMap(),
      input_map.at("render_settings").AsMap());

  Json::PrintValue(
      Requests::ProcessAll(db, input_map.at("stat_requests").AsArray()), cout);
  cout << endl;

  return 0;
}
