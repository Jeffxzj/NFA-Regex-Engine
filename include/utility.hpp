#ifndef REGEX_UTILITY
#define REGEX_UTILITY


#include <utility>
#include <cstdint>
#include <iostream>


#define CASE_NUMERIC \
  '0':      case '1': case '2': case '3': case '4': \
  case '5': case '6': case '7': case '8': case '9'

#define CASE_LOWER_CASE \
  'a':      case 'b': case 'c': case 'd': case 'e': \
  case 'f': case 'g': case 'h': case 'i': case 'j': \
  case 'k': case 'l': case 'm': case 'n': case 'o': \
  case 'p': case 'q': case 'r': case 's': case 't': \
  case 'u': case 'v': case 'w': case 'x': case 'y': \
  case 'z'

#define CASE_UPPER_CASE \
  'A':      case 'B': case 'C': case 'D': case 'E': \
  case 'F': case 'G': case 'H': case 'I': case 'J': \
  case 'K': case 'L': case 'M': case 'N': case 'O': \
  case 'P': case 'Q': case 'R': case 'S': case 'T': \
  case 'U': case 'V': case 'W': case 'X': case 'Y': \
  case 'Z'


struct CharacterRange {
  char lower_bound;
  char upper_bound;

  friend std::ostream &
  operator<<(std::ostream &stream, const CharacterRange &other) {
    return stream
        << '[' << other.lower_bound << '-'
        << other.upper_bound << ']';
  }
};

struct RepeatRange {
  size_t lower_bound; // >=1
  size_t upper_bound; // if 0 means no upperbound

  friend std::ostream &
  operator<<(std::ostream &stream, const RepeatRange &other) {
    stream << "{" << other.lower_bound << ',';
    if (other.upper_bound != 0) { stream << other.upper_bound; }
    return stream << '}';
  }
};


#endif // REGEX_UTILITY
