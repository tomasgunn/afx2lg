// Copyright (c) 2012, Tomas Gunnarsson
// All rights reserved.

#include "test/test_utils.h"

#include "gtest/gtest.h"

#include <fstream>

using testing::internal::FilePath;
extern std::string g_process_path;

// This assumes that the working directory is the same as the build folder.
FilePath GetTestFilePath(const std::string& test_file) {
  FilePath ret(g_process_path);
  ret.Set(ret.RemoveFileName());
  ret.Set(FilePath::ConcatPaths(ret, FilePath("../test/data")));
  ret.Set(FilePath::ConcatPaths(ret, FilePath(test_file)));
  return ret;
}

bool ReadTestFileIntoBuffer(const std::string& file,
                            std::auto_ptr<byte>* buffer,
                            std::streampos* file_size) {
  FilePath path(GetTestFilePath(file));
  std::ifstream f;
  f.open(path.c_str(), std::fstream::in | std::ios::binary);
  if (!f.is_open())
    return false;

  f.seekg(0, std::ios::end);
  *file_size = f.tellg();
  f.seekg(0, std::ios::beg);

  buffer->reset(new byte[*file_size]);
  f.read(reinterpret_cast<char*>(buffer->get()), *file_size);
  
  return true;
}

