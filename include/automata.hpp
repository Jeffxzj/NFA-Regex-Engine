#ifndef REGEX_AUTOMATA
#define REGEX_AUTOMATA


#include <vector>
#include <optional>
#include <string_view>

#include "reg_graph.hpp"


class Automata {
private:
  struct StackElem {
    size_t offset;
    RegGraph::NodePtr node;
    size_t index;
    std::vector<size_t> loop;
    size_t match_start;
    bool finish = false;
  };

  RegGraph &graph;
  std::string_view input;
  std::vector<StackElem> stack;
  size_t best_match_start;
  size_t best_match_end;

  Automata(RegGraph &graph, std::string_view input) :
      graph{graph}, input{input}, stack{},
      best_match_start{input.size()}, best_match_end{input.size()}
  {}

  // void extend_current();

  // void step();

  void set_match(size_t begin, size_t end) {
    std::cout
        << "match: " << begin << ", " << end
        << ", " << input.substr(begin, end - begin) << std::endl;
    if (end - begin > best_match_end - best_match_start) {
      best_match_start = begin;
      best_match_end = end;
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
