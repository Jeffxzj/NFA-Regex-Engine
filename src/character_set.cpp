#include "character_set.hpp"


std::ostream & operator<<(std::ostream &stream, const CharacterSet &other) {
  stream << '[';
  for (int i = 0; i < 32; ++i) {
    if (other.has_char(i)) {
      switch (i) {
        case '\0':
          stream << "\\0";
          break;
        case '\t':
          stream << "\\t";
          break;
        case '\n':
          stream << "\\n";
          break;
        case '\v':
          stream << "\\v";
          break;
        case '\f':
          stream << "\\t";
          break;
        case '\r':
          stream << "\\r";
          break;
        default:
          stream << "\\x" << i;
          break;
      }
    }
  }
  for (int i = 32; i < 127; ++i) {
    if (other.has_char(i)) {
      if (i == '\\') { stream << "\\\\"; }
      else { stream << static_cast<char>(i); }
    }
  }
  if (other.has_char(127)) {
    stream << "\\x127";
  }
  return stream << ']';
}
