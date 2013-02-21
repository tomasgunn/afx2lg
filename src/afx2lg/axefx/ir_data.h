// Copyright (c) 2012, Tomas Gunnarsson
// All rights reserved.

#pragma once
#ifndef AXE_FX_IR_DATA_H_
#define AXE_FX_IR_DATA_H_

#include "common/common_types.h"
#include "axefx/sysex_callback.h"
#include "axefx/sysex_types.h"

#include <vector>

namespace axefx {

// TODO: This class borrows a lot from PresetParameters and a few tings
// from Preset.  Refactor!
class IRData {
 public:
  IRData();
  explicit IRData(const IRIdHeader& header);
  ~IRData();

  uint16_t id() const { return id_; }
  void set_id(uint16_t id) { id_ = id; }

  // TODO: Support a way to set the name.
  std::string name() const;
  uint32_t Checksum() const;

  bool AppendFromSysEx(const IRBlockHeader& header, size_t header_size);

  bool from_edit_buffer() const { return id_ == kEditBufferId; }

  bool Serialize(const SysExCallback& callback) const;

 private:
  uint16_t id_;
  std::vector<uint32_t> data_;
};

}  // namespace axefx

#endif
