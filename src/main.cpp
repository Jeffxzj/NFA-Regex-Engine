#include "regex.hpp"
#include "list.hpp"

#include <iostream>


int main(int argc, const char **argv) {
  if (argc < 3) { exit(1); }
  if (auto regex = Regex::init(argv[1])) {
    if (auto match = regex->match(argv[2])) {
      std::cout << "MATCH: " << match.value() << std::endl;
    } else {
      std::cout << "NO_MATCH" << std::endl;
    }
  } else {
    std::cout << "ERROR" << std::endl;
    exit(1);
  }
}
