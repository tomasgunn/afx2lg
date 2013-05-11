// Copyright (c) 2013, Tomas Gunnarsson
// All rights reserved.

#include "common/file_utils.h"

#include <fstream>

namespace base {

bool FileExists(const std::string& path) {
  std::ifstream file(path);
  return file.good();
}

bool ReadFileIntoBuffer(const std::string& path, unique_ptr<uint8_t[]>* buffer,
                        size_t* file_size) {
  std::ifstream f;
  f.open(path, std::fstream::in | std::ios::binary);
  if (!f.is_open())
    return false;

  f.seekg(0, std::ios::end);
  std::streampos size = f.tellg();
  f.seekg(0, std::ios::beg);

  if (size >= INT_MAX)
    return false;

  *file_size = static_cast<size_t>(size);
  buffer->reset(new uint8_t[*file_size]);
  f.read(reinterpret_cast<char*>(buffer->get()), *file_size);
  
  return true;
}

}  // namespace common
