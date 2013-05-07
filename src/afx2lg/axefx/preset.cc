// Copyright (c) 2012, Tomas Gunnarsson
// All rights reserved.

#include "axefx/preset.h"

#include "axefx/sysex_types.h"
#include "bcl/overrides/src/huffman.h"
#include "json/value.h"

#include <algorithm>
#include <iostream>

namespace axefx {

const int kInvalidPresetId = -1;
const uint16_t kCurrentParameterVersion = 0x0206;

namespace {

bool IsVersionSupported(uint16_t version) {
  // Versions 0x01nn seem to be using the same format, so we optimistically
  // allow them to pass.  We also assume that all version with major version
  // 0x02 will have the same format.
  return version >= 0x0100 && version < 0x0300;
}

}  // namespace

Preset::Preset() : version_(kCurrentParameterVersion), id_(kInvalidPresetId) {}
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
  return id_ == static_cast<int>(kEditBufferId);
}

void Preset::SetAsEditBuffer() {
  ASSERT(valid());
  ASSERT(!is_global_setting());
  if (is_global_setting() || !valid())
    return;
  id_ = static_cast<uint16_t>(kEditBufferId);
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

  id_ = header.id.As16bit();
  if (header.id.ms == 0x7f && header.id.ls == 0x0) {
    // This is a special case that means the preset is destined for (or comes
    // from) the edit buffer.
    // http://forum.fractalaudio.com/axe-fx-ii-discussion/58581-help-loading-presets-using-sysex-librarian.html#post732659
    ASSERT(id_ == static_cast<uint16_t>(kEditBufferId));
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
        header->checksum.Decode() != params_.Checksum()) {
      return false;
    }
  }

  PresetParameters::iterator p = params_.begin();

  if (is_global_setting()) {
    // For system backups, we treat each preset block as an opaque block of
    // data by default.
    name_ = "(global system data)";
    return true;
  }

  version_ = *p;
  if (!IsVersionSupported(version_)) {
    std::cerr << "Unsupported syx version - " << version_ << std::endl;
    return false;
  }

  // After the revision number comes the number of compressed bytes.
  // Usually this will be 0, but for presets that use the Tone Match block,
  // this will ne non-zero.
  uint16_t compressed_bytes = p[1];

  // Parse the preset name (values 2-32).
  p += 2;
  name_.assign(p, p + 31);
  std::string::size_type index = name_.length() - 1;
  while (index > 0 && (name_[index] == ' ' || name_[index] == '\0'))
    --index;  
  name_.resize(index + 1);
  p += 31;
  ASSERT(p[0] == 0);  // zero terminator.
  ++p;

  if (compressed_bytes != 0) {
    // In this case, the last 1024 16bit values in params_, contain the tone
    // match IR data.  Let's chop that off and save it.
    ir_data_.assign(params_.end() - 1024, params_.end());
    ASSERT(ir_data_.size() == 1024);
    params_.erase(params_.end() - 1024, params_.end());

    // The compression seems to assume that the bytes are ordered in a little
    // endian 16 bit fashion - which is what we already have - so no conversion
    // to network byte order (big endian) is necessary.

    // TODO: Come up with a better heuristic for picking a good buffer size
    // and truncating the buffer.
    std::vector<uint16_t> decompressed;
    decompressed.resize((compressed_bytes / sizeof(p[0])) * 10, 0xBADF);

    size_t uncompressed_size = Huffman_Uncompress(
        reinterpret_cast<uint8_t*>(&p[0]),
        reinterpret_cast<uint8_t*>(&decompressed[0]),
        compressed_bytes,
        decompressed.size() * sizeof(decompressed[0]));

    decompressed.resize(uncompressed_size / sizeof(decompressed[0]));

    p = params_.erase(p, p + (compressed_bytes / sizeof(p[0])));
    p = params_.insert(p, decompressed.begin(), decompressed.end());
  }

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
  // TODO: Configure a struct for the version, compressed_size and name values.
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
  p[pos++] = version_;
  uint16_t& compressed_size = p[pos++];

  for (size_t i = 0; i < 31; ++i)
    p[pos++] = (i < name_.length()) ? name_[i] : ' ';
  ++pos;  // zero terminator.

  // Copy the matrix.
  size_t matrix_begins = pos;
  memcpy(&p[pos], &matrix_[0][0], sizeof(matrix_));
  pos += sizeof(matrix_) / sizeof(p[0]);

  for (const auto& b: block_parameters_) {
    size_t values = b->Write(&p[pos], p.size() - pos);
    pos += values;
  }

  if (!ir_data_.empty()) {
    // Compress the parameters.
    std::vector<uint16_t> compressed;
    // Assume the compressed size will be smaller or equal to the data being
    // compressed.
    compressed.resize((pos + 1) - matrix_begins, 0);
    compressed_size = static_cast<uint16_t>(Huffman_Compress(
        reinterpret_cast<uint8_t*>(&p[matrix_begins]),
        reinterpret_cast<uint8_t*>(&compressed[0]),
        compressed.size() * sizeof(p[0])));
    compressed.resize(compressed_size / sizeof(compressed[0]));

    // Overwrite the parameters etc with the compressed equivalent.
    std::copy(compressed.begin(), compressed.end(), p.begin() + matrix_begins);

    // Fill the remains between the compressed data and the IR data with 0's.
    ASSERT(&p[matrix_begins + compressed.size()] <= &p[p.size() - 1024]);
    std::fill(&p[matrix_begins + compressed.size()], &p[p.size() - 1024], 0);

    // Copy uncompressed ir_data_ to the last 1024 words of p.
    ASSERT(ir_data_.size() == 1024);
    std::copy(ir_data_.begin(), ir_data_.end(), p.end() - 1024);
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
