#include "tokenizer.hpp"

#include <iostream>


static char escape_char(char origin) {
  switch(origin) {
    case '0':
      return '\0';
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
      return stream << "ATOM: " << make_escape(other.string);
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

std::optional<RegexToken> RegexTokenizer::handle_character_class() {
  std::string buffer{};

  while (true) {
    if (regex_unlikely(finish())) {
      return error("unexpected character class");
    }

    switch (auto c = regex[index++]) {
      case ':':
        if (regex_likely(!finish() && regex[index++] == ']')) {
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
}

std::optional<RegexToken> RegexTokenizer::handle_braces() {
  switch (regex[index++]) {
    case '}':
      stack.pop_back();
      return TokenType::RIGHT_BRACES;
    case ',':
      return TokenType::COMMA;
    case CASE_NUMERIC:
      --index;
      break;
    default:
      return error("unexpected character in range");
  }

  std::string buffer{};

  while (true) {
    if (regex_unlikely(finish())) {
      return handle_error(RegexToken::numeric(buffer));
    }

    switch (auto c = regex[index++]) {
      case CASE_NUMERIC:
        buffer.push_back(c);
        break;
      default:
        --index;
        return handle_error(RegexToken::numeric(buffer));
    }
  }
}

std::optional<RegexToken> RegexTokenizer::handle_brackets() {
  switch (regex[index++]) {
    case ']':
      stack.pop_back();
      return TokenType::RIGHT_BRACKETS;
    case '[':
      if (regex_likely(!finish() && regex[index++] == ':')) {
        return handle_character_class();
      } else {
        return error("nested brackets not allowed");
      }
    default:
      --index;
      break;
  }

  std::string buffer{};

  while(true) {
    if (regex_unlikely(finish())) {
      return RegexToken::atom(std::move(buffer));
    }

    switch (auto c = regex[index++]) {
      case ']':
      case '[':
        --index;
        return RegexToken::atom(std::move(buffer));
      case '-':
        if (regex_unlikely(finish())) {
          buffer.push_back(c);
          break;
        }

        if (buffer.size() == 1) {
          buffer.push_back(c);
          switch (auto c = regex[index++]) {
            case ']':
              --index;
              return RegexToken::atom(std::move(buffer));
            case CASE_NUMERIC:
            case CASE_LOWER_CASE:
            case CASE_UPPER_CASE:
              buffer.push_back(c);
              return handle_error(RegexToken::character_range(buffer));
            default:
              return error("unexpected character in range");
          }
        } else if (buffer.size() == 0) {
          regex_assert(regex[index - 2] == '[');
          buffer.push_back(c);
          break;
        } else {
          index -= 2;
          buffer.pop_back();
          return RegexToken::atom(std::move(buffer));
        }
      case '\\':
        if (regex_unlikely(finish())) {
          return error("escape at the end of expression");
        }
        buffer.push_back(escape_char(regex[index++]));
        break;
      default:
        buffer.push_back(c);
        break;
    }
  }
}

std::optional<RegexToken> RegexTokenizer::handle_parentheses() {
  // match parentheses
  switch (regex[index++]) {
    case '(':
      stack.emplace_back('(');
      return TokenType::LEFT_PARENTHESES;
    case ')':
      if (in_parentheses()) {
        stack.pop_back();
        return TokenType::RIGHT_PARENTHESES;
      } else {
        return error("unmatched right parentheses");
      }
    case '{':
      stack.emplace_back('{');
      return TokenType::LEFT_BRACES;
    case '}':
      return error("unmatched right braces");
    case '[':
      if (regex_unlikely(finish())) {
        stack.emplace_back('[');
        return TokenType::LEFT_BRACKETS;
      }

      switch (regex[index++]) {
        case '^':
          stack.emplace_back('[');
          return TokenType::LEFT_BRACKETS_NOT;
        default:
          --index;
          stack.emplace_back('[');
          return TokenType::LEFT_BRACKETS;
      }
    case ']':
      return error("unmatched right brackets");
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
    case '^':
      if (index == 1) {
        return TokenType::MATCH_BEGIN;
      } else {
        --index;
        break;
      }
    case '$':
      if (finish()) {
        return TokenType::MATCH_END;
      } else {
        --index;
        break;
      }
    default:
      --index;
      break;
  }

  std::string buffer{};

  while (true) {
    if (finish()) {
      return RegexToken::atom(std::move(buffer));
    }

    switch (auto c = regex[index++]) {
      case '(':
      case ')':
      case '{':
      case '}':
      case '[':
      case ']':
      case '.':
      case '|':
        --index;
        return RegexToken::atom(std::move(buffer));
      case '*':
      case '+':
      case '?':
        if (buffer.size() > 1) {
          index -= 2;
          buffer.pop_back();
        } else {
          --index;
        }
        return RegexToken::atom(std::move(buffer));
      case '$':
        if (finish()) {
          index--;
          return RegexToken::atom(std::move(buffer));
        } else {
          buffer.push_back(c);
          break;
        }
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
}

std::optional<RegexToken> RegexTokenizer::next() {
  if (regex_unlikely(debug && index == 0)) {
    std::cout << "---------- [TOKENIZER ] ----------" << std::endl;
  }

  std::optional<RegexToken> result = std::nullopt;

  if (finish()) {
    if (!stack.empty()) {
      result = error("unmatched left parentheses/braces/brackets");
    }
  } else {
    if (in_braces()) {
      result = handle_braces();
    } else if (in_brackets()) {
      result = handle_brackets();
    } else {
      result = handle_parentheses();
    }
  }

  if (regex_unlikely(debug && result)) {
    std::cout << result.value() << std::endl;
  }

  return result;
}
