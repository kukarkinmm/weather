//
// Created by michael on 20.10.2020.
//

#ifndef WEATHER_SRC_JSON_PARSER_INCLUDE_JSON_PARSER_JSONPARSER_H
#define WEATHER_SRC_JSON_PARSER_INCLUDE_JSON_PARSER_JSONPARSER_H

#include <strstream>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

class JsonParser {
  boost::property_tree::ptree pt;

public:
  explicit JsonParser(std::stringstream &json_stream);
  [[nodiscard]] std::optional<std::string> parse() const;
  [[nodiscard]] std::optional<std::string> get_location() const;
};

#endif //WEATHER_SRC_JSON_PARSER_INCLUDE_JSON_PARSER_JSONPARSER_H
