#include <cstdint>
#include <array>
#include <iostream>

#include "utility.hpp"


struct CharacterSet {
  std::array<uint8_t, 16> set;

  constexpr CharacterSet() : set{} {}

  constexpr CharacterSet(std::array<uint8_t, 16> set) : set{set} {}

  constexpr explicit CharacterSet(CharacterRange range) : set{} {
    for (int i = range.lower_bound; i <= range.upper_bound; ++i) {
      set_char(i);
    }
  }

  constexpr explicit CharacterSet(std::string_view string) : set{} {
    for (auto c: string) { set_char(c); }
  }

  constexpr bool has_char(uint32_t c) const {
    return c < 128 && (set[c / 8] & (1 << c % 8)) > 0;
  }

  constexpr void set_char(uint32_t c) {
    if (c < 128) { set[c / 8] |= 1 << c % 8; }
  }

  CharacterSet &operator|=(const CharacterSet &other) {
    for (size_t i = 0; i < set.size(); ++i) { set[i] |= other.set[i]; }
    return *this;
  }

  void complement() {
    for (size_t i = 0; i < set.size(); ++i) { set[i] = ~set[i]; }
  }

  friend std::ostream &
  operator<<(std::ostream &stream, const CharacterSet &other) {
    stream << '[';
    for (int i = 0; i < 128; ++i) {
      if (other.has_char(i)) {
        stream << make_escape(static_cast<char>(i));
      }
    }
    return stream << ']';
  }
};

static constexpr std::array<uint8_t, 16> CHARACTER_SET_EMPTY{
  0b00000000, //  \0  SOH STX ETX EOT ENQ ACK BEL
  0b00000000, //  BS  \t  \n  \v  \f  \r  SO  SI
  0b00000000, //  DLE DC1 DC2 DC3 DC4 NAK SYN ETB
  0b00000000, //  CAN EM  SUB ESC FS  GS  RS  US
  0b00000000, //  \s  !   "   #   $   %   &   '
  0b00000000, //  (   )   *   +   ,   -   .   /
  0b00000000, //  0   1   2   3   4   5   6   7
  0b00000000, //  8   9   :   ;   <   =   >   ?
  0b00000000, //  @   A   B   C   D   E   F   G
  0b00000000, //  H   I   J   K   L   M   N   O
  0b00000000, //  P   Q   R   S   T   U   V   W
  0b00000000, //  X   Y   Z   [   \   ]   ^   _
  0b00000000, //  `   a   b   c   d   e   f   g
  0b00000000, //  h   i   j   k   l   m   n   o
  0b00000000, //  p   q   r   s   t   u   v   w
  0b00000000, //  x   y   z   {   |   }   ~   DEL
};

static constexpr std::array<uint8_t, 16> CHARACTER_SET_ALL{
  0b11111111, //  \0  SOH STX ETX EOT ENQ ACK BEL
  0b11111111, //  BS  \t  \n  \v  \f  \r  SO  SI
  0b11111111, //  DLE DC1 DC2 DC3 DC4 NAK SYN ETB
  0b11111111, //  CAN EM  SUB ESC FS  GS  RS  US
  0b11111111, //  \s  !   "   #   $   %   &   '
  0b11111111, //  (   )   *   +   ,   -   .   /
  0b11111111, //  0   1   2   3   4   5   6   7
  0b11111111, //  8   9   :   ;   <   =   >   ?
  0b11111111, //  @   A   B   C   D   E   F   G
  0b11111111, //  H   I   J   K   L   M   N   O
  0b11111111, //  P   Q   R   S   T   U   V   W
  0b11111111, //  X   Y   Z   [   \   ]   ^   _
  0b11111111, //  `   a   b   c   d   e   f   g
  0b11111111, //  h   i   j   k   l   m   n   o
  0b11111111, //  p   q   r   s   t   u   v   w
  0b11111111, //  x   y   z   {   |   }   ~   DEL
};

static constexpr std::array<uint8_t, 16> CHARACTER_SET_UPPER{
  0b00000000, //  \0  SOH STX ETX EOT ENQ ACK BEL
  0b00000000, //  BS  \t  \n  \v  \f  \r  SO  SI
  0b00000000, //  DLE DC1 DC2 DC3 DC4 NAK SYN ETB
  0b00000000, //  CAN EM  SUB ESC FS  GS  RS  US
  0b00000000, //  \s  !   "   #   $   %   &   '
  0b00000000, //  (   )   *   +   ,   -   .   /
  0b00000000, //  0   1   2   3   4   5   6   7
  0b00000000, //  8   9   :   ;   <   =   >   ?
  0b11111110, //  @   A   B   C   D   E   F   G
  0b11111111, //  H   I   J   K   L   M   N   O
  0b11111111, //  P   Q   R   S   T   U   V   W
  0b00000111, //  X   Y   Z   [   \   ]   ^   _
  0b00000000, //  `   a   b   c   d   e   f   g
  0b00000000, //  h   i   j   k   l   m   n   o
  0b00000000, //  p   q   r   s   t   u   v   w
  0b00000000, //  x   y   z   {   |   }   ~   DEL
};

static constexpr std::array<uint8_t, 16> CHARACTER_SET_LOWER{
  0b00000000, //  \0  SOH STX ETX EOT ENQ ACK BEL
  0b00000000, //  BS  \t  \n  \v  \f  \r  SO  SI
  0b00000000, //  DLE DC1 DC2 DC3 DC4 NAK SYN ETB
  0b00000000, //  CAN EM  SUB ESC FS  GS  RS  US
  0b00000000, //  \s  !   "   #   $   %   &   '
  0b00000000, //  (   )   *   +   ,   -   .   /
  0b00000000, //  0   1   2   3   4   5   6   7
  0b00000000, //  8   9   :   ;   <   =   >   ?
  0b00000000, //  @   A   B   C   D   E   F   G
  0b00000000, //  H   I   J   K   L   M   N   O
  0b00000000, //  P   Q   R   S   T   U   V   W
  0b00000000, //  X   Y   Z   [   \   ]   ^   _
  0b11111110, //  `   a   b   c   d   e   f   g
  0b11111111, //  h   i   j   k   l   m   n   o
  0b11111111, //  p   q   r   s   t   u   v   w
  0b00000111, //  x   y   z   {   |   }   ~   DEL
};

static constexpr std::array<uint8_t, 16> CHARACTER_SET_ALPHA{
  0b00000000, //  \0  SOH STX ETX EOT ENQ ACK BEL
  0b00000000, //  BS  \t  \n  \v  \f  \r  SO  SI
  0b00000000, //  DLE DC1 DC2 DC3 DC4 NAK SYN ETB
  0b00000000, //  CAN EM  SUB ESC FS  GS  RS  US
  0b00000000, //  \s  !   "   #   $   %   &   '
  0b00000000, //  (   )   *   +   ,   -   .   /
  0b00000000, //  0   1   2   3   4   5   6   7
  0b00000000, //  8   9   :   ;   <   =   >   ?
  0b11111110, //  @   A   B   C   D   E   F   G
  0b11111111, //  H   I   J   K   L   M   N   O
  0b11111111, //  P   Q   R   S   T   U   V   W
  0b00000111, //  X   Y   Z   [   \   ]   ^   _
  0b11111110, //  `   a   b   c   d   e   f   g
  0b11111111, //  h   i   j   k   l   m   n   o
  0b11111111, //  p   q   r   s   t   u   v   w
  0b00000111, //  x   y   z   {   |   }   ~   DEL
};

static constexpr std::array<uint8_t, 16> CHARACTER_SET_DIGIT{
  0b00000000, //  \0  SOH STX ETX EOT ENQ ACK BEL
  0b00000000, //  BS  \t  \n  \v  \f  \r  SO  SI
  0b00000000, //  DLE DC1 DC2 DC3 DC4 NAK SYN ETB
  0b00000000, //  CAN EM  SUB ESC FS  GS  RS  US
  0b00000000, //  \s  !   "   #   $   %   &   '
  0b00000000, //  (   )   *   +   ,   -   .   /
  0b11111111, //  0   1   2   3   4   5   6   7
  0b00000011, //  8   9   :   ;   <   =   >   ?
  0b00000000, //  @   A   B   C   D   E   F   G
  0b00000000, //  H   I   J   K   L   M   N   O
  0b00000000, //  P   Q   R   S   T   U   V   W
  0b00000000, //  X   Y   Z   [   \   ]   ^   _
  0b00000000, //  `   a   b   c   d   e   f   g
  0b00000000, //  h   i   j   k   l   m   n   o
  0b00000000, //  p   q   r   s   t   u   v   w
  0b00000000, //  x   y   z   {   |   }   ~   DEL
};

static constexpr std::array<uint8_t, 16> CHARACTER_SET_XDIGIT{
  0b00000000, //  \0  SOH STX ETX EOT ENQ ACK BEL
  0b00000000, //  BS  \t  \n  \v  \f  \r  SO  SI
  0b00000000, //  DLE DC1 DC2 DC3 DC4 NAK SYN ETB
  0b00000000, //  CAN EM  SUB ESC FS  GS  RS  US
  0b00000000, //  \s  !   "   #   $   %   &   '
  0b00000000, //  (   )   *   +   ,   -   .   /
  0b11111111, //  0   1   2   3   4   5   6   7
  0b00000011, //  8   9   :   ;   <   =   >   ?
  0b01111110, //  @   A   B   C   D   E   F   G
  0b00000000, //  H   I   J   K   L   M   N   O
  0b00000000, //  P   Q   R   S   T   U   V   W
  0b00000000, //  X   Y   Z   [   \   ]   ^   _
  0b01111110, //  `   a   b   c   d   e   f   g
  0b00000000, //  h   i   j   k   l   m   n   o
  0b00000000, //  p   q   r   s   t   u   v   w
  0b00000000, //  x   y   z   {   |   }   ~   DEL
};

static constexpr std::array<uint8_t, 16> CHARACTER_SET_ALNUM{
  0b00000000, //  \0  SOH STX ETX EOT ENQ ACK BEL
  0b00000000, //  BS  \t  \n  \v  \f  \r  SO  SI
  0b00000000, //  DLE DC1 DC2 DC3 DC4 NAK SYN ETB
  0b00000000, //  CAN EM  SUB ESC FS  GS  RS  US
  0b00000000, //  \s  !   "   #   $   %   &   '
  0b00000000, //  (   )   *   +   ,   -   .   /
  0b11111111, //  0   1   2   3   4   5   6   7
  0b00000011, //  8   9   :   ;   <   =   >   ?
  0b11111110, //  @   A   B   C   D   E   F   G
  0b11111111, //  H   I   J   K   L   M   N   O
  0b11111111, //  P   Q   R   S   T   U   V   W
  0b00000111, //  X   Y   Z   [   \   ]   ^   _
  0b11111110, //  `   a   b   c   d   e   f   g
  0b11111111, //  h   i   j   k   l   m   n   o
  0b11111111, //  p   q   r   s   t   u   v   w
  0b00000111, //  x   y   z   {   |   }   ~   DEL
};

static constexpr std::array<uint8_t, 16> CHARACTER_SET_PUNCT{
  0b00000000, //  \0  SOH STX ETX EOT ENQ ACK BEL
  0b00000000, //  BS  \t  \n  \v  \f  \r  SO  SI
  0b00000000, //  DLE DC1 DC2 DC3 DC4 NAK SYN ETB
  0b00000000, //  CAN EM  SUB ESC FS  GS  RS  US
  0b11111110, //  \s  !   "   #   $   %   &   '
  0b11111111, //  (   )   *   +   ,   -   .   /
  0b00000000, //  0   1   2   3   4   5   6   7
  0b11111100, //  8   9   :   ;   <   =   >   ?
  0b00000001, //  @   A   B   C   D   E   F   G
  0b00000000, //  H   I   J   K   L   M   N   O
  0b00000000, //  P   Q   R   S   T   U   V   W
  0b11111000, //  X   Y   Z   [   \   ]   ^   _
  0b00000001, //  `   a   b   c   d   e   f   g
  0b00000000, //  h   i   j   k   l   m   n   o
  0b00000000, //  p   q   r   s   t   u   v   w
  0b01111000, //  x   y   z   {   |   }   ~   DEL
};

static constexpr std::array<uint8_t, 16> CHARACTER_SET_BLANK{
  0b00000000, //  \0  SOH STX ETX EOT ENQ ACK BEL
  0b00000010, //  BS  \t  \n  \v  \f  \r  SO  SI
  0b00000000, //  DLE DC1 DC2 DC3 DC4 NAK SYN ETB
  0b00000000, //  CAN EM  SUB ESC FS  GS  RS  US
  0b00000001, //  \s  !   "   #   $   %   &   '
  0b00000000, //  (   )   *   +   ,   -   .   /
  0b00000000, //  0   1   2   3   4   5   6   7
  0b00000000, //  8   9   :   ;   <   =   >   ?
  0b00000000, //  @   A   B   C   D   E   F   G
  0b00000000, //  H   I   J   K   L   M   N   O
  0b00000000, //  P   Q   R   S   T   U   V   W
  0b00000000, //  X   Y   Z   [   \   ]   ^   _
  0b00000000, //  `   a   b   c   d   e   f   g
  0b00000000, //  h   i   j   k   l   m   n   o
  0b00000000, //  p   q   r   s   t   u   v   w
  0b00000000, //  x   y   z   {   |   }   ~   DEL
};

static constexpr std::array<uint8_t, 16> CHARACTER_SET_SPACE{
  0b00000000, //  \0  SOH STX ETX EOT ENQ ACK BEL
  0b00111110, //  BS  \t  \n  \v  \f  \r  SO  SI
  0b00000000, //  DLE DC1 DC2 DC3 DC4 NAK SYN ETB
  0b00000000, //  CAN EM  SUB ESC FS  GS  RS  US
  0b00000001, //  \s  !   "   #   $   %   &   '
  0b00000000, //  (   )   *   +   ,   -   .   /
  0b00000000, //  0   1   2   3   4   5   6   7
  0b00000000, //  8   9   :   ;   <   =   >   ?
  0b00000000, //  @   A   B   C   D   E   F   G
  0b00000000, //  H   I   J   K   L   M   N   O
  0b00000000, //  P   Q   R   S   T   U   V   W
  0b00000000, //  X   Y   Z   [   \   ]   ^   _
  0b00000000, //  `   a   b   c   d   e   f   g
  0b00000000, //  h   i   j   k   l   m   n   o
  0b00000000, //  p   q   r   s   t   u   v   w
  0b00000000, //  x   y   z   {   |   }   ~   DEL
};

static constexpr std::array<uint8_t, 16> CHARACTER_SET_CONTRL{
  0b11111111, //  \0  SOH STX ETX EOT ENQ ACK BEL
  0b11111111, //  BS  \t  \n  \v  \f  \r  SO  SI
  0b11111111, //  DLE DC1 DC2 DC3 DC4 NAK SYN ETB
  0b11111111, //  CAN EM  SUB ESC FS  GS  RS  US
  0b00000000, //  \s  !   "   #   $   %   &   '
  0b00000000, //  (   )   *   +   ,   -   .   /
  0b00000000, //  0   1   2   3   4   5   6   7
  0b00000000, //  8   9   :   ;   <   =   >   ?
  0b00000000, //  @   A   B   C   D   E   F   G
  0b00000000, //  H   I   J   K   L   M   N   O
  0b00000000, //  P   Q   R   S   T   U   V   W
  0b00000000, //  X   Y   Z   [   \   ]   ^   _
  0b00000000, //  `   a   b   c   d   e   f   g
  0b00000000, //  h   i   j   k   l   m   n   o
  0b00000000, //  p   q   r   s   t   u   v   w
  0b10000000, //  x   y   z   {   |   }   ~   DEL
};

static constexpr std::array<uint8_t, 16> CHARACTER_SET_GRAPH{
  0b00000000, //  \0  SOH STX ETX EOT ENQ ACK BEL
  0b00000000, //  BS  \t  \n  \v  \f  \r  SO  SI
  0b00000000, //  DLE DC1 DC2 DC3 DC4 NAK SYN ETB
  0b00000000, //  CAN EM  SUB ESC FS  GS  RS  US
  0b11111110, //  \s  !   "   #   $   %   &   '
  0b11111111, //  (   )   *   +   ,   -   .   /
  0b11111111, //  0   1   2   3   4   5   6   7
  0b11111111, //  8   9   :   ;   <   =   >   ?
  0b11111111, //  @   A   B   C   D   E   F   G
  0b11111111, //  H   I   J   K   L   M   N   O
  0b11111111, //  P   Q   R   S   T   U   V   W
  0b11111111, //  X   Y   Z   [   \   ]   ^   _
  0b11111111, //  `   a   b   c   d   e   f   g
  0b11111111, //  h   i   j   k   l   m   n   o
  0b11111111, //  p   q   r   s   t   u   v   w
  0b01111111, //  x   y   z   {   |   }   ~   DEL
};

static constexpr std::array<uint8_t, 16> CHARACTER_SET_PRINT{
  0b00000000, //  \0  SOH STX ETX EOT ENQ ACK BEL
  0b00000000, //  BS  \t  \n  \v  \f  \r  SO  SI
  0b00000000, //  DLE DC1 DC2 DC3 DC4 NAK SYN ETB
  0b00000000, //  CAN EM  SUB ESC FS  GS  RS  US
  0b11111111, //  \s  !   "   #   $   %   &   '
  0b11111111, //  (   )   *   +   ,   -   .   /
  0b11111111, //  0   1   2   3   4   5   6   7
  0b11111111, //  8   9   :   ;   <   =   >   ?
  0b11111111, //  @   A   B   C   D   E   F   G
  0b11111111, //  H   I   J   K   L   M   N   O
  0b11111111, //  P   Q   R   S   T   U   V   W
  0b11111111, //  X   Y   Z   [   \   ]   ^   _
  0b11111111, //  `   a   b   c   d   e   f   g
  0b11111111, //  h   i   j   k   l   m   n   o
  0b11111111, //  p   q   r   s   t   u   v   w
  0b01111111, //  x   y   z   {   |   }   ~   DEL
};

static constexpr std::array<uint8_t, 16> CHARACTER_SET_WORD{
  0b00000000, //  \0  SOH STX ETX EOT ENQ ACK BEL
  0b00000000, //  BS  \t  \n  \v  \f  \r  SO  SI
  0b00000000, //  DLE DC1 DC2 DC3 DC4 NAK SYN ETB
  0b00000000, //  CAN EM  SUB ESC FS  GS  RS  US
  0b00000000, //  \s  !   "   #   $   %   &   '
  0b00000000, //  (   )   *   +   ,   -   .   /
  0b11111111, //  0   1   2   3   4   5   6   7
  0b00000011, //  8   9   :   ;   <   =   >   ?
  0b11111110, //  @   A   B   C   D   E   F   G
  0b11111111, //  H   I   J   K   L   M   N   O
  0b11111111, //  P   Q   R   S   T   U   V   W
  0b10000111, //  X   Y   Z   [   \   ]   ^   _
  0b11111110, //  `   a   b   c   d   e   f   g
  0b11111111, //  h   i   j   k   l   m   n   o
  0b11111111, //  p   q   r   s   t   u   v   w
  0b00000111, //  x   y   z   {   |   }   ~   DEL
};
