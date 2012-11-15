#pragma once
#ifndef TEST_TEST_UTILS_H_
#define TEST_TEST_UTILS_H_

#include "common_types.h"

#include <memory>
#include <string>

bool ReadTestFileIntoBuffer(const std::string& file,
                            std::auto_ptr<byte>* buffer,
                            size_t* file_size);


#endif
