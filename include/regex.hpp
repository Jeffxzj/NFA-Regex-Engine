#ifndef REGEX_REGEX
#define REGEX_REGEX


#include <optional>
#include <string_view>
#include <string>

#include "reg_graph.hpp"


class Regex {
private:
  RegGraph graph;

  Regex(RegGraph &&graph) : graph{std::move(graph)} {}

public:
  static std::optional<Regex> init(std::string_view regex);

  std::optional<std::string> match(std::string_view input);
};


#endif // REGEX_REGEX
