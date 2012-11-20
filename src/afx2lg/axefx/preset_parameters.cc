// Copyright (c) 2012, Tomas Gunnarsson
// All rights reserved.

#include "axefx/preset_parameters.h"

#include "axefx/sysex_types.h"

namespace axefx {

PresetParameters::PresetParameters() {}
PresetParameters::~PresetParameters() {}

bool PresetParameters::AppendFromSysEx(const ParameterBlockHeader& header,
                                       int header_size) {
  ASSERT(header.function() == PRESET_PARAMETERS);
  int expected_size = sizeof(header) +
      ((header.value_count - 1)* sizeof(header.values[0])) +
      kSysExTerminationByteCount;
  if (header_size != expected_size) {
    ASSERT(false);
    return false;
  }
  ASSERT(header.value_count == 0x40);
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

}  // namespace axefx
