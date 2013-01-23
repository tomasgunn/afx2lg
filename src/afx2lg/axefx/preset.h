// Copyright (c) 2012, Tomas Gunnarsson
// All rights reserved.

#pragma once
#ifndef AXEFX_PRESET_H_
#define AXEFX_PRESET_H_

#include "common/common_types.h"

#include "axefx/blocks.h"
#include "axefx/preset_parameters.h"

#include <map>
#include <string>
#include <vector>

namespace Json {
class Value;
}

namespace axefx {

struct ParameterBlockHeader;
struct PresetIdHeader;
struct PresetChecksumHeader;

class Preset {
 public:
  Preset();
  ~Preset();

  int id() const { return id_; }
  void set_id(int id);
  const std::string& name() const { return name_; }
  void set_name(const std::string& name);
  const Matrix& matrix() const { return matrix_; }
  const PresetParameters& params() const { return params_; }

  bool valid() const;

  bool is_global_setting() const;

  // Returns true if the preset doesn't have an ID assigned to it, but is
  // still valid (from AxeFx' edit buffer).
  bool from_edit_buffer() const;
  void SetAsEditBuffer();

  BlockParameters* LookupBlock(AxeFxIIBlockID block);

  // Parse methods.
  bool SetPresetId(const PresetIdHeader& header, size_t size);
  bool AddParameterData(const ParameterBlockHeader& header, size_t size);

  // Verify and optionally parse the preset data.  If you set |verify_only| to
  // true, the data will not be parsed but the checksum will be verified.
  // If you only verify the data, don't expect methods that access blocks,
  // matrix or other parameters to work correctly.
  bool Finalize(const PresetChecksumHeader* header, size_t size,
                bool verify_only);

  void ToJson(Json::Value* out) const;

  bool Serialize(const SysExCallback& callback) const;

 private:
  void WriteHeader(const SysExCallback& callback) const;
  void FillParameters(PresetParameters* params) const;
  void WriteChecksum(uint16_t checksum, const SysExCallback& callback) const;

  // Valid while parsing, then discarded.
  // TODO: rename PresetParameters to PresetData?
  PresetParameters params_;

  // Valid after parsing only.
  int id_;
  std::string name_;
  Matrix matrix_;
  std::vector<unique_ptr<BlockParameters> > block_parameters_;
};

}  // namespace axefx

#endif  // AXEFX_PRESET_H_
