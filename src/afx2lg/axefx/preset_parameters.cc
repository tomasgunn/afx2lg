// Copyright (c) 2012, Tomas Gunnarsson
// All rights reserved.

#include "axefx/preset_parameters.h"

static const size_t kParamValuesPerHeader = 128u / sizeof(uint16_t);

namespace axefx {

PresetParameters::PresetParameters() {}
PresetParameters::~PresetParameters() {}

bool PresetParameters::AppendFromSysEx(const ParameterBlockHeader& header,
                                       size_t header_size) {
  ASSERT(header.function() == PRESET_PARAMETERS);
  size_t expected_size = sizeof(header) +
      ((header.value_count - 1) * sizeof(header.values[0])) +
      kSysExTerminationByteCount;
  if (header_size != expected_size) {
    ASSERT(false);
    return false;
  }
  ASSERT(header.value_count == kParamValuesPerHeader);
  ASSERT(header.values[header.value_count].b2 == kSysExEnd);
  reserve(size() + header.value_count);
  for (int i = 0; i < header.value_count; ++i)
    push_back(header.values[i].Decode());
  return true;
}

uint16_t PresetParameters::Checksum() const {
  return CalculateChecksum(*this);
}

// TODO: Combine this implementation with the IR and Firmware implementations.
bool PresetParameters::Serialize(const SysExCallback& callback) const {
  ASSERT(!empty());

  std::vector<uint8_t> data;
  data.resize(
      sizeof(ParameterBlockHeader) +
      (sizeof(Fractal16bit) * (kParamValuesPerHeader - 1)) +
      sizeof(FractalSysExEnd));
  auto header = new (&data[0]) ParameterBlockHeader(kParamValuesPerHeader);

  size_t value_index = 0;
  for (size_t i = 0; i < size(); ++i) {
    value_index = i % kParamValuesPerHeader;
    header->values[value_index].Encode(at(i));
    if (value_index == (kParamValuesPerHeader - 1)) {
      auto checksum = new (&header->values[value_index + 1]) FractalSysExEnd();
      checksum->CalculateChecksum(header);
      callback(data);
      memset(&header->values[0], 0,
             kParamValuesPerHeader * sizeof(header->values[0]));
    }
  }

  if (value_index != (kParamValuesPerHeader - 1)) {
    // TODO(tommi): Does this ever happen in practice?
    auto checksum = new (&header->values[value_index + 1]) FractalSysExEnd();
    checksum->CalculateChecksum(header);
    callback(data);
  }

  return true;
}

}  // namespace axefx
