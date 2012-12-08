// Copyright (c) 2012, Tomas Gunnarsson
// All rights reserved.

#pragma once
#ifndef AXE_FX_SYSEX_TYPES_H_
#define AXE_FX_SYSEX_TYPES_H_

#include "common/common_types.h"

#include <string>

namespace axefx {

const uint8_t kSysExStart = 0xF0;
const uint8_t kSysExEnd = 0xF7;
const int kSysExTerminationByteCount = 2;  // checksum + kSysExEnd == 2 bytes.
const uint8_t kFractalMidiId[] = { 0x00, 0x01, 0x74 };

bool IsFractalSysEx(const uint8_t* sys_ex, int size);
bool IsFractalSysExNoChecksum(const uint8_t* sys_ex, int size);

// http://wiki.fractalaudio.com/axefx2/index.php?title=MIDI_SysEx
uint8_t CalculateSysExChecksum(const uint8_t* sys_ex, int size);

enum AxeFxModel {
  AXE_FX_STANDARD = 0x01,  // or 0?
  AXE_FX_ULTRA = 0x02,
  AXE_FX_II = 0x03,
};

enum FunctionId {
  TEMPO_HEARTBEAT = 0x10,
  BANK_DUMP_REQUEST = 0x1C,
  PRESET_ID = 0x77,  // Use PresetIdHeader.
  PRESET_PARAMETERS = 0x78,  // Use ParameterBlockHeader.
  PRESET_CHECKSUM = 0x79,  // Use PresetChecksumHeader.
};

#pragma pack(push)
#pragma pack(1)

// Convert two septets (7bit integers), into a 16 bit integer.
struct SeptetPair {
  uint8_t ms, ls;

  uint16_t As16bit() const;
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
  // Initializes default values.
  explicit FractalSysExHeader(FunctionId func);

  uint8_t sys_ex_start;  // kSysExStart.
  uint8_t manufacturer_id[arraysize(kFractalMidiId)];  // kFractalMidiId.
  uint8_t model_id;  // AxeFxModel
  uint8_t function_id;

  AxeFxModel model() const;
  FunctionId function() const;
};

struct FractalSysExEnd {
  FractalSysExEnd() : checksum(0), sys_ex_end(kSysExEnd) {}

  void CalculateChecksum(const FractalSysExHeader* start);

  uint8_t checksum;
  uint8_t sys_ex_end;  // kSysExend.
};

struct PresetIdHeader : public FractalSysExHeader {
  PresetIdHeader() : FractalSysExHeader(PRESET_ID) {}
  SeptetPair preset_number;
  SeptetPair unknown;
};

struct ParameterBlockHeader : public FractalSysExHeader {
  ParameterBlockHeader() : FractalSysExHeader(PRESET_PARAMETERS) {}
  uint8_t value_count;  // I've only ever seen this be 0x40.
  uint8_t reserved;  // Always 0.
  Fractal16bit values[1];  // Actual size is |parameter_count|.
};

struct PresetChecksumHeader : public FractalSysExHeader {
  PresetChecksumHeader() : FractalSysExHeader(PRESET_CHECKSUM) {}
  Fractal16bit checksum;
};

struct BankDumpRequest : public FractalSysExHeader {
  enum BankId {
    BANK_A = 0,
    BANK_B,
    BANK_C,
    SYSTEM_BANK,
  };

  BankDumpRequest(BankId id)
      : FractalSysExHeader(BANK_DUMP_REQUEST),
        bank_id(static_cast<uint8_t>(id)) {
    end.CalculateChecksum(this);
  }

  uint8_t bank_id;
  FractalSysExEnd end;
};

#pragma pack(pop)

}  // namespace axefx

#endif  // AXE_FX_SYSEX_TYPES_H_
