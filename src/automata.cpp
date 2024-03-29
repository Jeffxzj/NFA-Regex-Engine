#include "automata.hpp"

#include <unordered_map>

#include "utility.hpp"


std::optional<std::pair<size_t, size_t>> Automata::run() {
  std::unordered_map<RegGraph::NodePtr, size_t> node_map{};

  if (regex_unlikely(debug)) {
    std::cout << "---------- [ AUTOMATA ] ----------" << std::endl;

    for (auto ptr = graph.nodes.begin(); ptr != graph.nodes.end(); ++ptr) {
      node_map.emplace(ptr, node_map.size());
    }
  }

  stack.emplace_back(StackElem{
    .offset = 0,
    .node = graph.head,
    .index = 0,
    .loop = std::vector<size_t>{},
    .match_start = input.size(),
  });

  while (!stack.empty()) {
    auto &[offset, node, index, loop, match_start, finish] = stack.back();

    if (index == 0) {
      // this node is visited for the first time
      if (node->marker == NodeMarker::MATCH_BEGIN) {
        if (offset < match_start) { match_start = offset; }
      }
    }

    if (index < node->edges.size()) {
      auto &[edge, dest] = node->edges[index++];

      if (regex_unlikely(debug)) {
        std::cout
            << node_map[node] << ' ' << index << ' ' << match_start
            << " Edge " << "=> " << node_map[dest] << ": " << edge
            << std::endl;
      }

      switch (edge.type) {
        case EdgeType::EMPTY:
          stack.emplace_back(StackElem{
            .offset = offset,
            .node = dest,
            .index = 0,
            .loop = loop,
            .match_start = match_start,
          });
          break;
        case EdgeType::ENTER_LOOP: {
          auto new_loop = loop;
          new_loop.emplace_back(1);

          stack.emplace_back(StackElem{
            .offset = offset,
            .node = dest,
            .index = 0,
            .loop = std::move(new_loop),
            .match_start = match_start,
          });
          break;
        }
        case EdgeType::EXIT_LOOP: {
          auto new_loop = loop;
          if (edge.range.in_range(new_loop.back())) {
            new_loop.pop_back();
            stack.emplace_back(StackElem{
              .offset = offset,
              .node = dest,
              .index = 0,
              .loop = std::move(new_loop),
              .match_start = match_start,
            });
          }
          break;
        }
        case EdgeType::REPEAT: {
          auto new_loop = loop;

          if (edge.range.in_upper_range(++new_loop.back())) {
            stack.emplace_back(StackElem{
              .offset = offset,
              .node = dest,
              .index = 0,
              .loop = std::move(new_loop),
              .match_start = match_start,
            });
          }
          break;
        }
        case EdgeType::CONCATENATION:
          if (
              input.substr(offset).rfind(edge.string, 0) !=
              std::string_view::npos
          ) {
            stack.emplace_back(StackElem{
              .offset = offset + edge.string.size(),
              .node = dest,
              .index = 0,
              .loop = loop,
              .match_start = match_start,
            });
          }

          break;
        case EdgeType::CHARACTER_SET:
          if (offset < input.size() && edge.set.has_char(input[offset])) {
              stack.emplace_back(StackElem{
              .offset = offset + 1,
              .node = dest,
              .index = 0,
              .loop = loop,
              .match_start = match_start,
            });
          }
          break;
        default:
          regex_abort("unknown edge type");
      }
    } else {
      if (regex_unlikely(debug)) {
        std::cout
            << node_map[node] << ' ' << index << ' ' << match_start
            << " Leaving" << std::endl;
      }

      finish |= offset >= input.length();

      if (node->marker == NodeMarker::MATCH_END) {
        if (finish) { set_match(match_start, offset); }
      }

      stack.pop_back();
      if (!stack.empty()) { stack.back().finish |= finish; }
    }
  }

  return best_match;
}
