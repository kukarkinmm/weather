//
// Created by michael on 20.10.2020.
//

#ifndef WEATHER_SRC_LOCAL_STORAGE_INCLUDE_LOCAL_STORAGE_LOCALSTORAGE_H
#define WEATHER_SRC_LOCAL_STORAGE_INCLUDE_LOCAL_STORAGE_LOCALSTORAGE_H

#include <tuple>
#include <fstream>
#include <set>
#include <chrono>

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

class LocalStorage {

  struct comparator {
    using t_type = std::tuple<unsigned long, std::string, std::string>;
    bool operator() (const t_type &, const t_type &) const;
  };

  static constexpr auto filename = "storage";
  std::fstream file;
  std::set<std::tuple<unsigned long, std::string, std::string>, comparator> data;

  bool exists() const;
  void populate();
  void invalidate();
public:
  LocalStorage();
  void clear();
  void add(
      const std::string &location,
      const std::string &json,
      const std::chrono::time_point<std::chrono::system_clock> &time = std::chrono::system_clock::now()
  );
  std::optional<std::string> get_local(const std::string &location);
  void persist();
  ~LocalStorage();
};

#endif //WEATHER_SRC_LOCAL_STORAGE_INCLUDE_LOCAL_STORAGE_LOCALSTORAGE_H
