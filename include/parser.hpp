#ifndef REGREX_PARSER
#define REGREX_PARSER


#include "tokenizer.hpp"
#include "reg_graph.hpp"



class Parser {
public:
  RegexTokenizer &tokenizer;
  RegGraph regex_graph;
  using GraphStack = std::vector<std::pair<TokenType, std::vector<RegGraph>>>;
  void read_right_of_bar(GraphStack &stack);

  std::optional<std::string> build_graph();

  Parser(RegexTokenizer &tokenizer) : tokenizer(tokenizer) {}
};


#endif // REGREX_PARSER
