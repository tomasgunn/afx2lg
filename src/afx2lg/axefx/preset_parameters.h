// Copyright (c) 2012, Tomas Gunnarsson
// All rights reserved.

#pragma once
#ifndef AXE_FX_PRESET_PARAMETERS_H_
#define AXE_FX_PRESET_PARAMETERS_H_

#include "common/common_types.h"
#include "axefx/sysex_callback.h"
#include "axefx/sysex_types.h"

#include <vector>

namespace axefx {

class PresetParameters : public std::vector<uint16_t> {
 public:
  PresetParameters();
  ~PresetParameters();

  bool AppendFromSysEx(const ParameterBlockHeader& header, size_t header_size);

  uint16_t Checksum() const;

  bool Serialize(const SysExCallback& callback) const;
};

}  // namespace axefx

#endif
