// Copyright (c) 2012, Tomas Gunnarsson
// All rights reserved.

#pragma once
#ifndef AXE_FX_SYSEX_PARSER_H_
#define AXE_FX_SYSEX_PARSER_H_

#include "common_types.h"
#include "preset.h"

#include <vector>

namespace axefx {

struct FractalSysExHeader;
struct PresetIdHeader;
struct PresetProperty;

typedef std::vector<uint16_t> BlockData;

class SysExParser {
 public:
  SysExParser();
  ~SysExParser();

  void ParseSysExBuffer(const uint8_t* begin, const uint8_t* end);

  const PresetMap& presets() const { return presets_; }

 protected:
  void ParseSingleSysEx(BlockData* block_data, const uint8_t* sys_ex, int size,
                        Preset* preset);
  void ParseFractalSysEx(BlockData* block_data,
                         const FractalSysExHeader& header,
                         int size, Preset* preset);
  void ParsePresetId(const PresetIdHeader& header, int size, Preset* preset);
  void ParsePresetProperties(int* preset_chunk_id,
                             const PresetProperty& header, int size,
                             Preset* preset);
  void AppendBlockData(BlockData* block_data,
                       const FractalSysExHeader& header,
                       int size);
  void ParseBlockData(const BlockData& data, Preset* preset);

 private:
  PresetMap presets_;
};

}  // namespace axefx

#endif
