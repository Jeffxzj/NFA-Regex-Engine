#ifndef REGEX_UTILITY
#define REGEX_UTILITY


#include <utility>
#include <cstdint>
#include <iostream>
#include <iomanip>


#define regex_no_return __attribute__((noreturn))

#define regex_likely(x)  __builtin_expect((x), 1)
#define regex_unlikely(x) __builtin_expect((x), 0)

inline void regex_warn(const char *file, int line, std::string msg) {
  std::cerr
      << "Warn at file " << file << ", line " << line << ": "
      << msg << std::endl;
}

#define regex_warn(msg) regex_warn(__FILE__, __LINE__, msg)

inline regex_no_return void
regex_abort(const char *file, int line, std::string msg) {
  std::cerr
      << "Abort at file " << file << ", line " << line << ": "
      << msg << std::endl;

  exit(1);
}

#define regex_abort(msg) regex_abort(__FILE__, __LINE__, msg)

inline void
regex_assert(const char *file, int line, bool result, std::string msg) {
  if (regex_unlikely(!result)) {
    std::cerr
        << "Assert failed at file " << file << ", line " << line << ": "
        << msg << std::endl;

    exit(1);
  }
}

#if defined(__DEBUG__)
#define regex_assert(expr) regex_assert(__FILE__, __LINE__, expr, #expr)
#else
#define regex_assert(expr)
#endif


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

  bool operator==(const CharacterRange &other) const {
    return
        lower_bound == other.lower_bound &&
        upper_bound == other.upper_bound;
  }

  bool operator<(const CharacterRange &other) const {
    if (upper_bound - lower_bound != other.upper_bound - other.lower_bound) {
      return upper_bound - lower_bound < other.upper_bound - other.lower_bound;
    } else {
      return lower_bound < other.upper_bound;
    }
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

  bool operator==(const RepeatRange &other) const {
    return
        lower_bound == other.lower_bound &&
        upper_bound == other.upper_bound;
  }

  bool operator<(const RepeatRange &other) const {
    if (upper_bound - lower_bound != other.upper_bound - other.lower_bound) {
      return upper_bound - lower_bound < other.upper_bound - other.lower_bound;
    } else {
      return lower_bound < other.upper_bound;
    }
  }

  bool in_lower_range(size_t value) const {
    return value >= lower_bound;
  }

  bool in_upper_range(size_t value) const {
    return upper_bound == 0 || value < upper_bound;
  }

  bool in_range(size_t value) const {
    return in_lower_range(value) && in_upper_range(value);
  }
};

template<class T>
struct Escape {
};

template<class T>
Escape<T> make_escape(const T &other) { return Escape<T>{other}; }

template<>
struct Escape<char> {
  const char &c;

  friend std::ostream &operator<<(std::ostream &stream, const Escape &other) {
    const char &c = other.c;

    if (c >= 32) {
      switch (c) {
        case '\x7f':
          return stream << "\\x7f";
        default:
          return stream << c;
      }
    } else if (c >= 0) {
      switch (c) {
        case '\t':
          return stream << "\\t";
        case '\n':
          return stream << "\\n";
        case '\v':
          return stream << "\\v";
        case '\f':
          return stream << "\\t";
        case '\r':
          return stream << "\\r";
        case '\\':
          return stream << "\\\\";
        default:
          return stream
              << "\\x" << std::setw(2) << std::hex << std::setfill('0')
              << static_cast<int>(c)
              << std::setw(0) << std::dec << std::setfill(' ');
      }
    } else {
      regex_warn("meet none ascii");
      return stream;
    }
  }
};

template<>
struct Escape<std::string> {
  const std::string &string;

  friend std::ostream &operator<<(std::ostream &stream, const Escape &other) {
    for (const char &c : other.string) { stream << make_escape(c); }
    return stream;
  }
};


#endif // REGEX_UTILITY
