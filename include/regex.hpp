#include <optional>

class Regex {
private:
  Regex() {}

public:
  static std::optional<Regex> init(std::string_view regex);
};
