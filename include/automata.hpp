#ifndef REGEX_AUTOMATA
#define REGEX_AUTOMATA


#include <cstdint>
#include <new>
#include <vector>
#include <string>

#include "utility.hpp"


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

  void give_up_nodes(RegGraph &other) {
    nodes.give_up_nodes(other.nodes);
  }

  void join_graph_continue(RegGraph &&graph);

  void concatenat_graph_continue(RegGraph &&graph);

  RegGraph() : nodes{}, head{create_node()}, tail{create_node()} {}

public:
  List<Node> nodes;
  NodePtr head;
  NodePtr tail;

  static RegGraph single_edge(Edge &&edge);

  static RegGraph repeat_graph(RegGraph &&graph, RepeatRange range);

  template<class GraphPtr>
  static RegGraph join_graph(GraphPtr begin, GraphPtr end);

  template<class GraphPtr>
  static RegGraph concatenat_graph(GraphPtr begin, GraphPtr end);
};

class Node {
public:
  std::vector<std::pair<Edge, RegGraph::NodePtr>> edges;

  Node() = default;

  void add_edge(Edge &&edge, RegGraph::NodePtr next);

  void add_empty_edge(RegGraph::NodePtr next);
};

enum class EdgeType {
  EMPTY,
  CONCATENATION,
  CHARACTER_UNION,
  REPERAT,
  CHARACTER_RANGE,
  // character class
  CHARACTER_CLASS_UPPER,
  CHARACTER_CLASS_LOWER,
  CHARACTER_CLASS_ALPHA,
  CHARACTER_CLASS_DIGIT,
  CHARACTER_CLASS_XDIGIT,
  CHARACTER_CLASS_ALNUM,
  CHARACTER_CLASS_PUNCT,
  CHARACTER_CLASS_BLANK,
  CHARACTER_CLASS_SPACE,
  CHARACTER_CLASS_CNTRL,
  CHARACTER_CLASS_GRAPH,
  CHARACTER_CLASS_PRINT,
  CHARACTER_CLASS_WORD,
};

class Edge {
private:
  Edge(EdgeType type, std::string value) : type{type}, string{value} {}

  Edge(EdgeType type, RepeatRange range) : type{type}, repeat_range{range} {}

  Edge(EdgeType type, CharacterRange range) :
      type{type}, character_range{range} {}

  void drop() {
    switch(type) {
      case EdgeType::CONCATENATION:
      case EdgeType::CHARACTER_UNION:
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
      case EdgeType::CHARACTER_UNION:
        new(&string) std::string{other.string};
        break;
      case EdgeType::REPERAT:
        repeat_range = other.repeat_range;
        break;
      case EdgeType::CHARACTER_RANGE:
        character_range = other.character_range;
        break;
      default:
        break;
    }
  }

  void emplace(Edge &&other) {
    type = other.type;

    switch(type) {
      case EdgeType::CONCATENATION:
      case EdgeType::CHARACTER_UNION:
        new(&string) std::string{std::move(other.string)};
        break;
      case EdgeType::REPERAT:
        repeat_range = other.repeat_range;
        break;
      case EdgeType::CHARACTER_RANGE:
        character_range = other.character_range;
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
    RepeatRange repeat_range;
    CharacterRange character_range;
  };

  Edge(EdgeType type) : type{type}, null{} {}

  static Edge empty() { return Edge{EdgeType::EMPTY}; }

  static Edge concanetation(std::string value) {
    return Edge{EdgeType::CONCATENATION, value};
  }

  static Edge character_union(std::string value) {
    return Edge{EdgeType::CHARACTER_UNION, value};
  }

  static Edge repeat(RepeatRange range) {
    return Edge{EdgeType::REPERAT, range};
  }

  static Edge characters(CharacterRange range) {
    return Edge{EdgeType::CHARACTER_RANGE, range};
  }

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

  ~Edge() {}
};

template<class GraphPtr>
inline RegGraph RegGraph::join_graph(GraphPtr begin, GraphPtr end) {
  if (begin == end) { return single_edge(Edge::empty()); }

  RegGraph graph{};
  for (auto ptr = begin; ptr != end; ++ptr) {
    graph.join_graph_continue(std::move(*ptr));
  }
  return graph;
}

template<class GraphPtr>
inline RegGraph RegGraph::concatenat_graph(GraphPtr begin, GraphPtr end) {
  if (begin == end) { return single_edge(Edge::empty()); }

  RegGraph graph = std::move(*begin++);

  for (auto ptr = begin; ptr != end; ++ptr) {
    graph.join_graph_continue(std::move(*ptr));
  }

  return graph;
}

inline void Node::add_edge(Edge &&edge, RegGraph::NodePtr next) {
  edges.emplace_back(std::move(edge), next);
}

inline void Node::add_empty_edge(RegGraph::NodePtr next) {
  edges.emplace_back(Edge::empty(), next);
}

#endif // REGEX_AUTOMATA
