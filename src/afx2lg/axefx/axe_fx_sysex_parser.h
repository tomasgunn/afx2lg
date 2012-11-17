// Copyright (c) 2012, Tomas Gunnarsson
// All rights reserved.

#pragma once
#ifndef AXE_FX_SYSEX_PARSER_H_
#define AXE_FX_SYSEX_PARSER_H_

#include "common_types.h"
#include "preset.h"

namespace axefx {

struct FractalSysExHeader;
struct PresetIdHeader;
struct PresetProperty;

class SysExParser {
 public:
  SysExParser();
  ~SysExParser();

  void ParseSysExBuffer(const byte* begin, const byte* end);

  const PresetMap& presets() const { return presets_; }

 protected:
  void ParseSingleSysEx(int* preset_chunk_id, const byte* sys_ex, int size,
                        Preset* preset);
  void ParseFractalSysEx(int* preset_chunk_id, const FractalSysExHeader& header,
                         int size, Preset* preset);
  void ParsePresetId(const PresetIdHeader& header, int size, Preset* preset);
  void ParsePresetProperties(int* preset_chunk_id,
                             const PresetProperty& header, int size,
                             Preset* preset);
  void ParsePresetEpilogue(const FractalSysExHeader& header, int size,
                           Preset* preset);

 private:
  PresetMap presets_;
};

}  // namespace axefx

#endif
