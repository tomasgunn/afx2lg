// Copyright (c) 2013, Tomas Gunnarsson
// All rights reserved.

#pragma once
#ifndef COMMON_FILE_UTILS_H_
#define COMMON_FILE_UTILS_H_

#include <string>

#include "common_types.h"

namespace base {

bool FileExists(const std::string& path);

bool ReadFileIntoBuffer(const std::string& path, unique_ptr<uint8_t[]>* buffer,
                        size_t* file_size);

}  // namespace base

#endif  // COMMON_FILE_UTILS_H_
