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
const uint16_t kEditBufferId = (0x7F << 7u);  // Used for presets and IR data.

bool IsFractalSysEx(const uint8_t* sys_ex, size_t size);
bool IsFractalSysExNoChecksum(const uint8_t* sys_ex, size_t size);

// http://wiki.fractalaudio.com/axefx2/index.php?title=MIDI_SysEx
uint8_t CalculateSysExChecksum(const uint8_t* sys_ex, size_t size);

enum AxeFxModel {
  // I've seen 0 being set for .syx files that contain only IR data.
  AXE_FX_STANDARD = 0x01,  // or 0?
  AXE_FX_ULTRA = 0x02,
  AXE_FX_II = 0x03,
};

enum FunctionId {
  // To request a dump of the current preset, send REQUEST_PRESET_DUMP
  // and set id to 7f 00 (edit buffer id).
  REQUEST_PRESET_DUMP = 0x03,  // Includes SeptedPair == preset id.
  TUNER_DATA = 0x0D,  // Includes SeptetPair.
  PRESET_NAME = 0x0F,
  TEMPO_HEARTBEAT = 0x10,
  PRESET_CHANGE = 0x14,  // Includes SeptedPair == preset id.
  BANK_DUMP_REQUEST = 0x1C,
  PARAMETER_CHANGED = 0x21,  // Just a notification. Use GenericNoDataMessage.
  PRESET_ID = 0x77,  // Use PresetIdHeader.
  PRESET_PARAMETERS = 0x78,  // Use ParameterBlockHeader.
  PRESET_CHECKSUM = 0x79,  // Use PresetChecksumHeader.
  IR_BEGIN = 0x7A,
  IR_DATA = 0x7B,
  IR_END = 0x7C,
};

#pragma pack(push)
#pragma pack(1)

// Convert two septets (7bit integers), into a 16 bit integer.
struct SeptetPair {
  SeptetPair() {}
  SeptetPair(uint16_t value);

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

  uint16_t Decode() const;
  void Encode(uint16_t value);
};

// IR data is 32 bit where each 32bit chunk is encoded in 5 bytes.
// http://wiki.fractalaudio.com/axefx2/index.php?title=MIDI_SysEx#Obtaining_Parameter_Values_via_SYSEX_Messages
struct Fractal32bit {
  uint8_t b1, b2, b3, b4, b5;

  uint32_t Decode() const;
  void Encode(uint32_t value);
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

template<FunctionId func, typename UnknownType>
struct IdHeader : public FractalSysExHeader {
  IdHeader() : FractalSysExHeader(func) {}

  IdHeader(uint16_t entry_id)
      : FractalSysExHeader(func),
        id(entry_id),
        unknown(0x10) {
    end.CalculateChecksum(this);
  }

  SeptetPair id;
  UnknownType unknown;  // always 0x10
  FractalSysExEnd end;
};

typedef IdHeader<PRESET_ID, SeptetPair> PresetIdHeader;
typedef IdHeader<IR_BEGIN, uint8_t> IRIdHeader;

// Common template class for header types that contain 7bit encoded data.
template<typename EncodingType, FunctionId func>
struct DataBlockHeader : public FractalSysExHeader {
  DataBlockHeader(uint8_t value_count)
      : FractalSysExHeader(func),
        value_count(value_count),
        reserved(0u) {
  }

  uint8_t value_count;     // Typically 0x40 (16bit) or 0x20 (32bit).
  uint8_t reserved;        // Always 0.
  EncodingType values[1];  // Actual size is |value_count|.
};

typedef DataBlockHeader<Fractal16bit, PRESET_PARAMETERS> ParameterBlockHeader;
typedef DataBlockHeader<Fractal32bit, IR_DATA> IRBlockHeader;

template<typename EncodingType, typename int_type, FunctionId func>
struct ChecksumHeader : public FractalSysExHeader {
  ChecksumHeader() : FractalSysExHeader(func) {}
  ChecksumHeader(int_type sum) : FractalSysExHeader(func) {
    checksum.Encode(sum);
    end.CalculateChecksum(this);
  }
  EncodingType checksum;
  FractalSysExEnd end;
};

typedef ChecksumHeader<Fractal16bit, uint16_t, PRESET_CHECKSUM>
    PresetChecksumHeader;
typedef ChecksumHeader<Fractal32bit, uint32_t, IR_END> IRChecksumHeader;

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

struct GenericNoDataMessage : public FractalSysExHeader {
  GenericNoDataMessage(FunctionId id)
      : FractalSysExHeader(id) {
    end.CalculateChecksum(this);
  }
  FractalSysExEnd end;
};

struct PresetDumpRequest : public FractalSysExHeader {
  PresetDumpRequest(uint16_t preset_id)
      : FractalSysExHeader(REQUEST_PRESET_DUMP),
        preset_id_(preset_id) {
    end.CalculateChecksum(this);
  }

  // The default constructor sets the preset id to the edit buffer id.
  PresetDumpRequest() : FractalSysExHeader(REQUEST_PRESET_DUMP) {
    preset_id_.ms = 0x7F;
    preset_id_.ls = 0;
    end.CalculateChecksum(this);
  }

  SeptetPair preset_id_;
  FractalSysExEnd end;
};

#pragma pack(pop)

}  // namespace axefx

#endif  // AXE_FX_SYSEX_TYPES_H_
