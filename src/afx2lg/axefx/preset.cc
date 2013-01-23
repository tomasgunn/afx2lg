// Copyright (c) 2012, Tomas Gunnarsson
// All rights reserved.

#include "axefx/preset.h"

#include "axefx/sysex_types.h"
#include "json/value.h"

#include <algorithm>
#include <iostream>

namespace axefx {

const int kInvalidPresetId = -1;
const int kPresetIdBuffer = (0x7F << 7u);
const uint16_t kCurrentParameterVersion = 0x204;

Preset::Preset() : id_(kInvalidPresetId) {}
Preset::~Preset() {}

void Preset::set_id(int id) {
  ASSERT(id >= 0 && id < 512);
  id_ = id;
}

void Preset::set_name(const std::string& name) {
  ASSERT(params_.empty());
  name.length() >= 31 ? name_ = name.substr(0, 30) : name_ = name;
}

bool Preset::valid() const {
  return id_ != kInvalidPresetId;
}

bool Preset::is_global_setting() const {
  return id_ >= (3 * 128) && id_ < (4 * 128);
}

bool Preset::from_edit_buffer() const {
  return id_ == kPresetIdBuffer;
}

void Preset::SetAsEditBuffer() {
  ASSERT(valid());
  ASSERT(!is_global_setting());
  if (is_global_setting() || !valid())
    return;
  id_ = kPresetIdBuffer;
}

BlockParameters* Preset::LookupBlock(AxeFxIIBlockID block) {
  ASSERT(params_.empty());
  // TODO: Use a map for lookups?
  for (const auto& p: block_parameters_) {
    if (p->block() == block)
      return p.get();
  }
  return nullptr;
}

bool Preset::SetPresetId(const PresetIdHeader& header, size_t size) {
  ASSERT(header.function() == PRESET_ID);
  ASSERT(size == sizeof(header));
  ASSERT(header.unknown.As16bit() == 0x10);  // <- not sure what this is.

  id_ = header.preset_number.As16bit();
  if (header.preset_number.ms == 0x7f && header.preset_number.ls == 0x0) {
    // This is a special case that means the preset is destined for (or comes
    // from) the edit buffer.  In this case, we just set the id to -1.
    // http://forum.fractalaudio.com/axe-fx-ii-discussion/58581-help-loading-presets-using-sysex-librarian.html#post732659
    ASSERT(id_ == kPresetIdBuffer);
  }

  return valid();
}

bool Preset::AddParameterData(const ParameterBlockHeader& header, size_t size) {
  ASSERT(valid());
  bool ret = params_.AppendFromSysEx(header, size);
  if (!ret) {
    id_ = kInvalidPresetId;
    ASSERT(false);
  }
  return ret;
}

bool Preset::Finalize(const PresetChecksumHeader* header, size_t size,
                      bool verify_only) {
  ASSERT(valid());
  // Support for skipping checksum checks is here because a preset dump
  // might not have a parameter checksum for some reason.  Possibly this
  // is simply a bug in the AxeFx when realtime sysex sending is set to "All".
  if (header) {
    if (size != sizeof(PresetChecksumHeader) ||
        header->checksum.As16bit() != params_.Checksum()) {
      return false;
    }
  }

  PresetParameters::const_iterator p = params_.begin();

  if (is_global_setting()) {
    // For system backups, we treat each preset block as an opaque block of
    // data by default.
    name_ = "(global system data)";
    return true;
  }

  uint16_t version = *p;
  // 0 == 516 for fw9 and higher. 514 for older.
  if (version != kCurrentParameterVersion && version != 0x202) {
    std::cerr << "Unsupported syx file - 0x" << std::hex << version << std::dec
              << std::endl;
    return false;
  }

  // Parse the preset name (values 2-32).
  if (p[1] != 0) {
    // Not sure what this is.  When not 0, the preset seems to be
    // encoded/compressed somehow. I pinged Fractal support about this.
    // Let's see what they say.
    return verify_only;
  }

  p += 2;
  name_.assign(p, p + 31);
  std::string::size_type index = name_.length() - 1;
  while (index > 0 && (name_[index] == ' ' || name_[index] == '\0'))
    --index;  
  name_.resize(index + 1);
  p += 31;
  ++p;  // zero terminator.

  // Save the effect block matrix.
  static_assert(sizeof(matrix_[0][0]) == sizeof(*p) * 2,
                "matrix size mismatch");
  memcpy(&matrix_[0][0], &(*p), sizeof(matrix_));
  p += sizeof(matrix_) / sizeof(*p);

  // Parse per block parameters (including modifiers).
  while (p < params_.end() && *p) {
    unique_ptr<BlockParameters> block_params(new BlockParameters());
    size_t values_eaten = block_params->Initialize(&(*p), params_.end() - p);
    if (!values_eaten)
      return false;
    block_parameters_.push_back(std::move(block_params));
    p += values_eaten;
  }

  // Free some memory since we don't need it anymore.
  params_.clear();

  return true;
}

void Preset::ToJson(Json::Value* out) const {
  Json::Value& j = *out;
  if (from_edit_buffer()) {
    j["id"] = Json::Value();
  } else {
    j["id"] = id_;
  }
  j["name"] = name_;

  if (is_global_setting() || !params_.empty()) {
    // TODO: Support at least user cabs and the 0x1234 "preset".
    return;
  }

  Json::Value matrix;
  for (size_t y = 0; y < kMatrixRows; ++y) {
    Json::Value row;
    for (size_t x = 0; x < kMatrixColumns; ++x) {
      const BlockInMatrix& b = matrix_[x][y];
      Json::Value block;
      b.ToJson(&block);
      row.append(block);
    }
    matrix["row" + std::to_string(y)] = row;
  }

  j["matrix"] = matrix;

  Json::Value block_params;
  for (const auto& p: block_parameters_) {
    Json::Value params;
    p->ToJson(&params);
    block_params.append(params);
  }

  j["block_params"] = block_params;
}

bool Preset::Serialize(const SysExCallback& callback) const {
  ASSERT(valid());

  WriteHeader(callback);

  PresetParameters params;
  FillParameters(&params);
  ASSERT(params.size() == 2048);
  params.Serialize(callback);

  WriteChecksum(params.Checksum(), callback);

  return true;
}

void Preset::WriteHeader(const SysExCallback& callback) const {
  std::vector<uint8_t> data;
  data.resize(sizeof(PresetIdHeader));
  new (&data[0]) PresetIdHeader(static_cast<uint16_t>(id_));
  callback(data);
}

void Preset::FillParameters(PresetParameters* params) const {
  PresetParameters& p = *params;
  if (is_global_setting() || !params_.empty()) {
    // If we get here for non-global settings, we haven't parsed the parameters
    // and therefore we don't support modifying them (including the preset
    // name).  So, let's copy the original parameters over directly.
    p = params_;
    return;
  }

  // Param block size is fixed at 2048.
  p.assign(2048, 0);

  size_t pos = 0;
  p[pos++] = kCurrentParameterVersion;
  pos++;  // Unknown value.
  for (size_t i = 0; i < 31; ++i)
    p[pos++] = (i < name_.length()) ? name_[i] : ' ';
  ++pos;  // zero terminator.

  // Copy the matrix.
  memcpy(&p[pos], &matrix_[0][0], sizeof(matrix_));
  pos += sizeof(matrix_) / sizeof(p[0]);

  for (const auto& b: block_parameters_) {
    size_t values = b->Write(&p[pos], p.size() - pos);
    pos += values;
  }
}

void Preset::WriteChecksum(uint16_t checksum,
                           const SysExCallback& callback) const {
  std::vector<uint8_t> data;
  data.resize(sizeof(PresetChecksumHeader));
  new (&data[0]) PresetChecksumHeader(static_cast<uint16_t>(checksum));
  callback(data);
}

}  // namespace axefx
