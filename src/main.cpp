#include <iostream>
#include <string>

#include "utility.hpp"
#include "regex.hpp"


int main(int argc, const char **argv) {
  if (argc < 3) { regex_abort("need three argument"); }
  if (auto regex = Regex::init(argv[1])) {
    std::string input = argv[2];
    if (auto match = regex->match(input)) {
      std::cout << "========== [  RESULT  ] ==========" << std::endl;

      auto &[start, end] = match.value();
      std::cout
          << "MATCH: " << start << ", " << end << ", "
          << input.substr(start, end - start) << std::endl;
    } else {
      std::cout << "NO_MATCH" << std::endl;
    }
  } else {
    regex_abort("error met while parsing regex expression");
  }
}
