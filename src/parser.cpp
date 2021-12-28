#include "parser.hpp"
#include "tokenizer.hpp"
#include "reg_graph.hpp"


std::optional<std::string> Parser::build_graph() {

  GraphStack graph_stack;
  graph_stack.emplace_back(TokenType::LEFT_PARENTHESES, std::vector<RegGraph>{});  // add virtual '(' layer, thus it will never be empty
  bool match_begin = false;
  bool match_end = false;
  // bool right_of_bar = false;

  while (auto token = tokenizer.next()) {
    switch (token->type) {
      case TokenType::ATOM:
      {
        RegGraph graph{};

        if (graph_stack.back().first == TokenType::VERTICAL_BAR){
          graph_stack.emplace_back(token->type, std::vector<RegGraph>{});
          graph = RegGraph::single_edge(Edge::concanetation(token->string));
          graph_stack.back().second.emplace_back(std::move(graph));
          break;
        }

        if (graph_stack.back().first == TokenType::LEFT_BRACKETS ||
            graph_stack.back().first == TokenType::LEFT_BRACKETS_NOT)
        {
          graph = RegGraph::single_edge(Edge::character_set(token->string));
        } else {
          graph = RegGraph::single_edge(Edge::concanetation(token->string));
        }

        graph_stack.back().second.emplace_back(std::move(graph));

        break;
      }

      case TokenType::VERTICAL_BAR:
      {
        // if its left side has no expression, just push '|' layer on stack
        if (graph_stack.back().second.empty()) {
          graph_stack.emplace_back(token->type, std::vector<RegGraph>{});
          break;
        } else {
          // concatentate all the graphs on its left and move it to '|' layer
          auto &&top_vector = graph_stack.back().second;
          auto left_graph = RegGraph::concatenat_graph(top_vector.begin(), top_vector.end());
          top_vector.clear();
          graph_stack.emplace_back(token->type, std::vector<RegGraph>{});
          graph_stack.back().second.emplace_back(std::move(left_graph));
          break;
        }
      }

      case TokenType::LEFT_PARENTHESES:
      case TokenType::LEFT_BRACKETS_NOT:
      case TokenType::LEFT_BRACKETS:
      {
        graph_stack.emplace_back(token->type, std::vector<RegGraph>{});
        break;
      }

      case TokenType::RIGHT_BRACKETS:
      {
        if (graph_stack.back().first == TokenType::LEFT_BRACKETS_NOT) {
          for (auto &g : graph_stack.back().second) {
            g.character_set_complement();
          }
        }

        if (graph_stack.back().first == TokenType::LEFT_BRACKETS ||
            graph_stack.back().first == TokenType::LEFT_BRACKETS_NOT ) {
          auto &&top_vector = graph_stack.back().second;
          auto con_graph = RegGraph::join_character_set_graph(top_vector.begin(), top_vector.end());
          graph_stack.pop_back();
          if (graph_stack.back().first != TokenType::VERTICAL_BAR) {
            graph_stack.back().second.emplace_back(std::move(con_graph));
          } else {
            graph_stack.emplace_back(token->type, std::vector<RegGraph>{});
            graph_stack.back().second.emplace_back(std::move(con_graph));
          }
          break;
        }

        return "";
      }

      case TokenType::RIGHT_PARENTHESES:
      {
        // std::cout<<"stakc size: "<<graph_stack.size()<<std::endl;
        RegGraph union_graph{};
        while (graph_stack.back().first != TokenType::LEFT_PARENTHESES) {
          if (!graph_stack.back().second.empty()) {
            union_graph = RegGraph::join_graph(std::move(union_graph), std::move(graph_stack.back().second.back()));
          }
          graph_stack.pop_back();
        }
        graph_stack.back().second.emplace_back(std::move(union_graph));
        // no vertical bar layer in '()'
        if (graph_stack.back().first == TokenType::LEFT_PARENTHESES) {
          // std::cout<<"in"<<std::endl;
          auto &&top_vector = graph_stack.back().second;
          auto con_graph = RegGraph::concatenat_graph(top_vector.begin(), top_vector.end());
          graph_stack.pop_back();
          graph_stack.back().second.emplace_back(std::move(con_graph));
          break;
        }

        return "";
      }

      case TokenType::RIGHT_BRACES:
      {
        return "";
      }

      /* Matches the preceding element
         {m}   exactly m times
         {m,}  at least m times
         {m,n} at least m times, and no more than n times */
      case TokenType::LEFT_BRACES:
      {
        // if it has preceding graph
        if (!graph_stack.back().second.empty()) {
          std::vector<size_t> range_buf{};
          while (token->type != TokenType::RIGHT_BRACES) {
            if (token = tokenizer.next()) {
              if (token->type == TokenType::COMMA) {
                range_buf.emplace_back(0);
              } else if (token->type == TokenType::NUMERIC) {
                range_buf.emplace_back(token->value);
              }
            } else {
              return "";
            }
          }
          RepeatRange range;
          if (range_buf.size() == 1)
            range = RepeatRange{range_buf[0], range_buf[0] + 1};
          else if (range_buf.size() == 2)
            range = RepeatRange{range_buf[0], 0};
          else if (range_buf.size() == 3 && range_buf[0] <= range_buf[2])
            range = RepeatRange{range_buf[0], range_buf[2] + 1};
          else
            return "";

          graph_stack.back().second.back().repeat_graph(range);
          break;
        }
        return "";
      }

      case TokenType::MATCH_BEGIN:
      {
        match_begin = true;
        break;
      }

      case TokenType::MATCH_END:
      {
        match_end = true;
        break;
      }

      case TokenType::ASTERISK:
      {
        if (!graph_stack.back().second.empty()) {
          graph_stack.back().second.back().repeat_graph(RepeatRange{0, 0});
          break;
        }
        return "";
      }

      case TokenType::PLUS_SIGN:
      {
        if (!graph_stack.back().second.empty()) {
          graph_stack.back().second.back().repeat_graph(RepeatRange{1, 0});
          break;
        }
        return "";
      }

      case TokenType::QUESTION_MARK:
      {
        if (!graph_stack.back().second.empty()) {
          graph_stack.back().second.back().repeat_graph(RepeatRange{0, 2});
          break;
        }
        return "";
      }

      case TokenType::CHARACTER_RANGE:
      {
        RegGraph graph = RegGraph::single_edge(Edge::character_set(token->range));
        graph_stack.back().second.emplace_back(std::move(graph));
        break;
      }

      // character class
      case TokenType::CHARACTER_CLASS_UPPER:
      case TokenType::CHARACTER_CLASS_LOWER:
      case TokenType::CHARACTER_CLASS_ALPHA:
      case TokenType::CHARACTER_CLASS_DIGIT:
      case TokenType::CHARACTER_CLASS_XDIGIT:
      case TokenType::CHARACTER_CLASS_ALNUM:
      case TokenType::CHARACTER_CLASS_PUNCT:
      case TokenType::CHARACTER_CLASS_BLANK:
      case TokenType::CHARACTER_CLASS_SPACE:
      case TokenType::CHARACTER_CLASS_CNTRL:
      case TokenType::CHARACTER_CLASS_GRAPH:
      case TokenType::CHARACTER_CLASS_PRINT:
      case TokenType::CHARACTER_CLASS_WORD:
      case TokenType::PERIOD:
      {
        RegGraph graph = RegGraph::single_edge(Edge::character_set(token->type));
        graph_stack.back().second.emplace_back(std::move(graph));
        break;
      }

      case TokenType::ERROR:
        return token->string;

      default:
        return "unexpected token";
    }
  }

  /* After parsing, the graph_stack should only have at most 2 graph vector,
     pop it and concatenat it to one graph representing the expression */

  if (graph_stack.back().second.empty()) {
    return "No expressions";
  }

  if (graph_stack.back().first != TokenType::LEFT_PARENTHESES) {
    RegGraph union_graph{};
    while (graph_stack.back().first != TokenType::LEFT_PARENTHESES) {
      if (!graph_stack.back().second.empty()) {
        union_graph = RegGraph::join_graph(std::move(union_graph), std::move(graph_stack.back().second.back()));
      }
      graph_stack.pop_back();
    }
    graph_stack.back().second.emplace_back(std::move(union_graph));
  }

  // no vertical bar layer in '()'
  auto &&top_vector = graph_stack.back().second;
  auto con_graph = RegGraph::concatenat_graph(top_vector.begin(), top_vector.end());
  graph_stack.back().second.emplace_back(std::move(con_graph));

  regex_graph = std::move(graph_stack.back().second.back());

  regex_graph.head->marker = NodeMarker::MATCH_BEGIN;
  regex_graph.tail->marker = NodeMarker::MATCH_END;

  if (!match_begin) { regex_graph.match_begin_unknown(); }
  if (!match_end) { regex_graph.match_tail_unknown(); }

  graph_stack.pop_back();

  if (!graph_stack.empty()) return "";

  if (debug) {
    std::cout << "---------- [  PARSER  ] ----------" << std::endl;
    std::cout << regex_graph;
  }

  return std::nullopt;
}
