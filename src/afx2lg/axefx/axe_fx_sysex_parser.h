// Copyright (c) 2012, Tomas Gunnarsson
// All rights reserved.

#pragma once
#ifndef AXE_FX_SYSEX_PARSER_H_
#define AXE_FX_SYSEX_PARSER_H_

#include "common/common_types.h"

#include "axefx/preset_parameters.h"
#include "axefx/sysex_callback.h"

#include <map>

namespace axefx {

class IRData;
class Preset;
typedef std::map<int, shared_ptr<Preset> > PresetMap;
typedef std::vector<unique_ptr<IRData> > IRDataArray;

class SysExParser {
 public:
  SysExParser();
  ~SysExParser();

  bool ParseSysExBuffer(const uint8_t* begin, const uint8_t* end,
                        bool parse_parameter_data);

  const PresetMap& presets() const { return presets_; }
  PresetMap& presets() { return presets_; }
  IRDataArray& ir_array() { return ir_array_; }

  bool Serialize(const SysExCallback& callback) const;

 private:
  PresetMap presets_;
  IRDataArray ir_array_;
};

}  // namespace axefx

#endif
