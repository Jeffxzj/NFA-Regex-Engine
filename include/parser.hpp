#ifndef REGREX_PARSER
#define REGREX_PARSER


#include "tokenizer.hpp"

class Parser {
private:
  RegexTokenizer &tokenizer;

public:
  void build_graph();

  Parser(RegexTokenizer &tokenizer) : tokenizer(tokenizer) {}
};


#endif // REGREX_PARSER
