#include "parser.hpp"
#include "tokenizer.hpp"
#include "reg_graph.hpp"


std::optional<std::string> Parser::build_graph() {
  // add virtual '(' layer
  graph_stack.emplace_back(
      TokenType::LEFT_PARENTHESES, std::vector<RegGraph>{}
  );

  bool match_begin = false;
  bool match_end = false;

  while (auto token = tokenizer.next()) {
    regex_assert(!graph_stack.empty());
    auto &[top_sym, top_vec] = graph_stack.back();

    switch (token->type) {
      case TokenType::ATOM: {
        Edge edge{};

        if (
            top_sym == TokenType::LEFT_BRACKETS ||
            top_sym == TokenType::LEFT_BRACKETS_NOT
        ) {
          edge = Edge::character_set(token->string);
        } else {
          edge = Edge::concanetation(token->string);
        }

        top_vec.emplace_back(RegGraph::single_edge(std::move(edge)));

        break;
      }
      case TokenType::VERTICAL_BAR:
      case TokenType::LEFT_PARENTHESES:
      case TokenType::LEFT_BRACKETS_NOT:
      case TokenType::LEFT_BRACKETS:
        graph_stack.emplace_back(token->type, std::vector<RegGraph>{});
        break;
      case TokenType::RIGHT_BRACKETS: {
        regex_assert(
            top_sym == TokenType::LEFT_BRACKETS ||
            top_sym == TokenType::LEFT_BRACKETS_NOT
        );

        auto con_graph = RegGraph::join_character_set_graph(
            top_vec.begin(), top_vec.end()
        );
        graph_stack.pop_back();

        if (top_sym == TokenType::LEFT_BRACKETS_NOT) {
          con_graph.character_set_complement();
        }

        regex_assert(!graph_stack.empty());
        graph_stack.back().second.emplace_back(std::move(con_graph));

        break;
      }
      case TokenType::RIGHT_PARENTHESES: {
        auto graph = pop_and_join();
        graph_stack.back().second.emplace_back(std::move(graph));

        break;
      }
      /* Matches the preceding element
         {m}    exactly m times
         {m,}   at least m times
         {,n}   no more than n times
         {m,n}  at least m times, and no more than n times */
      case TokenType::LEFT_BRACES: {
        if (top_vec.empty()) { return "invalid suffix operator"; }

        std::vector<size_t> range_buf{};

        while (true) {
          token = tokenizer.next();
          regex_assert(token.has_value());

          switch (token->type) {
            case TokenType::COMMA:
              if (range_buf.size() == 0) {
                range_buf.emplace_back(0);
                range_buf.emplace_back(0);
              } else if (range_buf.size() == 1) {
                range_buf.emplace_back(0);
              } else {
                return "invalid braces format";
              }
              break;
            case TokenType::NUMERIC:
              if (range_buf.size() != 0 && range_buf.size() != 2) {
                return "invalid braces format";
              }
              range_buf.emplace_back(token->value);
              break;
            case TokenType::RIGHT_BRACES:
              goto end_braces;
            case TokenType::ERROR:
              return token->string;
            default:
              regex_abort("unexpected token");
          }
        }
end_braces:
        RepeatRange range{};

        if (range_buf.size() == 1) {
          range = RepeatRange{range_buf[0], range_buf[0] + 1};
        } else if (range_buf.size() == 2) {
          range = RepeatRange{range_buf[0], 0};
        } else if (range_buf.size() == 3 && range_buf[0] <= range_buf[2]) {
          range = RepeatRange{range_buf[0], range_buf[2] + 1};
        } else {
          return "invalid braces format";
        }

        top_vec.back().repeat_graph(range);

        break;
      }
      case TokenType::MATCH_BEGIN:
        match_begin = true;
        break;
      case TokenType::MATCH_END:
        match_end = true;
        break;
      case TokenType::ASTERISK:
        if (top_vec.empty()) { return "invalid suffix operator"; }
        top_vec.back().repeat_graph(RepeatRange{0, 0});
        break;
      case TokenType::PLUS_SIGN:
        if (top_vec.empty()) { return "invalid suffix operator"; }
        top_vec.back().repeat_graph(RepeatRange{1, 0});
        break;
      case TokenType::QUESTION_MARK:
        if (top_vec.empty()) { return "invalid suffix operator"; }
        top_vec.back().repeat_graph(RepeatRange{0, 2});
        break;
      case TokenType::CHARACTER_RANGE:
        top_vec.emplace_back(RegGraph::single_edge(
            Edge::character_set(token->range)
        ));
        break;
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
        top_vec.emplace_back(RegGraph::single_edge(
            Edge::character_set(token->type)
        ));
        break;
      case TokenType::ERROR:
        return token->string;
      default:
        regex_abort("unexpected token");
    }
  }

  regex_graph = pop_and_join();

  regex_graph.head->marker = NodeMarker::MATCH_BEGIN;
  regex_graph.tail->marker = NodeMarker::MATCH_END;

  if (!match_begin) { regex_graph.match_begin_unknown(); }
  if (!match_end) { regex_graph.match_tail_unknown(); }

  if (debug) {
    std::cout << "---------- [  PARSER  ] ----------" << std::endl;
    std::cout << regex_graph;
  }

  return std::nullopt;
}

RegGraph Parser::pop_and_join() {
  regex_assert(!graph_stack.empty());
  auto &[_, top_vec] = graph_stack.back();
  auto con_graph = RegGraph::concatenate_graph(top_vec.begin(), top_vec.end());

  while (graph_stack.back().first != TokenType::LEFT_PARENTHESES) {
    graph_stack.pop_back();

    regex_assert(!graph_stack.empty());
    auto &top_vec = graph_stack.back().second;

    auto graph = RegGraph::concatenate_graph(top_vec.begin(), top_vec.end());
    con_graph = RegGraph::join_graph(std::move(con_graph), std::move(graph));
  }

  graph_stack.pop_back();

  return con_graph;
}
