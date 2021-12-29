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
  regex_assert(graph.is_simple_character_set_graph());

  get_first_edge().first.set |= graph.get_first_edge().first.set;
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
  // loop needs to be created
  auto new_head = create_node();
  auto new_tail = create_node();
  if (range.lower_bound < 2 && range.upper_bound == 0) {
    // unbounded loop, empty edge could do it
    tail->add_empty_edge(head);
    new_head->add_empty_edge(head);
    tail->add_empty_edge(new_tail);
  } else {
    // bounded loop, we need a stack to track loop count

    // todo: optimize for simple cases

    tail->add_edge(Edge::repeat(range), head);
    new_head->add_edge(Edge::enter_loop(), head);
    tail->add_edge(Edge::exit_loop(range), new_tail);
  }
  head = new_head;
  tail = new_tail;
  // allow skipping the whole loop
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
  auto node0 = create_node();
  auto node1 = create_node();
  auto node2 = create_node();

  node0->add_empty_edge(node1);
  node1->add_edge(Edge::character_set(CHARACTER_SET_ALL), node2);
  node2->add_empty_edge(head);

  node2->add_empty_edge(node1);
  node0->add_empty_edge(head);

  head = node0;
}

void RegGraph::match_tail_unknown() {
  auto node0 = create_node();
  auto node1 = create_node();
  auto node2 = create_node();

  tail->add_empty_edge(node0);
  node0->add_edge(Edge::character_set(CHARACTER_SET_ALL), node1);
  node1->add_empty_edge(node2);

  node1->add_empty_edge(node0);
  tail->add_empty_edge(node2);

  tail = node2;
}

std::ostream &operator<<(std::ostream &stream, RegGraph &other) {
  std::unordered_map<RegGraph::NodePtr, size_t> node_map{};

  for (auto ptr = other.nodes.begin(); ptr != other.nodes.end(); ++ptr) {
    node_map.emplace(ptr, node_map.size());
  }

  stream << "[GRAPH] size: " << node_map.size() << '\n';

  for (auto ptr = other.nodes.begin(); ptr != other.nodes.end(); ++ptr) {
    if (ptr == other.head) {
      stream << "NODE: head";
    } else if (ptr == other.tail) {
      stream << "NODE: tail";
    } else {
      stream << "NODE: " << node_map[ptr];
    }
    switch (ptr->marker) {
      case NodeMarker::MATCH_BEGIN:
        stream << ", MATCH_BEGIN";
        break;
      case NodeMarker::MATCH_END:
        stream << ", MATCH_END";
        break;
      default:
        break;
    }
    stream << '\n';

    for (auto &[edge, dest] : ptr->edges) {
      if (dest == other.head) {
        stream << "    |=> head,\t" << edge << '\n';
      } else if (dest == other.tail) {
        stream << "    |=> tail,\t" << edge << '\n';
      } else {
        stream << "    |=> " << node_map[dest] << ",\t" << edge << '\n';
      }
    }
  }

  return stream;
}

std::ostream &operator<<(std::ostream &stream, const Edge &other) {
  switch (other.type) {
    case EdgeType::EMPTY:
      return stream << "EMPTY";
    case EdgeType::CONCATENATION:
      return stream << "CONCATENATION: " << make_escape(other.string);
    case EdgeType::CHARACTER_SET:
      return stream << "CHARACTER_SET: " << other.set;
    case EdgeType::REPEAT:
      return stream << "REPEAT: " << other.range;
    case EdgeType::ENTER_LOOP:
      return stream << "ENTER_LOOP";
    case EdgeType::EXIT_LOOP:
      return stream << "EXIT_LOOP: " << other.range;
    default:
      return stream << "UNKNOWN";
  }
}
