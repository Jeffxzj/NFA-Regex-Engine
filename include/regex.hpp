#ifndef REGEX_REGEX
#define REGEX_REGEX


#include <optional>
#include <string_view>


class Regex {
private:
  Regex() {}

public:
  static std::optional<Regex> init(std::string_view regex);
};


#endif // REGEX_REGEX
