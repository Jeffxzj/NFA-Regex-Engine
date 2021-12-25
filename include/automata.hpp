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

  void join_graph_continue(RegGraph &&graph);

  void concatenat_graph_continue(RegGraph &&graph);

  RegGraph() : nodes{}, head{create_node()}, tail{create_node()} {}

public:
  List<Node> nodes;
  NodePtr head;
  NodePtr tail;

  static RegGraph single_edge(Edge &&edge);

  static RegGraph complement(RegGraph &&graph);

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
  CHARACTER_SET,
  REPERAT,
};

class Edge {
private:
  Edge() : type{EdgeType::EMPTY}, null{} {}

  Edge(std::string value) : type{EdgeType::CONCATENATION}, string{value} {}

  Edge(CharacterSet set) : type{EdgeType::CHARACTER_SET}, character_set{set} {}

  Edge(RepeatRange range) : type{EdgeType::REPERAT}, repeat_range{range} {}

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
        repeat_range = other.repeat_range;
        break;
      case EdgeType::CHARACTER_SET:
        character_set = other.character_set;
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
        repeat_range = other.repeat_range;
        break;
      case EdgeType::CHARACTER_SET:
        character_set = other.character_set;
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
    CharacterSet character_set;
  };

  static Edge empty() { return Edge{}; }

  static Edge concanetation(std::string value) { return Edge{value}; }

  static Edge character_union(CharacterSet set) { return Edge{set}; }

  static Edge character_union(std::string value) {
    return character_union(CharacterSet{value});
  }

  static Edge character_union(CharacterRange range) {
    return character_union(CharacterSet{range});
  }

  static Edge character_union(TokenType type) {
    switch (type) {
      case TokenType::CHARACTER_CLASS_UPPER:
        return character_union(CHARACTER_SET_UPPER);
      case TokenType::CHARACTER_CLASS_LOWER:
        return character_union(CHARACTER_SET_LOWER);
      case TokenType::CHARACTER_CLASS_ALPHA:
        return character_union(CHARACTER_SET_ALPHA);
      case TokenType::CHARACTER_CLASS_DIGIT:
        return character_union(CHARACTER_SET_DIGIT);
      case TokenType::CHARACTER_CLASS_XDIGIT:
        return character_union(CHARACTER_SET_XDIGIT);
      case TokenType::CHARACTER_CLASS_ALNUM:
        return character_union(CHARACTER_SET_ALNUM);
      case TokenType::CHARACTER_CLASS_PUNCT:
        return character_union(CHARACTER_SET_PUNCT);
      case TokenType::CHARACTER_CLASS_BLANK:
        return character_union(CHARACTER_SET_BLANK);
      case TokenType::CHARACTER_CLASS_SPACE:
        return character_union(CHARACTER_SET_SPACE);
      case TokenType::CHARACTER_CLASS_CNTRL:
        return character_union(CHARACTER_SET_CONTRL);
      case TokenType::CHARACTER_CLASS_GRAPH:
        return character_union(CHARACTER_SET_GRAPH);
      case TokenType::CHARACTER_CLASS_PRINT:
        return character_union(CHARACTER_SET_PRINT);
      case TokenType::CHARACTER_CLASS_WORD:
        return character_union(CHARACTER_SET_WORD);
      case TokenType::PERIOD:
        return character_union(CHARACTER_SET_ALL);
      default:
        exit(1);
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
