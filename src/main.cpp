#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <filesystem>
#include <optional>

#include "utility.hpp"
#include "regex.hpp"


std::string escape_string(std::string string) {
  std::string result{};

  for (size_t i = 0; i < string.size(); ++i) {
    if (i + 1 < string.size() && string[i] == '\\' && string[i + 1] == 'n') {
      result.push_back('\n');
      ++i;
    } else {
      result.push_back(string[i]);
    }
  }

  return result;
}

void test_match(
    Regex &regex, std::string input,
    size_t expect_start, size_t expect_size
) {
  std::cout << "========== [ MATCHING ] ==========" << std::endl;
  std::cout << make_escape(input) << std::endl;
  std::cout << std::endl;

  if (auto match = regex.match(input)) {
    std::cout << "---------- [  RESULT  ] ----------" << std::endl;
    auto &[start, end] = match.value();
    std::cout << "MATCH: " << start << ", " << end << ", ";
    std::cout << make_escape(input.substr(start, end - start));
    std::cout << std::endl;
    if (expect_start != start || expect_start + expect_size != end) {
      if (expect_start != (size_t) -1) {
        std::cout
            << "expect start: " << expect_start
            << ", expect size: " << expect_size << std::endl;
      } else {
        std::cout << "expect no match" << std::endl;
      }

      regex_warn("match error");
    }
  } else {
    std::cout << "---------- [  RESULT  ] ----------" << std::endl;
    std::cout << "NO_MATCH" << std::endl;
    if (expect_start != (size_t) -1) {
      std::cout
          << "expect start: " << expect_start
          << ", expect size: " << expect_size << std::endl;

      regex_warn("match error");
    }
  }

  std::cout << std::endl;
}

void test_file(std::istream &stream) {
  std::string buffer{};
  std::optional<Regex> regex{std::nullopt};

  while (std::getline(stream, buffer)) {
    if (buffer.empty()) { continue; }

    if (buffer[0] != '\t') {
      auto pos = buffer.find('\t');
      if (pos != std::string::npos) {
        auto marker = buffer.substr(0, pos);

        auto regex_string = escape_string(buffer.substr(pos + 1));

        std::cout
            << "+---------------------------------------" << std::endl
            << "| TESTING REGEX: " << make_escape(regex_string)
            << std::endl
            << "+---------------------------------------" << std::endl
            << std::endl;

        regex = Regex::init(regex_string);

        switch (marker[0]) {
          case 'I':
            if (regex) {
              regex_warn("expect parse failure");
              regex.reset();
            }
            continue;
          case 'V':
            if (!regex) {
              regex_warn("expect parse success");
              continue;
            }
            break;
          default:
            regex_warn("unknown marker");
            break;
        }

        bool match_empty = false;

        if (marker.size() > 1) {
          switch (marker[1]) {
            case 'E':
              match_empty = true;
              break;
            default:
              regex_warn("unknown marker");
              break;
          }
        }

        if (match_empty) {
          test_match(regex.value(), "", 0, 0);
        } else {
          test_match(regex.value(), "", -1, -1);
        }

        std::cout << std::endl;
      } else {
        regex_warn("invalid format");
      }
    } else {
      if (regex) {
        size_t expect_start = -1, expect_size = -1;
        size_t offset = 1;

        auto pos = buffer.find('\t', offset);
        if (pos != std::string::npos) {
          if (buffer[offset] != '-') {
            expect_start = std::stoi(buffer.substr(offset, pos - offset));
          }
          offset = pos + 1;
        }

        pos = buffer.find('\t', offset);
        if (pos != std::string::npos) {
          if (buffer[offset] != '-') {
            expect_size = std::stoi(buffer.substr(offset, pos - offset));
          }
          offset = pos + 1;
        }

        std::string input = escape_string(buffer.substr(offset));

        test_match(regex.value(), input, expect_start, expect_size);
      }
    }
  }
}

int main(int argc, const char **argv) {
  if (argc < 2) { regex_abort("need three argument"); }

  std::string test_dir = argv[1];

  if (!std::filesystem::is_directory(test_dir)) {
    regex_abort(test_dir.append(" is not a directory"));
  }

  bool has_file = false;

  for (auto &item : std::filesystem::directory_iterator{test_dir}) {
    try {
      if (item.is_regular_file() && item.path().extension() == ".txt") {
        std::ifstream file{item.path()};

        has_file = true;

        std::cout << "########################################" << std::endl;
        std::cout << "# TESTING FILE: " << item.path() << std::endl;
        std::cout << "########################################" << std::endl;
        std::cout << std::endl;

        test_file(file);
      }
    } catch (...) {
      regex_warn("error thrown but suppressed");
      continue;
    }
  }

  if (!has_file) {
    regex_warn(
      std::string("no text file found in directory ").append(test_dir)
    );
  }
}
