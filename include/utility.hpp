#ifndef REGEX_UTILITY
#define REGEX_UTILITY


#include <utility>
#include <cstdint>
#include <iostream>


#define regex_no_return __attribute__((noreturn))

inline void regex_warn(const char *file, int line, const char *msg) {
  std::cerr
      << "Warn at file " << file << ", line " << line << ": "
      << msg << std::endl;
}

#define regex_warn(msg) regex_warn(__FILE__, __LINE__, msg)

inline regex_no_return void
regex_abort(const char *file, int line, const char *msg) {
  std::cerr
      << "Abort at file " << file << ", line " << line << ": "
      << msg << std::endl;

  exit(1);
}

#define regex_abort(msg) regex_abort(__FILE__, __LINE__, msg)

inline void
regex_assert(const char *file, int line, bool result, const char *msg) {
  if (!result) {
    std::cerr
        << "Assert failed at file " << file << ", line " << line << ": "
        << msg << std::endl;

    exit(1);
  }
}

#define regex_assert(expr) regex_assert(__FILE__, __LINE__, expr, #expr)


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

  bool in_lower_range(size_t value) const {
    return value >= lower_bound;
  }

  bool in_upper_range(size_t value) const {
    return upper_bound != 0 && value < upper_bound;
  }

  bool in_range(size_t value) const {
    return in_lower_range(value) && in_upper_range(value);
  }
};


#endif // REGEX_UTILITY
