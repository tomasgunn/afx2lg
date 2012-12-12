// Copyright (c) 2012, Tomas Gunnarsson
// All rights reserved.

#pragma once
#ifndef TEST_TEST_UTILS_H_
#define TEST_TEST_UTILS_H_

#include "common/common_types.h"

#include <string>

bool ReadTestFileIntoBuffer(const std::string& file,
                            std::unique_ptr<uint8_t>* buffer,
                            int* file_size);


#endif
