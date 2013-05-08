// Copyright (c) 2012, Tomas Gunnarsson
// All rights reserved.

#include "axefx/ir_data.h"

static const size_t kIRValuesPerHeader = 128u / sizeof(uint32_t);

namespace axefx {

IRData::IRData() : id_(kEditBufferId) {}

IRData::IRData(const IRIdHeader& header) : id_(header.id.As16bit()) {}

IRData::~IRData() {}

std::string IRData::name() const {
  std::string ret;
  if (data_.size() < 8)
    return ret;

  std::vector<uint32_t>::const_iterator i;
  for (i = data_.begin(); i != data_.end(); ++i) {
    if (!*i)
      break;

    ret.push_back(static_cast<char>(*i >> 24));
    ret.push_back(static_cast<char>(*i >> 16));
    ret.push_back(static_cast<char>(*i >> 8));
    ret.push_back(static_cast<char>(*i & 0xFF));

    if (ret.size() > 8 * sizeof(uint32_t))
      break;
  }

  return ret;
}

// TODO: Move all these checksum algorithms (8, 16 and 32 bit) to a
// common place.
uint32_t IRData::Checksum() const {
  uint32_t checksum = 0;
  std::vector<uint32_t>::const_iterator it;
  for (it = data_.begin(); it != data_.end(); ++it)
    checksum ^= *it;
  return checksum;
}

bool IRData::AppendFromSysEx(const IRBlockHeader& header, size_t header_size) {
  ASSERT(header.function() == IR_DATA);
  size_t expected_size = sizeof(header) +
      ((header.value_count - 1) * sizeof(header.values[0])) +
      kSysExTerminationByteCount;
  if (header_size != expected_size) {
    ASSERT(false);
    return false;
  }
  ASSERT(header.value_count == kIRValuesPerHeader);
  ASSERT(header.values[header.value_count].b2 == kSysExEnd);
  data_.reserve(data_.size() + header.value_count);
  for (int i = 0; i < header.value_count; ++i)
    data_.push_back(header.values[i].Decode());

  return true;
}

bool IRData::Serialize(const SysExCallback& callback) const {
  ASSERT(!data_.empty());

  // Write the ID.
  std::vector<uint8_t> data;
  data.resize(sizeof(IRIdHeader));
  new (&data[0]) IRIdHeader(static_cast<uint16_t>(id_));
  callback(data);

  // Write the data.
  data.resize(
      sizeof(IRBlockHeader) +
      (sizeof(Fractal32bit) * (kIRValuesPerHeader - 1)) +
      sizeof(FractalSysExEnd));
  auto header = new (&data[0]) IRBlockHeader(kIRValuesPerHeader);
  size_t value_index = 0;
  for (size_t i = 0; i < data_.size(); ++i) {
    value_index = i % kIRValuesPerHeader;
    header->values[value_index].Encode(data_[i]);
    if (value_index == (kIRValuesPerHeader - 1)) {
      auto checksum = new (&header->values[value_index + 1]) FractalSysExEnd();
      checksum->CalculateChecksum(header);
      callback(data);
      memset(&header->values[0], 0,
             kIRValuesPerHeader * sizeof(header->values[0]));
    }
  }

  if (value_index != (kIRValuesPerHeader - 1)) {
    // TODO: Does this ever happen in practice?
    ASSERT(false);
    auto checksum = new (&header->values[value_index + 1]) FractalSysExEnd();
    checksum->CalculateChecksum(header);
    callback(data);
  }

  // Write the Checksum.
  data.resize(sizeof(IRChecksumHeader));
  new (&data[0]) IRChecksumHeader(Checksum());
  callback(data);

  return true;
}

}  // namespace axefx
