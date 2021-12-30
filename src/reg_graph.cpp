#include "reg_graph.hpp"

#include <iostream>
#include <unordered_map>
#include <unordered_set>


std::pair<Edge, RegGraph::NodePtr> &RegGraph::get_first_edge() {
  return head->edges[0];
}

bool RegGraph::is_simple_graph() {
  return
      size == 2 &&
      head->edges.size() == 1 &&
      get_first_edge().second == tail;
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
    other.size -= 1;
}

void RegGraph::concatenat_graph_continue(RegGraph &&graph) {
  if (graph.is_simple_empty_graph()) {
    return;
  } else if (is_simple_empty_graph()) {
    *this = std::move(graph);
  } else if (
      is_simple_concatenation_graph() &&
      graph.is_simple_concatenation_graph()
  ) {
    get_first_edge().first.string.append(
      graph.get_first_edge().first.string
    );
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
  if (is_simple_empty_graph()) { return; }

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

void RegGraph::garbage_collection(PassFn pass_fn) {
  std::unordered_set<NodePtr> new_nodes{};

  if ((this->*pass_fn)(new_nodes)) {
    List<Node> new_list{};

    for (auto &node : new_nodes) {
      nodes.give_up_node(node, new_list);
    }

    size = new_nodes.size();
    nodes = std::move(new_list);
  }
}

bool RegGraph::replace_empty_transition(NodeSet &new_nodes) {
  std::unordered_map<NodePtr, NodePtr> empty_transition{};

  for (auto ptr = nodes.begin(); ptr != nodes.end(); ++ptr) {
    if (ptr->marker == NodeMarker::ANONYMOUS && ptr->edges.size() == 1) {
      auto &[edge, dest] = ptr->edges[0];

      if (edge.is_empty()) {
        empty_transition.emplace(ptr, dest);
      }
    }
  }

  if (empty_transition.empty()) { return false; }

  auto ptr = empty_transition.find(head);
  if (ptr != empty_transition.end()) {
    head = ptr->second;
  }

  std::vector<std::pair<NodePtr, size_t>> stack{{head, 0}};
  new_nodes.emplace(head);

  while(!stack.empty()) {
    auto &[node, index] = stack.back();

    if (index == 0) {
      for (auto &[_, dest] : node->edges) {
        auto ptr = empty_transition.find(dest);
        if (ptr != empty_transition.end()) {
          dest = ptr->second;
        }
      }
    }

    if (node != tail && index < node->edges.size()) {
      auto &[_, dest] = node->edges[index++];

      if (!new_nodes.contains(dest)) {
        new_nodes.emplace(dest);
        stack.emplace_back(dest, 0);
      }
    } else {
      stack.pop_back();
    }
  }

  new_nodes.emplace(tail);

  return true;
}

bool RegGraph::fold_empty_edge(NodeSet &new_nodes) {
  std::vector<std::pair<NodePtr, size_t>> stack{{head, 0}};
  new_nodes.emplace(head);

  while(!stack.empty()) {
    auto [fold_node, fold_index] = stack.back();

    if (fold_index == 0) {
      // the node was first met, collect nodes reachable through empty edge
      std::unordered_set<NodePtr> reachable{};
      size_t start_depth = stack.size();

      stack.emplace_back(fold_node, 0);

      while (stack.size() > start_depth) {
        auto &[curr, index] = stack.back();

        if (
            curr != tail &&
            index < curr->edges.size()
        ) {
          auto &[edge, dest] = curr->edges[index++];

          if (!reachable.contains(dest) && edge.is_empty()) {
            reachable.emplace(dest);
            if (dest->marker == NodeMarker::ANONYMOUS) {
              stack.emplace_back(dest, 0);
            }
          }
        } else {
          stack.pop_back();
        }
      }
      // now fold every empty edge
      std::vector<std::pair<Edge, NodePtr>> new_edges{};

      for (auto &[edge, dest] : fold_node->edges) {
        if (!edge.is_empty()) {
          new_edges.emplace_back(edge, dest);
        }
      }

      fold_node->edges = std::move(new_edges);

      for (auto curr : reachable) {
        if (curr == fold_node) { continue; }
        if (curr == tail || curr->marker != NodeMarker::ANONYMOUS) {
          fold_node->add_empty_edge(curr);
        } else {
          for (auto &[edge, dest] : curr->edges) {
            if (!edge.is_empty()) {
              fold_node->add_edge(Edge{edge}, dest);
            }
          }
        }
      }

      fold_node->unique_edge();
    }

    auto &[node, index] = stack.back();

    if (node != tail && index < node->edges.size()) {
      auto &[_, dest] = node->edges[index++];

      if (!new_nodes.contains(dest)) {
        new_nodes.emplace(dest);
        stack.emplace_back(dest, 0);
      }
    } else {
      stack.pop_back();
    }
  }

  new_nodes.emplace(tail);

  return true;
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

  node->add_edge(Edge::character_set(CHARACTER_SET_ALL), node);
  node->add_empty_edge(head);

  head = node;
}

void RegGraph::match_tail_unknown() {
  auto node = create_node();

  tail->add_empty_edge(node);
  node->add_edge(Edge::character_set(CHARACTER_SET_ALL), node);

  tail = node;
}

void RegGraph::optimize_graph() {
  for (auto ptr = nodes.begin(); ptr != nodes.end(); ++ptr) {
    ptr->unique_edge();
  }

  garbage_collection(&RegGraph::replace_empty_transition);

  for (auto ptr = nodes.begin(); ptr != nodes.end(); ++ptr) {
    ptr->unique_edge();
  }

  garbage_collection(&RegGraph::fold_empty_edge);
}

std::ostream &operator<<(std::ostream &stream, RegGraph &other) {
  std::unordered_map<RegGraph::NodePtr, size_t> node_map{};

  for (auto ptr = other.nodes.begin(); ptr != other.nodes.end(); ++ptr) {
    node_map.emplace(ptr, node_map.size() + 1);
  }

  stream
      << "[GRAPH] size: " << other.size << ' ' << node_map.size()
      << ", head: " << node_map[other.head]
      << ", tail: " << node_map[other.tail]
      << '\n';

  for (auto ptr = other.nodes.begin(); ptr != other.nodes.end(); ++ptr) {
    stream << "NODE: " << node_map[ptr];
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
      stream << "    |=> " << node_map[dest] << ",\t" << edge << '\n';
    }
  }

  regex_assert(other.size == node_map.size());

  return stream;
}

static bool compare_edge(
    std::pair<Edge, RegGraph::NodePtr> a,
    std::pair<Edge, RegGraph::NodePtr> b
) {
    if (a.first == b.first) {
      return a.first < b.first;
    } else {
      return &*a.second < &*b.second;
    }
}

void Node::unique_edge() {
  std::sort(edges.begin(), edges.end(), compare_edge);
  edges.erase(std::unique(edges.begin(), edges.end()), edges.end());
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
