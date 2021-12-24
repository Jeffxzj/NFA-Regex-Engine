#include "parser.hpp"
#include "tokenizer.hpp"


void Parser::build_graph() {

  std::vector<std::vector<RegexToken>> stack;
  while (auto token = tokenizer.next()) {
    int flag = 0;
    std::vector<RegexToken> buffer; // buffer to store minimal graph
    switch (token.value().type) {
      case TokenType::ATOM:
        buffer.emplace_back(token.value());
        stack.emplace_back(buffer);
        break;
      case TokenType::ASTERISK:
        // std::vector<RegexToken> token_buf = stack.pop_back();
        break;
      
      case TokenType::LEFT_PARENTHESES:
        // buf = stack.back();
        // stack.pop_back()
        buffer.emplace_back(token.value());

      case TokenType::LEFT_BRACKETS:
        buffer.emplace_back(token.value());
        break;
      
      case TokenType::LEFT_BRACES:
        buffer.emplace_back(token.value());
        break;
      case TokenType::RIGHT_BRACES:
        
        break;        
      case TokenType::RIGHT_PARENTHESES:
        break;
      case TokenType::RIGHT_BRACKETS:
        break;
      
      case TokenType::VERTICAL_BAR:
        break;

    }
    // std::cout << token.value() << std::endl;
  }
}
