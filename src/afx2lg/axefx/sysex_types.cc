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
  return CalculateChecksum(&sys_ex[0],
      &sys_ex[size - sizeof(FractalSysExEnd)]) & 0x7F;
}

SeptetPair::SeptetPair(uint16_t value)
    : ms(static_cast<uint8_t>(value >> 7)),
      ls(static_cast<uint8_t>(value & 0x7F)) {
  ASSERT((value >> 14) == 0);
}

uint16_t SeptetPair::As16bit() const {
  ASSERT((ms & 0x80) == 0);
  ASSERT((ls & 0x80) == 0);
  return (static_cast<uint16_t>(ls) | (static_cast<uint16_t>(ms) << 7));
}

uint16_t Fractal16bit::Decode() const {
  uint16_t ret = (b3 & 0x3) << 14;
  ret |= (b2 & 0x7F) << 7;
  ret |= (b1 & 0x7F);
  return ret;
}

void Fractal16bit::Encode(uint16_t value) {
  b1 = (value & 0x7F);
  b2 = ((value >> 7) & 0x7F);
  b3 = ((value >> 14) & 0x7F);
}

uint32_t Fractal32bit::Decode() const {
  return static_cast<uint32_t>(b1) << 24 |
      (static_cast<uint32_t>(b2) & 1) << 31 |
      (static_cast<uint32_t>(b2) >> 1) << 16 |
      (static_cast<uint32_t>(b3) & 3) << 22 |
      (static_cast<uint32_t>(b3) >> 2) << 8 |
      (static_cast<uint32_t>(b4) & 7) << 13 |
      (static_cast<uint32_t>(b4) >> 3) |
      (static_cast<uint32_t>(b5) & 0xF) << 4;
}

void Fractal32bit::Encode(uint32_t value) {
  const uint8_t bytes[4] = {
    static_cast<uint8_t>((value) & 0xFF),
    static_cast<uint8_t>((value >> 8) & 0xFF),
    static_cast<uint8_t>((value >> 16) & 0xFF),
    static_cast<uint8_t>((value >> 24) & 0xFF),
  };
  b1 = bytes[3] & 0x7F;
  b2 = (bytes[3] >> 7) | ((bytes[2] & 0x3F) << 1);
  b3 = (bytes[2] >> 6) | ((bytes[1] & 0x1F) << 2);
  b4 = (bytes[1] >> 5) | ((bytes[0] & 0x0F) << 3);
  b5 = (bytes[0] >> 4);
}

uint32_t Fractal28bit::Decode() const {
  uint32_t ret =
      (b1 & 0x7F) |
      (b2 & 0x7F) << 7 |
      (b3 & 0x7F) << 14 |
      (b4 & 0x7F) << 21;
  ASSERT((ret & 0xF0000000) == 0);
  return ret;
}

void Fractal28bit::Encode(uint32_t value) {
  ASSERT((value & 0xF0000000) == 0);
  b1 = (value & 0x7F);
  b2 = ((value >> 7) & 0x7F);
  b3 = ((value >> 14) & 0x7F);
  b4 = ((value >> 21) & 0x7F);
}

uint16_t Fractal14bit::Decode() const {
  uint16_t ret = (b1 & 0x7F) | (b2 & 0x7F) << 7;
  ASSERT((ret & 0xC000) == 0);
  return ret;
}

void Fractal14bit::Encode(uint16_t value) {
  ASSERT((value & 0xC000) == 0);
  b1 = (value & 0x7F);
  b2 = ((value >> 7) & 0x7F);
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

bool FractalSysExEnd::VerifyChecksum(const FractalSysExHeader* start) const {
  const uint8_t* sys_ex = reinterpret_cast<const uint8_t*>(start);
  size_t size = (&sys_ex_end - sys_ex) + 1;
  return checksum == CalculateSysExChecksum(sys_ex, size);
}

}  // namespace axefx
