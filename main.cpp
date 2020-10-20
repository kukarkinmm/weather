#include <iostream>
#include <csignal>

#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>

#include "client/Client.h"
#include "json_parser/JsonParser.h"
#include "local_storage/LocalStorage.h"

void handler(int s) {
  std::cout << "This is bad" <<  s << std::endl;
  exit(1);
}

void handle_abort() {
  struct sigaction sigIntHandler;
  sigIntHandler.sa_handler = handler;
  sigemptyset(&sigIntHandler.sa_mask);
  sigIntHandler.sa_flags = 0;

  sigaction(SIGINT, &sigIntHandler, NULL);
}

int main(int argc, char **argv) {
  LocalStorage storage{};

  const std::string KEY{"cabbabd749aef9a10036b4d98598e114"};
  Client client{"api.weatherstack.com", "80"};

  // I am very ashamed of this but I am also tired
  int n_locations = argc - 2;
  int received = 0;

  handle_abort();

  boost::program_options::options_description desc{"Allowed options"};
  desc.add_options()
      ("help,h", "help message")
      ("clear,c", "clear local storage")
      ("locations,l", boost::program_options::value<std::vector<std::string>>(),
       "list of locations to fetch weather");

  boost::program_options::positional_options_description p;
  p.add("locations", -1);

  boost::program_options::variables_map vm;
  boost::program_options::store(
      boost::program_options::command_line_parser(argc, argv).options(desc).positional(p).run(),
      vm
  );
  boost::program_options::notify(vm);

  if (vm.count("help")) {
    std::cout << desc << std::endl;
    return 0;
  }

  if (vm.count("clear")) {
    storage.clear();
    return 0;
  }

  std::vector<std::string> locations{};
  locations.reserve(argc);
  if (vm.count("locations")) {
    auto locs = vm["locations"].as<std::vector<std::string>>();
    std::copy(locs.cbegin(), locs.cend(), std::back_inserter(locations));
  }


  client.subscribe([&storage, &received](auto result) {
    if (result.first) {
      JsonParser parser{result.second};
      if (auto opt = parser.parse(); opt) {
        storage.add(*parser.get_location(), result.second.str());
        std::cout << *opt << std::endl;
      } else {
        std::cout << "Data is not available" << std::endl;
      }
    } else {
      std::cout << "Something went wrong: " << result.second.str() << std::endl;
    }
    received++;
  });

  for (const auto &location : locations) {
    if (auto opt = storage.get_local(location); opt) {
      std::stringstream ss{};
      ss << *opt;
      JsonParser parser{ss};
      std::cout << *parser.parse() << std::endl;
      received++;
    } else {
      client.send("/current?access_key=" + KEY + "&query=" + location);
    }
  }

  // I am truly ashamed
  std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  while (n_locations != received) {
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
  }

  storage.persist();

  return 0;
}