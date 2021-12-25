#include "automata.hpp"

#include <iostream>


RegGraph RegGraph::single_edge(Edge &&edge) {
  RegGraph graph{};
  graph.head->add_edge(std::move(edge), graph.tail);
  return graph;
}

RegGraph RegGraph::complement(RegGraph &&graph) {
  return std::move(graph);
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

std::ostream &operator<<(std::ostream &stream, const Edge &other) {
  switch (other.type) {
    case EdgeType::EMPTY:
      return stream << "EMPTY";
    case EdgeType::CONCATENATION:
      return stream << "CONCATENATION: " << other.string;
    case EdgeType::CHARACTER_SET:
      return stream << "CHARACTER_SET: " << other.character_set;
    case EdgeType::REPERAT:
      return stream << "REPERAT: " << other.repeat_range;
    default:
      return stream << "UNKNOWN";
  }
}
