#include "gtest/gtest.h"

std::string g_process_path;

int main(int argc, char* argv[]) {
  g_process_path = argv[0];
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
