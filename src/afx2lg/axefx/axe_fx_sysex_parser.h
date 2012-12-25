// Copyright (c) 2012, Tomas Gunnarsson
// All rights reserved.

#pragma once
#ifndef AXE_FX_SYSEX_PARSER_H_
#define AXE_FX_SYSEX_PARSER_H_

#include "common/common_types.h"

#include "axefx/preset_parameters.h"

#include <map>

namespace axefx {

class Preset;
typedef std::map<int, shared_ptr<Preset> > PresetMap;

class SysExParser {
 public:
  SysExParser();
  ~SysExParser();

  bool ParseSysExBuffer(const uint8_t* begin, const uint8_t* end);

  const PresetMap& presets() const { return presets_; }
  PresetMap& presets() { return presets_; }

  bool Serialize(const SysExCallback& callback) const;

 private:
  PresetMap presets_;
};

}  // namespace axefx

#endif
