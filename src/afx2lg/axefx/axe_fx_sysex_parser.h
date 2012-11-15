// Copyright (c) 2012, Tomas Gunnarsson
// All rights reserved.

#pragma once
#ifndef AXE_FX_SYSEX_PARSER_H_
#define AXE_FX_SYSEX_PARSER_H_

#include "common_types.h"
#include "preset.h"

class AxeFxSysExParser {
 public:
  AxeFxSysExParser();
  ~AxeFxSysExParser();

  void ParseSysExBuffer(const byte* begin, const byte* end);

  const PresetMap& presets() const { return presets_; }

 protected:
  struct FractalSysExHeader;
  struct PresetIdHeader;
  struct PresetProperty;

  void ParseSingleSysEx(const byte* sys_ex, int size,
                        Preset* preset);
  void ParseFractalSysEx(const FractalSysExHeader& header, int size,
                         Preset* preset);
  void ParsePresetId(const PresetIdHeader& header, int size, Preset* preset);
  void ParsePresetProperties(const PresetProperty& header, int size,
                             Preset* preset);
  void ParsePresetEpilogue(const FractalSysExHeader& header, int size,
                           Preset* preset);

 private:
  PresetMap presets_;
};

#endif
