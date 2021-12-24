
#include <list>
#include <memory>
#include "tokenizer.hpp"

struct Edge;
struct Node {
  std::vector<Edge> edges;
  Node(): edges{} {}
};

struct RepeatRange {
  size_t lower_bound; // >=1
  size_t upper_bound; // if 0 means no upperbound
};

enum class EdgeType {
  NORMAL,
  EMPTY,
  LOOP,
};

struct Edge {
  std::string value;
  EdgeType type;
  Node *next;
  RepeatRange range;

  Edge():type(EdgeType::EMPTY), next{} {}
  Edge(std::string value, EdgeType type):value(value), type(type), next{} {}
  Edge(std::string value, EdgeType type, Node *next):value(value), type(type), next(next) {}

};

class RegGraph {
  Node *head;
  Node *tail;
  std::list<std::unique_ptr<Node>> nodes;

};


class Parser {
    
public:
  RegexTokenizer &m_tokenizer;
  

  void build_graph();


  Parser(RegexTokenizer &tokenizer):m_tokenizer(tokenizer) {};

};