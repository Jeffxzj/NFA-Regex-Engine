#ifndef REGREX_PARSER
#define REGREX_PARSER


#include "tokenizer.hpp"
#include "reg_graph.hpp"



class Parser {
public:
  RegexTokenizer &tokenizer;
  RegGraph regex_graph;

  bool build_graph();

  Parser(RegexTokenizer &tokenizer) : tokenizer(tokenizer) {}
};


#endif // REGREX_PARSER
