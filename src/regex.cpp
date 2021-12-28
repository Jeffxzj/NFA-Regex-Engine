#include "regex.hpp"

#include <iostream>

#include "tokenizer.hpp"
#include "parser.hpp"
#include "automata.hpp"


bool check_ascii(std::string_view regex) {
  for (int c : regex) { if (c >= 128 || c < 0) { return false; } }
  return true;
}

std::optional<Regex> Regex::init(std::string_view regex) {
  if (!check_ascii(regex)) { regex_warn("regex string includes none ascii"); }

  RegexTokenizer tokenizer{regex};
  Parser parser{tokenizer};

  if (auto error = parser.build_graph()) {
    regex_warn(error->c_str());
    return std::nullopt;
  } else {
    return Regex{std::move(parser.regex_graph)};
  }
}

std::optional<std::pair<size_t, size_t>> Regex::match(std::string_view input) {
  if (!check_ascii(input)) { regex_warn("input string includes none ascii"); }

  return Automata::accept(graph, input);
}
