// Copyright (c) 2012, Tomas Gunnarsson
// All rights reserved.

#pragma once
#ifndef AXE_FX_SYSEX_TYPES_H_
#define AXE_FX_SYSEX_TYPES_H_

#include "common_types.h"
#include "axefx/axe_fx_sysex_parser.h"

namespace axefx {

const uint8_t kSysExStart = 0xF0;
const uint8_t kSysExEnd = 0xF7;
const int kSysExTerminationByteCount = 2;  // checksum + kSysExEnd == 2 bytes.
const uint8_t kFractalMidiId[] = { 0x00, 0x01, 0x74 };

// http://wiki.fractalaudio.com/axefx2/index.php?title=MIDI_SysEx
uint8_t CalculateChecksum(const uint8_t* sys_ex, int size);
bool VerifyChecksum(const uint8_t* sys_ex, int size);

enum AxeFxModel {
  AXE_FX_STANDARD = 0x01,  // or 0?
  AXE_FX_ULTRA = 0x02,
  AXE_FX_II = 0x03,
};

enum FunctionId {
  PRESET_ID = 0x77,
  PRESET_PROPERTY = 0x78,
  PRESET_EPILOGUE = 0x79,
};

#pragma pack(push)
#pragma pack(1)

// Convert two septets (7bit integers), into a 16 bit integer.
struct SeptetPair {
  uint8_t ms, ls;

  uint16_t As16bit() const;
};

// This could be a wide char stored in 3 bytes, but we treat it as
// ASCII for now.
struct SeptetChar {
  uint8_t b1, b2, b3;
  char AsChar() const;
};

// AxeFx-II uses three bytes for 16bit values.  The bytes are stored in
// reverse order as documented here:
// http://wiki.fractalaudio.com/axefx2/index.php?title=MIDI_SysEx
// This is different from how things were done on the Ultra and Standard
// models where there were only two bytes and the  most significant one
// came first.
struct Fractal16bit {
  uint8_t b1, b2, b3;

  uint16_t As16bit() const;
};

struct FractalSysExHeader {
  uint8_t sys_ex_start;  // kSysExStart.
  uint8_t manufacture_id[arraysize(kFractalMidiId)];  // kFractalMidiId.
  uint8_t model_id;  // kAxeFx2ModelId
  uint8_t function_id;

  AxeFxModel model() const;
  FunctionId function() const;
};

struct PresetIdHeader : public FractalSysExHeader {
  SeptetPair preset_number;
  SeptetPair unknown;
};

struct PresetProperty : public FractalSysExHeader {
  const uint8_t payload[1];
};

struct PresetName {
  // According to the manual the preset name can be as long as 23 characters.
  // In sysex files I've seen, the name is padded with space but appears
  // to be able to be as long as 31 chars + zero terminator.
  SeptetChar chars[23];

  std::string ToString() const;
};

struct BlockState {
  Fractal16bit block_id;
  Fractal16bit state1;  // usually 2 :-/
  Fractal16bit state2;
  Fractal16bit state3;
  Fractal16bit state4;
  Fractal16bit state5;
  Fractal16bit state6;
  Fractal16bit state7;
};

#pragma pack(pop)

}  // namespace axefx

#endif  // AXE_FX_SYSEX_TYPES_H_
