#include "automata.hpp"

#include <iostream>
#include <unordered_map>


RegGraph RegGraph::single_edge(Edge &&edge) {
  RegGraph graph{};
  graph.head->add_edge(std::move(edge), graph.tail);
  return graph;
}

RegGraph RegGraph::join_graph(RegGraph &&graph1, RegGraph &&graph2) {
  graph1.merge_head(graph1.head, graph2);
  graph2.tail->add_empty_edge(graph1.tail);
  graph2.give_up_nodes(graph1);
  return std::move(graph1);
}

void RegGraph::concatenat_graph_continue(RegGraph &&graph) {
  if (graph.is_simple_empty_graph()) { return; }

  if (is_simple_empty_graph()) {
    *this = std::move(graph);
  } else if (is_simple_concatenation_graph()) {
    if (graph.is_simple_concatenation_graph()) {
      get_first_edge().first.string.append(
        graph.get_first_edge().first.string
      );
    }
  } else {
    merge_head(tail, graph);
    tail = graph.tail;
    graph.give_up_nodes(*this);
  }
}

void RegGraph::join_character_set_graph_continue(RegGraph &&graph) {
  if (!graph.is_simple_character_set_graph()) { exit(1); }

  get_first_edge().first.set |=
      graph.get_first_edge().first.set;
}

void RegGraph::repeat_graph(RepeatRange range) {
  // {1,1} has no effect
  if (range.lower_bound == 1 && range.upper_bound == 2) { return; }
  // {0,0} clears the graph
  if (range.lower_bound == 0 && range.upper_bound == 1) {
    *this = single_edge(Edge::empty());
    return;
  }
  // {0,1} does not introduce loop
  if (range.lower_bound == 0 && range.upper_bound == 2) {
    head->add_empty_edge(tail);
    return;
  }

  tail->add_edge(Edge::repeat(range), head);

  if (range.lower_bound == 0) {
    auto origin_tail = tail;
    tail = create_node();
    origin_tail->add_empty_edge(tail);
    head->add_empty_edge(tail);
  }
}

void RegGraph::character_set_complement() {
  if (!is_simple_character_set_graph()) { exit(1); }
  get_first_edge().first.set.complement();
}

void Node::add_edge(Edge &&edge, RegGraph::NodePtr next) {
  edges.emplace_back(std::move(edge), next);
}

std::ostream &operator<<(std::ostream &stream, RegGraph &other) {
  std::unordered_map<Node *, size_t> node_map{};

  for (auto &node : other.nodes) {
    node_map.emplace(&node, node_map.size());
  }

  for (auto &node : other.nodes) {
    stream << "NODE: " << node_map[&node] << '\n';
    for (auto &[edge, dest] : node.edges) {
      stream << '\t' << node_map[&*dest] << '\t' << edge << '\n';
    }
  }

  return stream;
}

std::ostream &operator<<(std::ostream &stream, const Edge &other) {
  switch (other.type) {
    case EdgeType::EMPTY:
      return stream << "EMPTY";
    case EdgeType::CONCATENATION:
      return stream << "CONCATENATION: " << other.string;
    case EdgeType::CHARACTER_SET:
      return stream << "CHARACTER_SET: " << other.set;
    case EdgeType::REPERAT:
      return stream << "REPERAT: " << other.range;
    default:
      return stream << "UNKNOWN";
  }
}
