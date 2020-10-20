//
// Created by michael on 20.10.2020.
//

#include "local_storage/LocalStorage.h"

bool LocalStorage::comparator::operator()(const std::tuple<unsigned long, std::string, std::string> &lhs,
                                          const std::tuple<unsigned long, std::string, std::string> &rhs) const {
  return std::get<0>(lhs) < std::get<0>(rhs);
}

LocalStorage::LocalStorage() : file{}, data{} {
  if (exists()) {
    file.open(filename, std::fstream::in | std::fstream::out);
  } else {
    file.open(filename, std::fstream::in | std::fstream::out | std::fstream::trunc);
  }
  populate();
  invalidate();
}

bool LocalStorage::exists() const {
  return file.good();
}

void LocalStorage::populate() {
  std::string line;
  while (std::getline(file, line)) {
    std::vector<std::string> splitted{};
    boost::split(splitted, line, boost::is_any_of("\t"));
    data.emplace(boost::lexical_cast<unsigned long>(splitted[0]), splitted[1], splitted[2]);
  }
}

void LocalStorage::invalidate() {
  auto oldest_time =
      std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count()
          - 60 * 30;
  auto iterator = std::lower_bound(data.cbegin(), data.cend(), oldest_time, [](const auto &lhs, const auto &rhs) {
    return std::get<0>(lhs) < rhs;
  });
  data.erase(data.begin(), iterator);
}

void LocalStorage::persist() {
  file.close();
  clear();
  file.open(filename, std::fstream::in | std::fstream::out | std::fstream::trunc);
  for (const auto &value : data) {
    file << std::get<0>(value) << '\t' << std::get<1>(value) << '\t' << std::get<2>(value) << std::endl;
  }
}

void LocalStorage::clear() {
  std::remove(filename);
}

void LocalStorage::add(const std::string &location,
                       const std::string &json,
                       const std::chrono::time_point<std::chrono::system_clock> &time) {
  unsigned long timepoint = std::chrono::duration_cast<std::chrono::seconds>(time.time_since_epoch()).count();
  data.emplace(timepoint, location, json);
}

std::optional <std::string> LocalStorage::get_local(const std::string &location) {
  auto iterator = std::find_if(data.cbegin(), data.cend(), [&location](const auto &value) {
    return std::get<1>(value) == location;
  });
  if (iterator != data.end()) {
    return std::make_optional(std::get<2>(*iterator));
  }
  return std::nullopt;
}

LocalStorage::~LocalStorage() {
  file.close();
}
