#ifndef REGEX_TOKENIZER
#define REGEX_TOKENIZER


#include <cstdint>
#include <string_view>
#include <string>
#include <vector>
#include <optional>
#include <new>

#include "utility.hpp"


enum class TokenType {
  // errors
  ERROR,
  // strings
  ATOM,
  // parentheses related
  LEFT_PARENTHESES,
  RIGHT_PARENTHESES,
  // braces related
  LEFT_BRACES,
  RIGHT_BRACES,
  NUMERIC,
  COMMA,
  // brackets related
  LEFT_BRACKETS,
  LEFT_BRACKETS_NOT,
  RIGHT_BRACKETS,
  CHARACTER_RANGE,
  // character class
  CHARACTER_CLASS_UPPER,
  CHARACTER_CLASS_LOWER,
  CHARACTER_CLASS_ALPHA,
  CHARACTER_CLASS_DIGIT,
  CHARACTER_CLASS_XDIGIT,
  CHARACTER_CLASS_ALNUM,
  CHARACTER_CLASS_PUNCT,
  CHARACTER_CLASS_BLANK,
  CHARACTER_CLASS_SPACE,
  CHARACTER_CLASS_CNTRL,
  CHARACTER_CLASS_GRAPH,
  CHARACTER_CLASS_PRINT,
  CHARACTER_CLASS_WORD,
  // match begin and end
  MATCH_BEGIN,
  MATCH_END,
  // other symbol
  ASTERISK,
  PLUS_SIGN,
  QUESTION_MARK,
  PERIOD,
  VERTICAL_BAR,
};

class RegexToken {
private:
  RegexToken(TokenType type, std::string value) : type{type}, string{value} {}

  RegexToken(TokenType type, size_t value) : type{type}, value{value} {}

  RegexToken(TokenType type, char lower, char upper) :
      type{type}, range{lower, upper} {}

  void drop() {
    switch(type) {
      case TokenType::ATOM:
      case TokenType::ERROR:
        string.~basic_string();
        break;
      default:
        break;
    }
  }

  void copy(const RegexToken &other) {
    type = other.type;

    switch(type) {
      case TokenType::ATOM:
      case TokenType::ERROR:
        new(&string) std::string{other.string};
        break;
      case TokenType::NUMERIC:
        value = other.value;
        break;
      case TokenType::CHARACTER_RANGE:
        range = other.range;
        break;
      default:
        break;
    }
  }

  void emplace(RegexToken &&other) {
    type = other.type;

    switch(type) {
      case TokenType::ATOM:
      case TokenType::ERROR:
        new(&string) std::string{std::move(other.string)};
        break;
      case TokenType::NUMERIC:
        value = other.value;
        break;
      case TokenType::CHARACTER_RANGE:
        range = other.range;
        break;
      default:
        break;
    }
  }

public:
  TokenType type;
  union {
    struct {} null;
    std::string string;
    size_t value;
    CharacterRange range;
  };

  static RegexToken error(const char *reason) {
    return RegexToken{TokenType::ERROR, reason};
  }

  static RegexToken atom(std::string &&name) {
    return RegexToken{TokenType::ATOM, std::move(name)};
  }

  static RegexToken numeric(const std::string &name) {
    size_t value = 0, value_max = std::numeric_limits<size_t>::max();

    for (size_t i = 0; i < name.size(); ++i) {
      size_t digit = name[i] - '0';
      if (value >= (value_max - digit) / 10) {
        return error("number exceeds maximum boundary");
      } else {
        value = value * 10 + digit;
      }
    }

    return RegexToken{TokenType::NUMERIC, value};
  }

  static RegexToken character_range(const std::string &name) {
    switch (name[0]) {
      case CASE_NUMERIC:
        switch (name[2]) {
          case CASE_NUMERIC:
            break;
          default:
            return error("invalid range");
        }
        break;
      case CASE_LOWER_CASE:
        switch (name[2]) {
          case CASE_LOWER_CASE:
            break;
          default:
            return error("invalid range");
        }
        break;
      case CASE_UPPER_CASE:
        switch (name[2]) {
          case CASE_UPPER_CASE:
            break;
          default:
            return error("invalid range");
        }
        break;
      default:
        return error("invalid range");
    }

    if (name[0] > name[2]) { return error("invalid range"); }
    return RegexToken{TokenType::CHARACTER_RANGE, name[0], name[2]};
  }

  static RegexToken character_class(const std::string &name) {
    if (name == "upper") {
      return TokenType::CHARACTER_CLASS_UPPER;
    } else if (name == "lower") {
      return TokenType::CHARACTER_CLASS_LOWER;
    } else if (name == "alpha") {
      return TokenType::CHARACTER_CLASS_ALPHA;
    } else if (name == "digit") {
      return TokenType::CHARACTER_CLASS_DIGIT;
    } else if (name == "xdigit") {
      return TokenType::CHARACTER_CLASS_XDIGIT;
    } else if (name == "alnum") {
      return TokenType::CHARACTER_CLASS_ALNUM;
    } else if (name == "punct") {
      return TokenType::CHARACTER_CLASS_PUNCT;
    } else if (name == "blank") {
      return TokenType::CHARACTER_CLASS_BLANK;
    } else if (name == "space") {
      return TokenType::CHARACTER_CLASS_SPACE;
    } else if (name == "cntrl") {
      return TokenType::CHARACTER_CLASS_CNTRL;
    } else if (name == "graph") {
      return TokenType::CHARACTER_CLASS_GRAPH;
    } else if (name == "print") {
      return TokenType::CHARACTER_CLASS_PRINT;
    } else if (name == "word") {
      return TokenType::CHARACTER_CLASS_WORD;
    } else {
      return error("unknown charactor class");
    }
  }

  RegexToken(TokenType type) : type{type}, null{} {}

  bool is_error() { return type == TokenType::ERROR; }

  friend std::ostream &
  operator<<(std::ostream &stream, const RegexToken &other);

  RegexToken(const RegexToken &other) { copy(other); }

  RegexToken(RegexToken &&other) noexcept { emplace(std::move(other)); }

  RegexToken &operator=(const RegexToken &other) {
    drop();
    copy(other);
    return *this;
  }

  RegexToken &operator=(RegexToken &&other) noexcept {
    drop();
    emplace(std::move(other));
    return *this;
  }

  ~RegexToken() { drop(); }
};

class RegexTokenizer {
private:
  std::string_view regex;
  std::vector<char> stack;
  size_t index;
  size_t parentheses_depth;
  size_t braces_depth;
  size_t brackets_depth;

  bool in_parentheses() { return !stack.empty() && stack.back() == '('; }

  bool in_braces() { return !stack.empty() && stack.back() == '{'; }

  bool in_brackets() { return !stack.empty() && stack.back() == '['; }

  bool finish() { return index >= regex.size(); }

  void clear() {
    stack.clear();
    index = regex.size();
    parentheses_depth = 0;
    braces_depth = 0;
    brackets_depth = 0;
  }

  std::optional<RegexToken> error(const char *reason) {
    clear();
    return RegexToken::error(reason);
  }

public:
  explicit RegexTokenizer(std::string_view regex) :
      regex(regex), stack{}, index(0), parentheses_depth{0},
      braces_depth{0}, brackets_depth{0} {}

  std::optional<RegexToken> next();
};


#endif // REGEX_TOKENIZER
