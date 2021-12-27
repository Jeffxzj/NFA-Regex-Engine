#include "tokenizer.hpp"

#include <iostream>


static char escape_char(char origin) {
  switch(origin) {
    case 't':
      return '\t';
    case 'n':
      return '\n';
    case 'r':
      return '\r';
    case 'f':
      return '\f';
    case 'v':
      return '\v';
    default:
      return origin;
  }
}


std::ostream &operator<<(std::ostream &stream, const RegexToken &other) {
  switch (other.type) {
    case TokenType::ATOM:
      return stream << "ATOM: " << other.string;
    case TokenType::ERROR:
      return stream << "ERROR: " << other.string;
    case TokenType::LEFT_PARENTHESES:
      return stream << "LEFT_PARENTHESES";
    case TokenType::RIGHT_PARENTHESES:
      return stream << "RIGHT_PARENTHESES";
    case TokenType::LEFT_BRACES:
      return stream << "LEFT_BRACES";
    case TokenType::RIGHT_BRACES:
      return stream << "RIGHT_BRACES";
    case TokenType::NUMERIC:
      return stream << "NUMERIC: " << other.value;
    case TokenType::COMMA:
      return stream << "COMMA";
    case TokenType::LEFT_BRACKETS:
      return stream << "LEFT_BRACKETS";
    case TokenType::LEFT_BRACKETS_NOT:
      return stream << "LEFT_BRACKETS_NOT";
    case TokenType::RIGHT_BRACKETS:
      return stream << "RIGHT_BRACKETS";
    case TokenType::CHARACTER_RANGE:
      return stream << "CHARACTER_RANGE: " << other.range;
    case TokenType::CHARACTER_CLASS_UPPER:
      return stream << "CHARACTER_CLASS_UPPER";
    case TokenType::CHARACTER_CLASS_LOWER:
      return stream << "CHARACTER_CLASS_LOWER";
    case TokenType::CHARACTER_CLASS_ALPHA:
      return stream << "CHARACTER_CLASS_ALPHA";
    case TokenType::CHARACTER_CLASS_DIGIT:
      return stream << "CHARACTER_CLASS_DIGIT";
    case TokenType::CHARACTER_CLASS_XDIGIT:
      return stream << "CHARACTER_CLASS_XDIGIT";
    case TokenType::CHARACTER_CLASS_ALNUM:
      return stream << "CHARACTER_CLASS_ALNUM";
    case TokenType::CHARACTER_CLASS_PUNCT:
      return stream << "CHARACTER_CLASS_PUNCT";
    case TokenType::CHARACTER_CLASS_BLANK:
      return stream << "CHARACTER_CLASS_BLANK";
    case TokenType::CHARACTER_CLASS_SPACE:
      return stream << "CHARACTER_CLASS_SPACE";
    case TokenType::CHARACTER_CLASS_CNTRL:
      return stream << "CHARACTER_CLASS_CNTRL";
    case TokenType::CHARACTER_CLASS_GRAPH:
      return stream << "CHARACTER_CLASS_GRAPH";
    case TokenType::CHARACTER_CLASS_PRINT:
      return stream << "CHARACTER_CLASS_PRINT";
    case TokenType::CHARACTER_CLASS_WORD:
      return stream << "CHARACTER_CLASS_WORD";
    case TokenType::MATCH_BEGIN:
      return stream << "MATCH_BEGIN";
    case TokenType::MATCH_END:
      return stream << "MATCH_END";
    case TokenType::ASTERISK:
      return stream << "ASTERISK";
    case TokenType::PLUS_SIGN:
      return stream << "PLUS_SIGN";
    case TokenType::QUESTION_MARK:
      return stream << "QUESTION_MARK";
    case TokenType::PERIOD:
      return stream << "PERIOD";
    case TokenType::VERTICAL_BAR:
      return stream << "VERTICAL_BAR";
    default:
      return stream << "UNKNOWN";
  }
}

std::optional<RegexToken> RegexTokenizer::next() {
  std::string buffer{};

  if (finish()) {
    if (!stack.empty()) {
      return error("unmatched left parentheses/braces/brackets");
    } else {
      return std::nullopt;
    }
  }
  // in braces, only number and comma is allowed
  if (in_braces()) {
    while (!finish()) {
      switch (auto c = regex[index++]) {
        case '}':
          --index;
          goto break_braces;
        case ',':
          return TokenType::COMMA;
        case CASE_NUMERIC:
          buffer.push_back(c);
          goto match_numeric;
        default:
          return error("unexpected charactor in range");
      }
    }
match_numeric:
    while (!finish()) {
      switch (auto c = regex[index++]) {
        case CASE_NUMERIC:
          buffer.push_back(c);
          break;
        default:
          --index;
          auto token = RegexToken::numeric(buffer);
          if (token.is_error()) { clear(); }
          return token;
      }
    }
  }
break_braces:
  switch (auto c = regex[index++]) {
    case '(':
      stack.emplace_back('(');
      ++parentheses_depth;
      return TokenType::LEFT_PARENTHESES;
    case ')':
      if (in_parentheses()) {
        stack.pop_back();
        --parentheses_depth;
        return TokenType::RIGHT_PARENTHESES;
      } else {
        return error("unmatched right parentheses");
      }
    case '{':
      if (braces_depth > 0) {
        return error("braces cannot be nested in braces");
      }

      stack.emplace_back('{');
      ++braces_depth;
      return TokenType::LEFT_BRACES;
    case '}':
      if (in_braces()) {
        stack.pop_back();
        --braces_depth;
        return TokenType::RIGHT_BRACES;
      } else {
        return error("unmatched right braces");
      }
    case '[':
      if (finish()) {
        stack.emplace_back('[');
        ++brackets_depth;
        return TokenType::LEFT_BRACKETS;
      }

      switch (regex[index++]) {
        case '^':
          stack.emplace_back('[');
          ++brackets_depth;
          return TokenType::LEFT_BRACKETS_NOT;
        case ':':
          while (!finish()) {
            switch (auto c = regex[index++]) {
              case ':':
                if (!finish() && regex[index++] == ']') {
                  return RegexToken::character_class(buffer);
                } else {
                  return error("unexpected character class");
                }
              case CASE_LOWER_CASE:
                buffer.push_back(c);
                break;
              default:
                return error("unexpected character class");
            }
          }
          return error("unexpected character class");
        default:
          stack.emplace_back('[');
          ++brackets_depth;
          --index;
          return TokenType::LEFT_BRACKETS;
      }
    case ']':
      if (in_brackets()) {
        stack.pop_back();
        --brackets_depth;
        return TokenType::RIGHT_BRACKETS;
      } else {
        return error("unmatched right brackets");
      }
    case '-':
      if (in_brackets() && regex[index - 2] != '[') {
        return error("unexpected hyphen");
      } else {
        buffer.push_back(c);
        break;
      }
    case '^':
      if (index == 1) {
        return TokenType::MATCH_BEGIN;
      } else {
        buffer.push_back(c);
        break;
      }
    case '$':
      if (finish()) {
        return TokenType::MATCH_END;
      } else {
        buffer.push_back(c);
        break;
      }
    case '*':
      if (in_brackets()) {
        buffer.push_back(c);
        break;
      } else {
        return TokenType::ASTERISK;
      }
    case '+':
      if (in_brackets()) {
        buffer.push_back(c);
        break;
      } else {
        return TokenType::PLUS_SIGN;
      }
    case '?':
      if (in_brackets()) {
        buffer.push_back(c);
        break;
      } else {
        return TokenType::QUESTION_MARK;
      }
    case '.':
      if (in_brackets()) {
        buffer.push_back(c);
        break;
      } else {
        return TokenType::PERIOD;
      }      
    case '|':
      if (in_brackets()) {
        buffer.push_back(c);
        break;
      } else {
        return TokenType::VERTICAL_BAR;
      }      
    case '\\':
      if (finish()) {
        return error("escape at the end of expression");
      }
      buffer.push_back(escape_char(regex[index++]));
      break;
    default:
      buffer.push_back(c);
      break;
  }
  // now matching atom
  while (!finish()) {
    switch (auto c = regex[index++]) {
      case '*':
      case '+':
      case '?':
      case '.':
      case '|':
        if (in_brackets()) {
          buffer.push_back(c);
          break;
        } else {
          --index;
          return RegexToken::atom(std::move(buffer));
        }
      case '(':
      case ')':
      case '{':
      case '}':
      case '[':
      case ']':
        --index;
        return RegexToken::atom(std::move(buffer));
      case '$':
        if (finish()) {
          index--;
          return RegexToken::atom(std::move(buffer));
        } else {
          buffer.push_back(c);
          break;
        }
      case '-':
        buffer.push_back(c);
        if (in_brackets()) {
          if (!finish() && buffer.size() == 2) {
            switch (auto c = regex[index++]) {
              case ']':
                --index;
                return RegexToken::atom(std::move(buffer));
              case CASE_NUMERIC:
              case CASE_LOWER_CASE:
              case CASE_UPPER_CASE: {
                buffer.push_back(c);
                auto token = RegexToken::character_range(std::move(buffer));
                if (token.is_error()) { clear(); }
                return token;
              }
              default:
                return error("unexpected charactor in range");
            }
          } else {
            index -= 2;
            buffer.pop_back();
            buffer.pop_back();
            return RegexToken::atom(std::move(buffer));
          }
        }
        break;
      case '\\':
        if (index >= regex.size()) {
          return error("escape at the end of expression");
        }
        buffer.push_back(escape_char(regex[index++]));
        break;
      default:
        buffer.push_back(c);
        break;
    }
  }

  return RegexToken::atom(std::move(buffer));
}
