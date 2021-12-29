#ifndef REGREX_PARSER
#define REGREX_PARSER


#include <cstdlib>

#include "tokenizer.hpp"
#include "reg_graph.hpp"


class Parser {
public:
  using GraphStack = std::vector<std::pair<TokenType, std::vector<RegGraph>>>;
  GraphStack graph_stack;

  RegexTokenizer &tokenizer;
  RegGraph regex_graph;
  bool debug;


  std::optional<std::string> build_graph();
  RegGraph pop_and_join();

  Parser(RegexTokenizer &tokenizer) : tokenizer(tokenizer), debug{false} {
    debug =
        std::getenv("REGEX_DEBUG") != nullptr ||
        std::getenv("REGEX_PARSER_DEBUG") != nullptr;
  }
};


#endif // REGREX_PARSER
