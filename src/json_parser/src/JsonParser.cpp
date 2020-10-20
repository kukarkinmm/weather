//
// Created by michael on 20.10.2020.
//

#include "json_parser/JsonParser.h"

JsonParser::JsonParser(std::stringstream &json_stream) : pt{} {
  boost::property_tree::read_json(json_stream, pt);
}

std::optional<std::string> JsonParser::parse() const {
  std::stringstream result;

  try {

    result << pt.get<std::string>("location.localtime") << '\n'
           << pt.get<std::string>("location.name") << ", " << pt.get<std::string>("location.country") << '\n'
           << "Observation time: " << pt.get<std::string>("current.observation_time") << '\n'
           << "Temperature: " << pt.get<std::string>("current.temperature") << '\n'
           << "Wind speed: " << pt.get<std::string>("current.wind_speed") << pt.get<std::string>("current.wind_dir") << '\n'
           << "Pressure: " << pt.get<std::string>("current.pressure") << '\n'
           << "Humidity: " << pt.get<std::string>("current.humidity") << '\n'
           << "Cloudcover: " << pt.get<std::string>("current.cloudcover") << '\n'
           << "Visibility: " << pt.get<std::string>("current.visibility") << '\n'
           << "-----------------------------------------";

  } catch (...) {
    return std::nullopt;
  }

  return std::make_optional(result.str());
}

std::optional<std::string> JsonParser::get_location() const {
  std::string name;
  try {

    name = pt.get<std::string>("location.name");

  } catch (...) {
    return std::nullopt;
  }
  return std::make_optional(name);
}
