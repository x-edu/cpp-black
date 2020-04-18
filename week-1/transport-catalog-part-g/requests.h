#pragma once

#include <string>
#include <variant>

#include "json.h"
#include "renderer.h"
#include "transport_catalog.h"

namespace Requests {

struct Stop {
  std::string name;

  Json::Dict Process(const TransportCatalog& db) const;
};

struct Bus {
  std::string name;

  Json::Dict Process(const TransportCatalog& db) const;
};

struct Route {
  std::string stop_from;
  std::string stop_to;

  Json::Dict Process(const TransportCatalog& db) const;
};

struct Map {
  Json::Dict Process(const TransportCatalog& db) const;
};

using Request = std::variant<Stop, Bus, Route, Map>;

Request Read(const Json::Dict& attrs);

std::vector<Json::Node> ProcessAll(const TransportCatalog& db,
                                   const std::vector<Json::Node>& requests);
}  // namespace Requests
