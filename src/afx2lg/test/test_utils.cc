// Copyright (c) 2012, Tomas Gunnarsson
// All rights reserved.

#include "test/test_utils.h"

#include "gtest/gtest.h"

#include <climits>
#include <fstream>

using testing::internal::FilePath;
extern std::string g_process_path;

// This assumes that the working directory is the same as the build folder.
FilePath GetTestFilePath(const std::string& test_file) {
  FilePath ret(g_process_path);
  ret.Set(ret.RemoveFileName());
  ret.Set(FilePath::ConcatPaths(ret, FilePath("../../afx2lg/test/data")));
  ret.Set(FilePath::ConcatPaths(ret, FilePath(test_file)));
  return ret;
}

bool ReadTestFileIntoBuffer(const std::string& file,
                            std::unique_ptr<uint8_t>* buffer,
                            int* file_size) {
  FilePath path(GetTestFilePath(file));
  std::ifstream f;
  f.open(path.c_str(), std::fstream::in | std::ios::binary);
  if (!f.is_open())
    return false;

  f.seekg(0, std::ios::end);
  std::streampos size = f.tellg();
  f.seekg(0, std::ios::beg);

  if (size >= INT_MAX)
    return false;

  *file_size = static_cast<int>(size);
  buffer->reset(new uint8_t[*file_size]);
  f.read(reinterpret_cast<char*>(buffer->get()), *file_size);
  
  return true;
}

