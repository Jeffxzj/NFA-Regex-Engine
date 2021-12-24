#ifndef REGEX_AUTOMATA
#define REGEX_AUTOMATA


#include <cstdint>
#include <list>
#include <new>
#include <vector>
#include <string>

#include "utility.hpp"


class Edge;
class Node;

class Node {
public:
  std::vector<Edge> edges;

  Node() : edges{} {}
};


class RegGraph {
public:
  using List = std::list<Node>;
  using NodePtr = List::iterator;

private:
  NodePtr create_node() { return nodes.emplace({}); }

public:
  List nodes;
  NodePtr head;
  NodePtr tail;

  RegGraph() : nodes{}, head{create_node()}, tail{create_node()} {}
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
  Edge(EdgeType type, std::string value, RegGraph::NodePtr next) :
      type{type}, next{next}, string{value} {}

  Edge(EdgeType type, size_t lower, size_t upper, RegGraph::NodePtr next) :
      type{type}, next{next}, repeat_range{lower, upper} {}

  Edge(EdgeType type, char lower, char upper, RegGraph::NodePtr next) :
      type{type}, next{next}, character_range{lower, upper} {}

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
    next = other.next;

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
    next = other.next;

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
  RegGraph::NodePtr next;
  union {
    struct {} empty;
    std::string string;
    RepeatRange repeat_range;
    CharacterRange character_range;
  };

  Edge(EdgeType type, RegGraph::NodePtr next) :
      type{type}, next{next}, empty{} {}

  static Edge concanetation(std::string value, RegGraph::NodePtr next) {
    return Edge{EdgeType::CONCATENATION, value, next};
  }

  static Edge character_union(std::string value, RegGraph::NodePtr next) {
    return Edge{EdgeType::CHARACTER_UNION, value, next};
  }

  static Edge repeat(size_t lower, size_t upper, RegGraph::NodePtr next) {
    return Edge{EdgeType::REPERAT, lower, upper, next};
  }

  static Edge characters(char lower, char upper, RegGraph::NodePtr next) {
    return Edge{EdgeType::CHARACTER_RANGE, lower, upper, next};
  }

  friend std::ostream &
  operator<<(std::ostream &stream, const EdgeType &other);

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


#endif // REGEX_AUTOMATA
