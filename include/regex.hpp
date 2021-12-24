#ifndef REGEX_REGEX
#define REGEX_REGEX


#include <optional>


class Regex {
private:
  Regex() {}

public:
  static std::optional<Regex> init(std::string_view regex);
};


#endif // REGEX_REGEX
