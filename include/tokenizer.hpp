#include <cstdint>
#include <string_view>
#include <string>
#include <vector>
#include <optional>

enum class TokenType {
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
  RANGE,
  CHARACTER_CLASS,
  // match begin and end
  MATCH_BEGIN,
  MATCH_END,
  // other symbol
  ASTERISK,
  PLUS_SIGN,
  QUESTION_MARK,
  PERIOD,
  VERTICAL_BAR,
  // errors
  ERROR,
};

class RegexToken {
private:
  TokenType type;
  std::string value;

public:
  RegexToken(TokenType type) : type{type}, value{} {}

  RegexToken(TokenType type, std::string value) : type{type}, value{value} {}

  bool is_error() { return type == TokenType::ERROR; }

  friend std::ostream &
  operator<<(std::ostream &stream, const RegexToken &other);
};

class RegexTokenizer {
private:
  std::string_view regex;
  std::vector<char> stack;
  size_t index;
  size_t parenthetheses_depth;
  size_t braces_depth;
  size_t brackets_depth;

  std::optional<RegexToken> error(const char *reason) {
    stack.clear();
    index = regex.size();
    parenthetheses_depth = 0;
    braces_depth = 0;
    brackets_depth = 0;
    return RegexToken{TokenType::ERROR, reason};
  }

  bool in_parenthetheses() { return !stack.empty() && stack.back() == '('; }

  bool in_braces() { return !stack.empty() && stack.back() == '{'; }

  bool in_brackets() { return !stack.empty() && stack.back() == '['; }

  bool finish() { return index >= regex.size(); }

  char escape_char(char origin) {
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

public:
  explicit RegexTokenizer(std::string_view regex) :
      regex(regex), stack{}, index(0), parenthetheses_depth{0},
      braces_depth{0}, brackets_depth{0} {}

  std::optional<RegexToken> next();
};
