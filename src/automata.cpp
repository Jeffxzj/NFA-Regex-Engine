#include "automata.hpp"

#include <iostream>


std::ostream &operator<<(std::ostream &stream, const Edge &other) {
  switch (other.type) {
    case EdgeType::EMPTY:
      return stream << "EMPTY";
    case EdgeType::CONCATENATION:
      return stream << "CONCATENATION: " << other.string;
    case EdgeType::CHARACTER_UNION:
      return stream << "CHARACTER_UNION: " << other.string;
    case EdgeType::REPERAT:
      return stream
          << "REPERAT: " << other.repeat_range.lower_bound
          << '-' << other.repeat_range.upper_bound;
    case EdgeType::CHARACTER_RANGE:
            return stream
          << "CHARACTER_RANGE: " << other.character_range.lower_bound
          << '-' << other.character_range.upper_bound;
    case EdgeType::CHARACTER_CLASS_UPPER:
      return stream << "CHARACTER_CLASS_UPPER";
    case EdgeType::CHARACTER_CLASS_LOWER:
      return stream << "CHARACTER_CLASS_LOWER";
    case EdgeType::CHARACTER_CLASS_ALPHA:
      return stream << "CHARACTER_CLASS_ALPHA";
    case EdgeType::CHARACTER_CLASS_DIGIT:
      return stream << "CHARACTER_CLASS_DIGIT";
    case EdgeType::CHARACTER_CLASS_XDIGIT:
      return stream << "CHARACTER_CLASS_XDIGIT";
    case EdgeType::CHARACTER_CLASS_ALNUM:
      return stream << "CHARACTER_CLASS_ALNUM";
    case EdgeType::CHARACTER_CLASS_PUNCT:
      return stream << "CHARACTER_CLASS_PUNCT";
    case EdgeType::CHARACTER_CLASS_BLANK:
      return stream << "CHARACTER_CLASS_BLANK";
    case EdgeType::CHARACTER_CLASS_SPACE:
      return stream << "CHARACTER_CLASS_SPACE";
    case EdgeType::CHARACTER_CLASS_CNTRL:
      return stream << "CHARACTER_CLASS_CNTRL";
    case EdgeType::CHARACTER_CLASS_GRAPH:
      return stream << "CHARACTER_CLASS_GRAPH";
    case EdgeType::CHARACTER_CLASS_PRINT:
      return stream << "CHARACTER_CLASS_PRINT";
    case EdgeType::CHARACTER_CLASS_WORD:
      return stream << "CHARACTER_CLASS_WORD";
    default:
      return stream << "UNKNOWN";
  }
}

RegGraph RegGraph::single_edge(Edge &&edge) {
  RegGraph graph{};
  graph.head->add_edge(std::move(edge), graph.tail);
  return graph;
}

RegGraph RegGraph::repeat_graph(RegGraph &&graph, RepeatRange range) {
  auto origin_head = graph.head;
  auto origin_tail = graph.tail;

  graph.head = graph.create_node();
  graph.tail = graph.create_node();

  graph.head->add_empty_edge(origin_head);
  origin_tail->add_empty_edge(graph.tail);

  if (range.lower_bound == 0) {
    graph.head->add_empty_edge(graph.tail);
  }

  origin_tail->add_edge(Edge::repeat(range), origin_tail);

  return std::move(graph);
}

void RegGraph::join_graph_continue(RegGraph &&graph) {
  head->add_empty_edge(graph.head);
  graph.tail->add_empty_edge(graph.tail);
  graph.give_up_nodes(*this);
}

void RegGraph::concatenat_graph_continue(RegGraph &&graph) {
  tail->add_empty_edge(graph.head);
  tail = graph.tail;

  graph.give_up_nodes(*this);
}
