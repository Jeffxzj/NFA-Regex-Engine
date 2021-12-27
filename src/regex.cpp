#include "regex.hpp"

#include <iostream>

#include "tokenizer.hpp"
#include "parser.hpp"


std::optional<Regex> Regex::init(std::string_view regex) {
  RegexTokenizer tokenizer{regex};
  RegexTokenizer tokenizer_for_graph{tokenizer};

  while (auto token = tokenizer.next()) {
    std::cout << token.value() << std::endl;
  }
  
  Parser parser{tokenizer_for_graph};
  bool success = parser.build_graph();
  // std::cout << success << std::endl;

  return Regex{};
}
