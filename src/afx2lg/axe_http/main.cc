// Copyright (c) 2012, Tomas Gunnarsson
// All rights reserved.

#include "common/common_types.h"

#include <iostream>

void PrintUsage() {
  std::cerr << "TODO\n";
}

bool ParseArgs(int argc, char* argv[]) {
  return false;
}

int main(int argc, char* argv[]) {
  if (!ParseArgs(argc, argv)) {
    PrintUsage();
    return -1;
  }

  return 0;
}
