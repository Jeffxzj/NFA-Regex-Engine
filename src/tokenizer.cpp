#include <iostream>

#include "tokenizer.hpp"


#define CASE_WHITESPACE \
  ' ': \
  case '\t': \
  case '\n': \
  case '\r': \
  case '\f': \
  case '\v'

#define CASE_NUMERIC \
  '0': \
  case '1': \
  case '2': \
  case '3': \
  case '4': \
  case '5': \
  case '6': \
  case '7': \
  case '8': \
  case '9'

#define CASE_LOWER_CASE \
  'a': \
  case 'b': \
  case 'c': \
  case 'd': \
  case 'e': \
  case 'f': \
  case 'g': \
  case 'h': \
  case 'i': \
  case 'j': \
  case 'k': \
  case 'l': \
  case 'm': \
  case 'n': \
  case 'o': \
  case 'p': \
  case 'q': \
  case 'r': \
  case 's': \
  case 't': \
  case 'u': \
  case 'v': \
  case 'w': \
  case 'x': \
  case 'y': \
  case 'z'

#define CASE_UPPER_CASE \
  'A': \
  case 'B': \
  case 'C': \
  case 'D': \
  case 'E': \
  case 'F': \
  case 'G': \
  case 'H': \
  case 'I': \
  case 'J': \
  case 'K': \
  case 'L': \
  case 'M': \
  case 'N': \
  case 'O': \
  case 'P': \
  case 'Q': \
  case 'R': \
  case 'S': \
  case 'T': \
  case 'U': \
  case 'V': \
  case 'W': \
  case 'X': \
  case 'Y': \
  case 'Z'

#define CASE_ALPHABET \
  CASE_LOWER_CASE : \
  case CASE_UPPER_CASE


std::ostream &
operator<<(std::ostream &stream, const RegexToken &other) {
  switch (other.type) {
    case TokenType::ATOM:
      return stream << "ATOM: " << other.value;
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
    case TokenType::RANGE:
      return stream << "RANGE: " << other.value;
    case TokenType::CHARACTER_CLASS:
      return stream << "CHARACTER_CLASS: " << other.value;
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
    case TokenType::ERROR:
      return stream << "ERROR: " << other.value;
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
          return error("");
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
          return RegexToken{TokenType::NUMERIC, std::move(buffer)};
      }
    }
  }
break_braces:
  switch (auto c = regex[index++]) {
    case '(':
      stack.emplace_back('(');
      ++parenthetheses_depth;
      return TokenType::LEFT_PARENTHESES;
    case ')':
      if (in_parenthetheses()) {
        stack.pop_back();
        --parenthetheses_depth;
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
      if (finish()) { return TokenType::LEFT_BRACKETS; }

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
                  return RegexToken{TokenType::CHARACTER_CLASS, buffer};
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
      if (in_braces() && regex[index - 2] != '[') {
        return error("unexpected hyphen");
      } else {
        buffer.push_back(c);
        break;
      }

    case '^':
      return TokenType::MATCH_BEGIN;
    case '$':
      return TokenType::MATCH_END;
    case '*':
      return TokenType::ASTERISK;
    case '+':
      return TokenType::PLUS_SIGN;
    case '?':
      return TokenType::QUESTION_MARK;
    case '.':
      return TokenType::PERIOD;
    case '|':
      return TokenType::VERTICAL_BAR;

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
      case '(':
      case ')':
      case '{':
      case '}':
      case '[':
      case ']':
      case '^':
      case '$':
      case '*':
      case '+':
      case '?':
      case '.':
      case '|':
        --index;
        return RegexToken{TokenType::ATOM, std::move(buffer)};
      case '-':
        if (in_brackets()) {
          if (!finish() && buffer.size() == 1) {
            switch (auto c = regex[index++]) {
              case ']':
                --index;
                return RegexToken{TokenType::ATOM, std::move(buffer)};
              case CASE_NUMERIC:
              case CASE_ALPHABET:
                buffer.push_back(c);
                return RegexToken{TokenType::RANGE, std::move(buffer)};
              default:
                return error("unexpected charactor in range");
            }
          } else {
            index -= 2;
            buffer.pop_back();
            return RegexToken{TokenType::ATOM, std::move(buffer)};
          }
        }
        --index;
        return RegexToken{TokenType::ATOM, std::move(buffer)};

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

  return RegexToken{TokenType::ATOM, std::move(buffer)};
}
