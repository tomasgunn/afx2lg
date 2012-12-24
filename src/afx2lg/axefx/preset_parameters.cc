// Copyright (c) 2012, Tomas Gunnarsson
// All rights reserved.

#include "axefx/preset_parameters.h"

#include "axefx/sysex_types.h"

static const size_t kValuesPerHeader = 64u;

namespace axefx {

PresetParameters::PresetParameters() {}
PresetParameters::~PresetParameters() {}

bool PresetParameters::AppendFromSysEx(const ParameterBlockHeader& header,
                                       size_t header_size) {
  ASSERT(header.function() == PRESET_PARAMETERS);
  size_t expected_size = sizeof(header) +
      ((header.value_count - 1)* sizeof(header.values[0])) +
      kSysExTerminationByteCount;
  if (header_size != expected_size) {
    ASSERT(false);
    return false;
  }
  ASSERT(header.value_count == kValuesPerHeader);
  ASSERT(header.values[header.value_count].b2 == kSysExEnd);
  reserve(size() + header.value_count);
  for (int i = 0; i < header.value_count; ++i)
    push_back(header.values[i].As16bit());
  return true;
}

uint16_t PresetParameters::Checksum() const {
  uint16_t checksum = 0;
  for (const_iterator it = begin(); it != end(); ++it)
    checksum ^= *it;
  return checksum;
}

bool PresetParameters::Serialize(const SysExCallback& callback) const {
  ASSERT(!empty());

  std::vector<uint8_t> data;
  data.resize(
      sizeof(ParameterBlockHeader) +
      (sizeof(Fractal16bit) * (kValuesPerHeader - 1)) +
      sizeof(FractalSysExEnd));
  auto* header = reinterpret_cast<ParameterBlockHeader*>(&data[0]);
  header->ParameterBlockHeader::ParameterBlockHeader(kValuesPerHeader);

  size_t value_index = 0;
  for (size_t i = 0; i < size(); ++i) {
    value_index = i % kValuesPerHeader;
    header->values[value_index].From16bit(at(i));
    if (value_index == (kValuesPerHeader - 1)) {
      auto checksum = reinterpret_cast<FractalSysExEnd*>(
          &header->values[value_index + 1]);
      checksum->FractalSysExEnd::FractalSysExEnd();
      checksum->CalculateChecksum(header);
      callback(data);
      memset(&header->values[0], 0,
             kValuesPerHeader * sizeof(header->values[0]));
    }
  }

  if (value_index != (kValuesPerHeader - 1)) {
    // TODO(tommi): Does this ever happen in practice?
    auto checksum = reinterpret_cast<FractalSysExEnd*>(
        &header->values[value_index + 1]);
    checksum->FractalSysExEnd::FractalSysExEnd();
    checksum->CalculateChecksum(header);
    callback(data);
  }

  return true;
}

}  // namespace axefx
