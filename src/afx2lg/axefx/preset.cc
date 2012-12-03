// Copyright (c) 2012, Tomas Gunnarsson
// All rights reserved.

#include "axefx/preset.h"

#include "axefx/sysex_types.h"

#include <algorithm>
#include <iostream>

namespace {
// Returns true if the string is zero terminated and all characters before the
// terminator are ascii characters.  |count| must include the size of the buffer
// not the assumed length of the string.
// A string of zero length, is not considered ascii by this function.
// Any characters below ' ' are not considered ascii!!! :)
bool IsAsciiString(const char* p, int count) {
  int i = 0;
  for (; i < count; ++i) {
    if (!p[i])
      break;

    if ((p[i] & 0x80) != 0 || p[i] < ' ')
      return false;
  }

  return i != 0 && p[i] == '\0';
}
}

namespace axefx {

const int kInvalidPresetId = -1;
const int kPresetIdBuffer = 0x7F00;

Preset::Preset() : id_(kInvalidPresetId) {}
Preset::~Preset() {}

bool Preset::valid() const {
  return id_ != kInvalidPresetId;
}

bool Preset::is_global_setting() const {
  return id_ >= (3 * 128) && id_ < (4 * 128);
}

bool Preset::from_edit_buffer() const {
  return id_ == kPresetIdBuffer;
}

shared_ptr<BlockParameters> Preset::LookupBlock(AxeFxIIBlockID block) const {
  // TODO: Use a map for lookups?
  std::vector<shared_ptr<BlockParameters> >::const_iterator i =
      block_parameters_.begin();
  for (; i != block_parameters_.end(); ++i) {
    if ((*i)->block() == block)
      return (*i);
  }
  return shared_ptr<BlockParameters>();
}

bool Preset::SetPresetId(const PresetIdHeader& header, int size) {
  ASSERT(header.function() == PRESET_ID);
  ASSERT(size == (sizeof(header) + kSysExTerminationByteCount));
  ASSERT(header.unknown.As16bit() == 0x10);  // <- not sure what this is.
  if (header.preset_number.ms == 0x7f && header.preset_number.ls == 0x0) {
    // This is a special case that means the preset is destined for (or comes
    // from) the edit buffer.  In this case, we just set the id to -1.
    // http://forum.fractalaudio.com/axe-fx-ii-discussion/58581-help-loading-presets-using-sysex-librarian.html#post732659
    id_ = kPresetIdBuffer;
  } else {
    id_ = header.preset_number.As16bit();
  }

  return valid();
}

bool Preset::AddParameterData(const ParameterBlockHeader& header, int size) {
  ASSERT(valid());
  bool ret = params_.AppendFromSysEx(header, size);
  if (!ret) {
    id_ = kInvalidPresetId;
    ASSERT(false);
  }
  return ret;
}

bool Preset::Finalize(const PresetChecksumHeader& header, int size) {
  ASSERT(valid());
  if (header.checksum.As16bit() != params_.Checksum())
    return false;

  PresetParameters::const_iterator p = params_.begin();

  if (is_global_setting()) {
    // For system backups, we treat each preset block as an opaque block of
    // data by default.
    return true;
  }

  uint16_t version = *p;
  // 0 == 516 for fw9 and higher. 514 for older.
  if (version != 0x204 && version != 0x202) {
    std::cerr << "Unsupported syx file - 0x" << std::hex << version << std::dec
              << std::endl;
    return false;
  }

  // Parse the preset name (values 2-32).
  ASSERT(p[1] == 0);  // Not sure what this is.
  p += 2;
  name_.assign(p, p + 31);
  std::string::size_type index = name_.length() - 1;
  while (index > 0 && (name_[index] == ' ' || name_[index] == '\0'))
    --index;  
  name_.resize(index + 1);
  p += 31;
  ++p;  // NULL terminator.

  // Save the effect block matrix.
  COMPILE_ASSERT(sizeof(matrix_[0][0]) == sizeof(*p) * 2,
                 matrix_size_mismatch);
  memcpy(&matrix_[0][0], &(*p), sizeof(matrix_));
  p += sizeof(matrix_) / sizeof(*p);

  // Parse per block parameters (including modifiers).
  while (p < params_.end() && *p) {
    shared_ptr<BlockParameters> block_params(new BlockParameters());
    int values_eaten = block_params->Initialize(&(*p), params_.end() - p);
    if (!values_eaten)
      return false;
    block_parameters_.push_back(block_params);
    p += values_eaten;
  }

  // Free some memory since we don't need it anymore.
  params_.clear();

  return true;
}

}  // namespace axefx
