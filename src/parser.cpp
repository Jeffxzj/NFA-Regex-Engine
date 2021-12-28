#include "parser.hpp"
#include "tokenizer.hpp"
#include "reg_graph.hpp"


void print_stack(std::vector<std::vector<RegexToken>> &stack) {

  for (auto buf:stack){
    for (auto token:buf) {
      std::cout<<token;
    }
    std::cout<<std::endl;
  }

}

std::optional<std::string> Parser::build_graph() {

  std::vector<std::pair<TokenType, std::vector<RegGraph>>> graph_stack;
  bool match_begin = false;
  bool match_end = false;

  while (auto token = tokenizer.next()) {
    std::cout << token.value() << std::endl;

    switch (token->type) {

      case TokenType::ATOM:
      {
        RegGraph graph{};
        if (!graph_stack.empty() && (graph_stack.back().first == TokenType::LEFT_BRACKETS ||
            graph_stack.back().first == TokenType::LEFT_BRACKETS_NOT))
        {
          graph = RegGraph::single_edge(Edge::character_set(token->string));
        } else {
          graph = RegGraph::single_edge(Edge::concanetation(token->string));
        }

        if (graph_stack.empty()) {
          std::vector<RegGraph> vec;
          vec.emplace_back(std::move(graph));
          graph_stack.emplace_back(std::make_pair(token->type, std::move(vec)));
        } else if (graph_stack.back().first == TokenType::VERTICAL_BAR) {
          auto &left_graph = graph_stack.back().second.back();
          RegGraph union_graph = RegGraph::join_graph(std::move(left_graph), std::move(graph));
          graph_stack.back().second.pop_back();
          graph_stack.back().second.emplace_back(std::move(union_graph));
        } else {
          graph_stack.back().second.emplace_back(std::move(graph));
        }

        break;
      }

      case TokenType::VERTICAL_BAR:
      {
        // move the last graph of cur stack top to the new stack top vector
        auto &&left_graph = graph_stack.back().second.back();
        graph_stack.back().second.pop_back();
        graph_stack.emplace_back(token->type, std::vector<RegGraph>{});
        graph_stack.back().second.emplace_back(std::move(left_graph));
        break;
      }

      case TokenType::LEFT_PARENTHESES:
      case TokenType::LEFT_BRACKETS_NOT:
      case TokenType::LEFT_BRACKETS:
      {
        // add an empty layer to stack
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
          if (graph_stack.size() == 1) {
            graph_stack.back().second.clear();
            // std::vector<RegGraph> vec;
            // vec.emplace_back(std::move(con_graph));
            // graph_stack.emplace_back(std::make_pair(token->type, std::move(vec)));
          } else {
            graph_stack.pop_back();
          }
          graph_stack.back().second.emplace_back(std::move(con_graph));
          break;
        }

        return "";
      }

      case TokenType::RIGHT_PARENTHESES:
      {
        if (graph_stack.back().first == TokenType::LEFT_PARENTHESES) {
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
        if (!graph_stack.empty()) {
          std::vector<size_t> range_buf{};
          while (token->type != TokenType::RIGHT_BRACES) {
            token = tokenizer.next();
            if (token->type == TokenType::COMMA) {
              range_buf.emplace_back(0);
            } else if (token->type == TokenType::NUMERIC) {
              range_buf.emplace_back(token->value);
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

      // Matches the preceding element zero or more times
      case TokenType::ASTERISK:
      {
        if (!graph_stack.empty()) {
          graph_stack.back().second.back().repeat_graph(RepeatRange{0, 0});
          break;
        }
        return "";
      }

      case TokenType::PLUS_SIGN:
      {
        if (!graph_stack.empty()) {
          graph_stack.back().second.back().repeat_graph(RepeatRange{1, 0});
          break;
        }
        return "";
      }
      case TokenType::QUESTION_MARK:
      {
        if (!graph_stack.empty()) {
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

  /* After parsing, the graph_stack should only have one graph vector,
     pop it and concatenat it to one graph representing the expression */

  if (graph_stack.empty()) {
    return "No expressions";
  }

  auto &&graph_vec = graph_stack.back().second;
  regex_graph = RegGraph::concatenat_graph(graph_vec.begin(), graph_vec.end());

  regex_graph.head->marker = NodeMarker::MATCH_BEGIN;
  regex_graph.tail->marker = NodeMarker::MATCH_END;

  if (!match_begin) { regex_graph.match_begin_unknown(); }
  if (!match_end) { regex_graph.match_tail_unknown(); }

  graph_stack.pop_back();

  if (!graph_stack.empty()) return "";

  return std::nullopt;
}
