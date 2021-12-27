#include "reg_graph.hpp"

#include <iostream>
#include <unordered_map>


std::pair<Edge, RegGraph::NodePtr> &RegGraph::get_first_edge() {
  return head->edges[0];
}

bool RegGraph::is_simple_graph() {
  return head->edges.size() == 1 && get_first_edge().second == tail;
}

bool RegGraph::is_simple_empty_graph() {
  return is_simple_graph() && get_first_edge().first.is_empty();
}

bool RegGraph::is_simple_concatenation_graph() {
  return is_simple_graph() && get_first_edge().first.is_concatenation();
}

bool RegGraph::is_simple_character_set_graph() {
  return is_simple_graph() && get_first_edge().first.is_character_set();
}

void RegGraph::merge_head(NodePtr node, RegGraph &other) {
    node->merge_node(*other.head);
    other.head.delete_node();
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
  // loop is introduced, new tail must be created to avoid others
  // using the loop
  auto new_tail = create_node();
  if (range.lower_bound < 2 && range.upper_bound == 0) {
    tail->add_empty_edge(head);
  } else {
    tail->add_edge(Edge::repeat(range), head);
  }
  tail->add_empty_edge(new_tail);
  tail = new_tail;

  if (range.lower_bound == 0) {
    head->add_empty_edge(tail);
  }
}

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

void RegGraph::character_set_complement() {
  if (!is_simple_character_set_graph()) { exit(1); }
  get_first_edge().first.set.complement();
}

void RegGraph::match_begin_unknown() {
  auto node = create_node();

  node->add_edge(Edge::character_set(CHARACTER_SET_ALL), head);
  head->add_edge(Edge::repeat(RepeatRange{0, 0}), node);

  head = node;
}

void RegGraph::match_tail_unknown() {
  auto node = create_node();

  tail->add_edge(Edge::character_set(CHARACTER_SET_ALL), node);
  node->add_edge(Edge::repeat(RepeatRange{0, 0}), tail);

  tail = node;
}

std::ostream &operator<<(std::ostream &stream, RegGraph &other) {
  std::unordered_map<RegGraph::NodePtr, size_t> node_map{};

  for (auto ptr = other.nodes.begin(); ptr != other.nodes.end(); ++ptr) {
    node_map.emplace(ptr, node_map.size());
  }

  for (auto ptr = other.nodes.begin(); ptr != other.nodes.end(); ++ptr) {
    stream << "NODE: " << node_map[ptr] << '\n';
    for (auto &[edge, dest] : ptr->edges) {
      stream << '\t' << node_map[dest] << '\t' << edge << '\n';
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
    case EdgeType::REPEAT:
      return stream << "REPEAT: " << other.range;
    default:
      return stream << "UNKNOWN";
  }
}
