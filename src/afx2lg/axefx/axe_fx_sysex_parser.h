// Copyright (c) 2012, Tomas Gunnarsson
// All rights reserved.

#pragma once
#ifndef AXE_FX_SYSEX_PARSER_H_
#define AXE_FX_SYSEX_PARSER_H_

#include "common/common_types.h"

#include <map>
#include <memory>

namespace axefx {

using std::shared_ptr;

class Preset;
typedef std::map<int, shared_ptr<Preset> > PresetMap;

class SysExParser {
 public:
  SysExParser();
  ~SysExParser();

  bool ParseSysExBuffer(const uint8_t* begin, const uint8_t* end);

  const PresetMap& presets() const { return presets_; }

 private:
  PresetMap presets_;
};

}  // namespace axefx

#endif
