// Copyright (c) 2012, Tomas Gunnarsson
// All rights reserved.

#pragma once
#ifndef TEST_TEST_UTILS_H_
#define TEST_TEST_UTILS_H_

#include "common_types.h"

#include <memory>
#include <string>

bool ReadTestFileIntoBuffer(const std::string& file,
                            std::auto_ptr<uint8_t>* buffer,
                            int* file_size);


#endif
