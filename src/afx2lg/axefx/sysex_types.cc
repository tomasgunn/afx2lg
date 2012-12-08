// Copyright (c) 2012, Tomas Gunnarsson
// All rights reserved.

#include "axefx/sysex_types.h"

namespace axefx {

// http://wiki.fractalaudio.com/axefx2/index.php?title=MIDI_SysEx
bool VerifySysExChecksum(const uint8_t* sys_ex, size_t size) {
  return size >= 3 &&
         sys_ex[0] == kSysExStart &&
         sys_ex[size - 1] == kSysExEnd &&
         sys_ex[size - 2] == CalculateSysExChecksum(sys_ex, size);
}

bool IsFractalSysEx(const uint8_t* sys_ex, size_t size) {
  if (!IsFractalSysExNoChecksum(sys_ex, size))
    return false;

  return VerifySysExChecksum(sys_ex, size);
}

bool IsFractalSysExNoChecksum(const uint8_t* sys_ex, size_t size) {
  ASSERT(sys_ex[0] == kSysExStart);
  ASSERT(sys_ex[size - 1] == kSysExEnd);

  if (size < (sizeof(kFractalMidiId) + kSysExTerminationByteCount) ||
      memcmp(&sys_ex[1], &kFractalMidiId[0], sizeof(kFractalMidiId)) != 0) {
    return false;
  }

  return true;
}

uint8_t CalculateSysExChecksum(const uint8_t* sys_ex, size_t size) {
  uint8_t checksum = 0;
  size_t i = 0;
  for (; i < size - 2; ++i)
    checksum ^= sys_ex[i];
  checksum &= 0x7F;
  return checksum;
}

uint16_t SeptetPair::As16bit() const {
  ASSERT((ms & 0x80) == 0);
  ASSERT((ls & 0x80) == 0);
  return (static_cast<uint16_t>(ls) | (static_cast<uint16_t>(ms) << 7));
}

uint16_t Fractal16bit::As16bit() const {
  uint16_t ret = (b3 & 0x3) << 14;
  ret |= ((b2 & 0x7f) >> 1) << 8;
  ret |= (b2 & 0x1) << 7;
  ret |= (b1 & 0x7F);
  return ret;
}

FractalSysExHeader::FractalSysExHeader(FunctionId func)
    : sys_ex_start(kSysExStart),
      model_id(static_cast<uint8_t>(AXE_FX_II)),
      function_id(static_cast<uint8_t>(func)) {
  static_assert(sizeof(manufacturer_id) == sizeof(kFractalMidiId),
                "Size mismatch");
  memcpy(&manufacturer_id, &kFractalMidiId[0], sizeof(kFractalMidiId));
}

AxeFxModel FractalSysExHeader::model() const {
  return static_cast<AxeFxModel>(model_id);
}

FunctionId FractalSysExHeader::function() const {
  return static_cast<FunctionId>(function_id);
}

void FractalSysExEnd::CalculateChecksum(const FractalSysExHeader* start) {
  const uint8_t* sys_ex = reinterpret_cast<const uint8_t*>(start);
  size_t size = (&sys_ex_end - sys_ex) + 1;
  checksum = CalculateSysExChecksum(sys_ex, size);
}

}  // namespace axefx
