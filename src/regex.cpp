#include <iostream>

#include "regex.hpp"
#include "tokenizer.hpp"


std::optional<Regex> Regex::init(std::string_view regex) {
  RegexTokenizer tokenizer{regex};

  while (auto token = tokenizer.next()) {
    std::cout << token.value() << std::endl;
  }

  return Regex{};
}
