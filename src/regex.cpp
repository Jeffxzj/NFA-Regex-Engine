#include "regex.hpp"

#include <iostream>

#include "tokenizer.hpp"
#include "parser.hpp"
#include "automata.hpp"


std::optional<Regex> Regex::init(std::string_view regex) {
  RegexTokenizer tokenizer{regex};
  Parser parser{tokenizer};

  std::cout << "=========== [TOKENIZER ] ===========" << std::endl;

  if (auto error = parser.build_graph()) {
    regex_warn(error->c_str());
    return std::nullopt;
  } else {
    std::cout << "=========== [  PARSER  ] ===========" << std::endl;
    std::cout << parser.regex_graph;

    return Regex{std::move(parser.regex_graph)};
  }
}

std::optional<std::pair<size_t, size_t>> Regex::match(std::string_view input) {
  std::cout << "=========== [ AUTOMATA ] ===========" << std::endl;
  return Automata::accept(graph, input);
}
