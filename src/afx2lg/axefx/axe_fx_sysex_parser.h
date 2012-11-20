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
struct ParameterBlockHeader;
struct PresetIdHeader;
struct PresetProperty;

class PresetParameters : public std::vector<uint16_t> {
 public:
  PresetParameters();
  ~PresetParameters();

  bool AppendFromSysEx(const ParameterBlockHeader& header, int header_size);

  uint16_t Checksum() const;
};

class SysExParser {
 public:
  SysExParser();
  ~SysExParser();

  bool ParseSysExBuffer(const uint8_t* begin, const uint8_t* end);

  const PresetMap& presets() const { return presets_; }

 protected:
  void ParseFractalSysEx(PresetParameters* block_data,
                         const FractalSysExHeader& header,
                         int size, Preset* preset);
  void ParsePresetId(const PresetIdHeader& header, int size, Preset* preset);

  // TODO: Remove
  void ParsePresetParameters(int* preset_chunk_id,
                             const PresetProperty& header, int size,
                             Preset* preset);

  void ParseBlockData(const PresetParameters& data, Preset* preset);

 private:
  PresetMap presets_;
};

}  // namespace axefx

#endif
