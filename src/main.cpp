#include "regex.hpp"


int main(int argc, const char **argv) {
  if (argc < 2) { exit(1); }
  Regex::init(argv[1]);
}
