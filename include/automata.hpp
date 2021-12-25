#ifndef REGEX_AUTOMATA
#define REGEX_AUTOMATA


#include <cstdint>
#include <new>
#include <vector>
#include <string>
#include <iostream>

#include "utility.hpp"
#include "character_set.hpp"
#include "list.hpp"
#include "tokenizer.hpp"


class Edge;

class Node;

class RegGraph {
public:
  using NodePtr = List<Node>::Iter;

private:
  NodePtr create_node() { return nodes.emplace_back(); }

  NodePtr give_up_node(NodePtr node, RegGraph &other) {
    return nodes.give_up_node(node, other.nodes);
  }

  void give_up_nodes(RegGraph &other) { nodes.give_up_nodes(other.nodes); }

  void merge_head(NodePtr node, RegGraph &other);

  std::pair<Edge, RegGraph::NodePtr> &get_first_edge();

  bool is_simple_graph();

  bool is_simple_empty_graph();

  bool is_simple_concatenation_graph();

  bool is_simple_character_set_graph();

  void concatenat_graph_continue(RegGraph &&graph);

  void join_character_set_graph_continue(RegGraph &&graph);

  RegGraph() : nodes{}, head{create_node()}, tail{create_node()} {}

public:
  List<Node> nodes;
  NodePtr head;
  NodePtr tail;

  static RegGraph single_edge(Edge &&edge);

  static RegGraph join_graph(RegGraph &&graph1, RegGraph &&graph2);

  template<class GraphPtr>
  static RegGraph concatenat_graph(GraphPtr begin, GraphPtr end);

  template<class GraphPtr>
  static RegGraph join_character_set_graph(GraphPtr begin, GraphPtr end);

  void repeat_graph(RepeatRange range);

  void character_set_complement();

  friend std::ostream &operator<<(std::ostream &stream, RegGraph &other);
};

class Node {
public:
  std::vector<std::pair<Edge, RegGraph::NodePtr>> edges;

  Node() = default;

  void add_edge(Edge &&edge, RegGraph::NodePtr next);

  void add_empty_edge(RegGraph::NodePtr next);

  void merge_node(Node &other) {
    std::move(
      std::begin(other.edges),
      std::end(other.edges),
      std::back_inserter(edges)
    );
    other.edges.clear();
  }
};

enum class EdgeType {
  EMPTY,
  CONCATENATION,
  CHARACTER_SET,
  REPERAT,
};

class Edge {
private:
  Edge() : type{EdgeType::EMPTY}, null{} {}

  Edge(std::string value) : type{EdgeType::CONCATENATION}, string{value} {}

  Edge(CharacterSet set) : type{EdgeType::CHARACTER_SET}, set{set} {}

  Edge(RepeatRange range) : type{EdgeType::REPERAT}, range{range} {}

  void drop() {
    switch(type) {
      case EdgeType::CONCATENATION:
        string.~basic_string();
        break;
      default:
        break;
    }
  }

  void copy(const Edge &other) {
    type = other.type;

    switch(type) {
      case EdgeType::CONCATENATION:
        new(&string) std::string{other.string};
        break;
      case EdgeType::REPERAT:
        range = other.range;
        break;
      case EdgeType::CHARACTER_SET:
        set = other.set;
        break;
      default:
        break;
    }
  }

  void emplace(Edge &&other) {
    type = other.type;

    switch(type) {
      case EdgeType::CONCATENATION:
        new(&string) std::string{std::move(other.string)};
        break;
      case EdgeType::REPERAT:
        range = other.range;
        break;
      case EdgeType::CHARACTER_SET:
        set = other.set;
        break;
      default:
        break;
    }
  }

public:
  EdgeType type;
  union {
    struct {} null;
    std::string string;
    RepeatRange range;
    CharacterSet set;
  };

  static Edge empty() { return Edge{}; }

  static Edge concanetation(std::string value) { return Edge{value}; }

  static Edge character_set(CharacterSet set) { return Edge{set}; }

  static Edge character_set(std::string value) {
    return character_set(CharacterSet{value});
  }

  static Edge character_set(CharacterRange range) {
    return character_set(CharacterSet{range});
  }

  static Edge character_set(TokenType type) {
    switch (type) {
      case TokenType::CHARACTER_CLASS_UPPER:
        return character_set(CHARACTER_SET_UPPER);
      case TokenType::CHARACTER_CLASS_LOWER:
        return character_set(CHARACTER_SET_LOWER);
      case TokenType::CHARACTER_CLASS_ALPHA:
        return character_set(CHARACTER_SET_ALPHA);
      case TokenType::CHARACTER_CLASS_DIGIT:
        return character_set(CHARACTER_SET_DIGIT);
      case TokenType::CHARACTER_CLASS_XDIGIT:
        return character_set(CHARACTER_SET_XDIGIT);
      case TokenType::CHARACTER_CLASS_ALNUM:
        return character_set(CHARACTER_SET_ALNUM);
      case TokenType::CHARACTER_CLASS_PUNCT:
        return character_set(CHARACTER_SET_PUNCT);
      case TokenType::CHARACTER_CLASS_BLANK:
        return character_set(CHARACTER_SET_BLANK);
      case TokenType::CHARACTER_CLASS_SPACE:
        return character_set(CHARACTER_SET_SPACE);
      case TokenType::CHARACTER_CLASS_CNTRL:
        return character_set(CHARACTER_SET_CONTRL);
      case TokenType::CHARACTER_CLASS_GRAPH:
        return character_set(CHARACTER_SET_GRAPH);
      case TokenType::CHARACTER_CLASS_PRINT:
        return character_set(CHARACTER_SET_PRINT);
      case TokenType::CHARACTER_CLASS_WORD:
        return character_set(CHARACTER_SET_WORD);
      case TokenType::PERIOD:
        return character_set(CHARACTER_SET_ALL);
      default:
        exit(1);
    }
  }

  bool is_empty() {
    switch (type) {
      case EdgeType::EMPTY:
        return true;
      default:
        return false;
    }
  }

  bool is_concatenation() {
    switch (type) {
      case EdgeType::CONCATENATION:
        return true;
      default:
        return false;
    }
  }

  bool is_character_set() {
    switch (type) {
      case EdgeType::CHARACTER_SET:
        return true;
      default:
        return false;
    }
  }

  static Edge repeat(RepeatRange range) { return Edge{range}; }

  friend std::ostream &operator<<(std::ostream &stream, const Edge &other);

  Edge(const Edge &other) { copy(other); }

  Edge(Edge &&other) { emplace(std::move(other)); }

  Edge &operator=(const Edge &other) {
    drop();
    copy(other);
    return *this;
  }

  Edge &operator=(Edge &&other) {
    drop();
    emplace(std::move(other));
    return *this;
  }

  ~Edge() { drop(); }
};

inline void
RegGraph::merge_head(NodePtr node, RegGraph &other) {
    node->merge_node(*other.head);
    other.head.delete_node();
}

inline std::pair<Edge, RegGraph::NodePtr> &RegGraph::get_first_edge() {
  return head->edges[0];
}

inline bool RegGraph::is_simple_graph() {
  return head->edges.size() == 1 && get_first_edge().second == tail;
}

inline bool RegGraph::is_simple_empty_graph() {
  return is_simple_graph() && get_first_edge().first.is_empty();
}

inline bool RegGraph::is_simple_concatenation_graph() {
  return is_simple_graph() && get_first_edge().first.is_concatenation();
}

inline bool RegGraph::is_simple_character_set_graph() {
  return is_simple_graph() && get_first_edge().first.is_character_set();
}

template<class GraphPtr>
inline RegGraph RegGraph::concatenat_graph(GraphPtr begin, GraphPtr end) {
  RegGraph graph = single_edge(Edge::empty());

  for (auto ptr = begin; ptr != end; ++ptr) {
    graph.concatenat_graph_continue(std::move(*ptr));
  }

  return graph;
}

template<class GraphPtr>
inline RegGraph
RegGraph::join_character_set_graph(GraphPtr begin, GraphPtr end) {
  RegGraph graph = single_edge(Edge::character_set(CHARACTER_SET_EMPTY));

  for (auto ptr = begin; ptr != end; ++ptr) {
    graph.join_character_set_graph_continue(std::move(*ptr));
  }

  return graph;
}

inline void Node::add_empty_edge(RegGraph::NodePtr next) {
  edges.emplace_back(Edge::empty(), next);
}

#endif // REGEX_AUTOMATA
