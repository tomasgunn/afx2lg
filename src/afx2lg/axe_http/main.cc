// Copyright (c) 2012, Tomas Gunnarsson
// All rights reserved.

#include "common/common_types.h"

#include "axe_http/server.h"

#include <iostream>

std::string g_process_path;

int main(int argc, char** argv) {
  g_process_path = argv[0];

  // TODO(tommi): make configurable.
  static const uint16_t port = 8888;

  Server server;
  if (!server.Initialize(port)) {
    std::cerr << "Failed to initialize server socket\n";
    return -1;
  }

  std::cout << "Server listening on port " << port << std::endl;

  server.Run();

  return 0;
}
