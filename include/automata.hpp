#ifndef REGEX_AUTOMATA
#define REGEX_AUTOMATA


#include <vector>
#include <optional>
#include <string_view>
#include <unordered_set>

#include "utility.hpp"
#include "reg_graph.hpp"


class Automata {
private:
  struct StackElem {
    size_t offset;
    RegGraph::NodePtr node;
    size_t index;
    std::vector<size_t> loop;
    size_t match_start;
    std::unordered_set<RegGraph::NodePtr> no_consum{};
    bool finish{false};
  };

  RegGraph &graph;
  std::string_view input;
  std::vector<StackElem> stack;
  std::optional<std::pair<size_t, size_t>> best_match;
  bool debug;

  Automata(RegGraph &graph, std::string_view input) :
      graph{graph}, input{input}, stack{}, best_match{std::nullopt},
      debug{false}
  {
    debug =
        std::getenv("REGEX_DEBUG") != nullptr ||
        std::getenv("REGEX_AUTOMATA_DEBUG") != nullptr;
  }

  void set_match(size_t begin, size_t end) {
    if (regex_unlikely(debug)) {
      std::cout
          << "match: " << begin << ", " << end
          << ", "
          << make_escape(std::string(input.substr(begin, end - begin)))
          << std::endl;
    }

    if (best_match) {
      auto &[best_match_start, best_match_end] = best_match.value();

      if (end - begin > best_match_end - best_match_start) {
        best_match = std::make_pair(begin, end);
      } else if (
          end - begin == best_match_end - best_match_start &&
          begin < best_match_start
      ) {
        best_match = std::make_pair(begin, end);
      }
    } else {
      best_match = std::make_pair(begin, end);
    }

  }

  std::optional<std::pair<size_t, size_t>> run();

public:
  static std::optional<std::pair<size_t, size_t>>
  accept(RegGraph &graph, std::string_view input) {
    return Automata{graph, input}.run();
  }
};


#endif // REGEX_AUTOMATA
