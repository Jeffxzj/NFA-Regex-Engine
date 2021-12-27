#include "parser.hpp"
#include "tokenizer.hpp"


void print_stack(std::vector<std::vector<RegexToken>> &stack) {

  for (auto buf:stack){
    for (auto token:buf) {
      std::cout<<token;
    }
    std::cout<<std::endl;
  }

}

bool Parser::build_graph() {

  std::vector<std::pair<TokenType, std::vector<RegGraph>>> graph_stack;
  
  while (auto token = tokenizer.next()) {
    
    switch (token->type) {

      case TokenType::ATOM: 
      {
        RegGraph graph{};
        if (graph_stack.back().first == TokenType::LEFT_BRACKETS || 
            graph_stack.back().first == TokenType::LEFT_BRACKETS_NOT ) 
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

      case TokenType::CHARACTER_RANGE:
      {
        RegGraph graph = RegGraph::single_edge(Edge::character_set(token->range));
        
        if (graph_stack.empty()) {
          std::vector<RegGraph> vec;
          vec.emplace_back(std::move(graph));
          graph_stack.emplace_back(std::make_pair(token->type, std::move(vec)));
        } else if (graph_stack.back().first == TokenType::VERTICAL_BAR) {
          auto &&left_graph = graph_stack.back().second.back();
          RegGraph union_graph = RegGraph::join_graph(std::move(left_graph), std::move(graph));
          graph_stack.back().second.pop_back();
          graph_stack.back().second.emplace_back(std::move(union_graph));
        } else {
          graph_stack.back().second.emplace_back(std::move(graph));
        }
        
        break;
      
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
          break;
        }
        break;
      }

      case TokenType::RIGHT_PARENTHESES:
      {
        if (graph_stack.back().first == TokenType::LEFT_PARENTHESES) {
          auto &top_vector = graph_stack.back().second;
          auto con_graph = RegGraph::concatenat_graph(top_vector.begin(), top_vector.end());
          graph_stack.pop_back();
          graph_stack.back().second.emplace_back(std::move(con_graph));
          break;
        }
          
        return false;
      }

      case TokenType::RIGHT_BRACES:
      {
        return false;
      } 
      
      /* Matches the preceding element
         {m}   exactly m times
         {m,}  at least m times
         {m,n} at least m times, and no more than n times */
      case TokenType::LEFT_BRACES:
      {
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
          return false;

        if (!graph_stack.empty()) {
          graph_stack.back().second.back().repeat_graph(range);
          break;
        }
        
        return false;
      }

      case TokenType::MATCH_BEGIN:
      {
        break;
      } 
      case TokenType::MATCH_END:
      {
        break;
      }
      
      // Matches the preceding element zero or more times
      case TokenType::ASTERISK:
      {
        if (!graph_stack.empty()) {
          graph_stack.back().second.back().repeat_graph(RepeatRange{0, 0});
          break;
        }
        return false;
      }

      case TokenType::PLUS_SIGN:
      {
        if (!graph_stack.empty()) {
          graph_stack.back().second.back().repeat_graph(RepeatRange{1, 0});
          break;
        }
        return false;       
      }
      case TokenType::QUESTION_MARK:
      {
        if (!graph_stack.empty()) {
          graph_stack.back().second.back().repeat_graph(RepeatRange{0, 2});
          break;
        }
        return false;          
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
        Edge edge = Edge::character_set(token->type);
        RegGraph graph = RegGraph::single_edge(std::move(edge));
        graph_stack.back().second.emplace_back(std::move(graph));
        break;
      }

      default:
        break;
    }
    // std::cout << token.value() << std::endl;
  }

  // pop all the graphs and concatenate them
  // RegGraph final_graph;
  // while (!graph_stack.empty()) {
    
  // }
  return true;
}
