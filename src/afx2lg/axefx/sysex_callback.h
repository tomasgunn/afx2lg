// Copyright (c) 2012, Tomas Gunnarsson
// All rights reserved.

#pragma once
#ifndef AXE_FX_SYSEX_CALLBACK_H_
#define AXE_FX_SYSEX_CALLBACK_H_

#include "common/common_types.h"

#include <functional>
#include <vector>

namespace axefx {

typedef std::function<void(const std::vector<uint8_t>&)> SysExCallback;

}  // namespace axefx

#endif  // AXE_FX_SYSEX_CALLBACK_H_
