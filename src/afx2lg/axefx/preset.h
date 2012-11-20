// Copyright (c) 2012, Tomas Gunnarsson
// All rights reserved.

#pragma once
#ifndef AXEFX_PRESET_H_
#define AXEFX_PRESET_H_

#include "common_types.h"
#include "axefx/preset_parameters.h"

#include <memory>
#include <map>
#include <string>

namespace axefx {
using std::tr1::shared_ptr;

struct ParameterBlockHeader;
struct PresetIdHeader;
struct PresetChecksumHeader;

class Preset {
 public:
  Preset();
  ~Preset();

  int id() const { return id_; }
  const std::string& name() const { return name_; }

  bool valid() const;
  // Returns true if the preset doesn't have an ID assigned to it, but is
  // still valid (from AxeFx' edit buffer).
  bool from_edit_buffer() const;

  bool SetPresetId(const PresetIdHeader& header, int size);
  bool AddParameterData(const ParameterBlockHeader& header, int size);
  bool Finalize(const PresetChecksumHeader& header, int size);

 private:
  int id_;
  std::string name_;
  PresetParameters params_;
};

}  // namespace axefx

#endif  // AXEFX_PRESET_H_
