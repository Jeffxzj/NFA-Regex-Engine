#include "regex.hpp"
#include "list.hpp"

#include <iostream>
#include <string>

int main(int argc, const char **argv) {
  if (argc < 2) { exit(1); }
  Regex::init(argv[1]);
}
