#include "regex.hpp"

#include <iostream>

#include "tokenizer.hpp"
#include "parser.hpp"
#include "automata.hpp"


std::optional<Regex> Regex::init(std::string_view regex) {
  RegexTokenizer tokenizer{regex};
  RegexTokenizer tokenizer_for_graph{tokenizer};

  while (auto token = tokenizer.next()) {
    std::cout << token.value() << std::endl;
  }

  Parser parser{tokenizer_for_graph};
  bool success = parser.build_graph();

  if (success) {
    return Regex{std::move(parser.regex_graph)};
  } else {
    return std::nullopt;
  }
}

std::optional<std::string> Regex::match(std::string_view input) {
  return Automata::accept(graph, input);
}
