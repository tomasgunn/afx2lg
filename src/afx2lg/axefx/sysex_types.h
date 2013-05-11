// Copyright (c) 2012, Tomas Gunnarsson
// All rights reserved.

#pragma once
#ifndef AXE_FX_SYSEX_TYPES_H_
#define AXE_FX_SYSEX_TYPES_H_

#include "common/common_types.h"

#include <algorithm>
#include <string>
#include <vector>

namespace axefx {

const uint8_t kSysExStart = 0xF0;
const uint8_t kSysExEnd = 0xF7;
const int kSysExTerminationByteCount = 2;  // checksum + kSysExEnd == 2 bytes.
const uint8_t kFractalMidiId[] = { 0x00, 0x01, 0x74 };
const uint16_t kEditBufferId = (0x7F << 7u);  // Used for presets and IR data.

bool IsFractalSysEx(const uint8_t* sys_ex, size_t size);
bool IsFractalSysExNoChecksum(const uint8_t* sys_ex, size_t size);

template<typename T>
T CalculateChecksum(const T* begin, const T* end) {
  T checksum = 0;
  std::for_each(begin, end, [&checksum](const T& val) { checksum ^= val; });
  return checksum;
}

template<typename T>
T CalculateChecksum(const std::vector<T>& v) {
  return v.empty() ? 0 : CalculateChecksum(&v[0], &v[0] + v.size());
}

// http://wiki.fractalaudio.com/axefx2/index.php?title=MIDI_SysEx
uint8_t CalculateSysExChecksum(const uint8_t* sys_ex, size_t size);

enum AxeFxModel {
  // I've seen 0 being set for .syx files that contain only IR data.
  AXE_FX_STANDARD = 0x01,  // or 0?
  AXE_FX_ULTRA = 0x02,
  AXE_FX_II = 0x03,
};

enum FunctionId {
  INVALID_FUNCTION = 0xFF,
  // To request a dump of the current preset, send REQUEST_PRESET_DUMP
  // and set id to 7f 00 (edit buffer id).
  REQUEST_PRESET_DUMP = 0x03,  // Includes SeptedPair == preset id.
  TUNER_DATA = 0x0D,  // Includes SeptetPair.
  PRESET_NAME = 0x0F,
  TEMPO_HEARTBEAT = 0x10,
  PRESET_CHANGE = 0x14,  // Includes SeptedPair == preset id.
  BANK_DUMP_REQUEST = 0x1C,
  PARAMETER_CHANGED = 0x21,  // Just a notification. Use GenericNoDataMessage.
  FIRMWARE_UPDATE = 0x25,  // Switch to the fw update page.
  REPLY = 0x64,  // Reply/confirmation message from the AxeFx.
  PRESET_ID = 0x77,  // Use PresetIdHeader.
  PRESET_PARAMETERS = 0x78,  // Use ParameterBlockHeader.
  PRESET_CHECKSUM = 0x79,  // Use PresetChecksumHeader.
  IR_BEGIN = 0x7A,
  IR_DATA = 0x7B,
  IR_END = 0x7C,
  FIRMWARE_BEGIN = 0x7D,
  FIRMWARE_DATA = 0x7E,
  FIRMWARE_END = 0x7F,
};

#pragma pack(push)
#pragma pack(1)

// Convert two septets (7bit integers), into a 16 bit integer.
struct SeptetPair {
  SeptetPair() : ms(), ls() {}
  explicit SeptetPair(uint16_t value);

  uint8_t ms, ls;

  uint16_t As16bit() const;

 private:
  DISALLOW_COPY_AND_ASSIGN(SeptetPair);
};

// AxeFx-II uses three bytes for 16bit values.  The bytes are stored in
// reverse order as documented here:
// http://wiki.fractalaudio.com/axefx2/index.php?title=MIDI_SysEx
// This is different from how things were done on the Ultra and Standard
// models where there were only two bytes and the  most significant one
// came first.
struct Fractal16bit {
  Fractal16bit() : b1(), b2(), b3() {}

  uint8_t b1, b2, b3;

  uint16_t Decode() const;
  void Encode(uint16_t value);

 private:
  DISALLOW_COPY_AND_ASSIGN(Fractal16bit);
};

// IR data is 32 bit where each 32bit chunk is encoded in 5 bytes.
// http://wiki.fractalaudio.com/axefx2/index.php?title=MIDI_SysEx#Obtaining_Parameter_Values_via_SYSEX_Messages
struct Fractal32bit {
  Fractal32bit() : b1(), b2(), b3(), b4(), b5() {}
  explicit Fractal32bit(uint32_t value) { Encode(value); }

  uint8_t b1, b2, b3, b4, b5;

  uint32_t Decode() const;
  void Encode(uint32_t value);

 private:
  DISALLOW_COPY_AND_ASSIGN(Fractal32bit);
};

struct Fractal28bit {
  Fractal28bit() : b1(), b2(), b3(), b4() {}
  explicit Fractal28bit(uint32_t value) { Encode(value); }

  uint8_t b1, b2, b3, b4;

  uint32_t Decode() const;
  void Encode(uint32_t value);

 private:
  DISALLOW_COPY_AND_ASSIGN(Fractal28bit);
};

struct Fractal14bit {
  Fractal14bit() : b1(), b2() {}
  explicit Fractal14bit(uint16_t value) { Encode(value); }

  uint8_t b1, b2;

  uint16_t Decode() const;
  void Encode(uint16_t value);

 private:
  DISALLOW_COPY_AND_ASSIGN(Fractal14bit);
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

 private:
  DISALLOW_COPY_AND_ASSIGN(FractalSysExHeader);
};

struct FractalSysExEnd {
  FractalSysExEnd() : checksum(0), sys_ex_end(kSysExEnd) {}

  void CalculateChecksum(const FractalSysExHeader* start);
  bool VerifyChecksum(const FractalSysExHeader* start) const;

  uint8_t checksum;
  uint8_t sys_ex_end;  // kSysExend.

 private:
  DISALLOW_COPY_AND_ASSIGN(FractalSysExEnd);
};

template<FunctionId func, typename UnknownType>
struct IdHeader : public FractalSysExHeader {
  IdHeader() : FractalSysExHeader(func) {}

  explicit IdHeader(uint16_t entry_id)
      : FractalSysExHeader(func),
        id(entry_id),
        unknown(0x10) {
    end.CalculateChecksum(this);
  }

  SeptetPair id;
  UnknownType unknown;  // always 0x10
  FractalSysExEnd end;

 private:
  DISALLOW_COPY_AND_ASSIGN(IdHeader);
};

typedef IdHeader<PRESET_ID, SeptetPair> PresetIdHeader;
typedef IdHeader<IR_BEGIN, uint8_t> IRIdHeader;

// Common template class for header types that contain 7bit encoded data.
template<typename ValueType, FunctionId func>
struct DataBlockHeader : public FractalSysExHeader {
  explicit DataBlockHeader(uint8_t value_count)
      : FractalSysExHeader(func),
        value_count(value_count),
        reserved(0u) {
  }

  uint8_t value_count;  // Typically 0x40 (16bit) or 0x20 (32bit).
  uint8_t reserved;  // Always 0.
  ValueType values[1];  // Actual size is |value_count|.

 private:
  DISALLOW_COPY_AND_ASSIGN(DataBlockHeader);
};

typedef DataBlockHeader<Fractal16bit, PRESET_PARAMETERS> ParameterBlockHeader;
typedef DataBlockHeader<Fractal32bit, IR_DATA> IRBlockHeader;

template<typename ChecksumType, typename int_type, FunctionId func>
struct ChecksumHeader : public FractalSysExHeader {
  ChecksumHeader() : FractalSysExHeader(func) {}
  explicit ChecksumHeader(int_type sum) : FractalSysExHeader(func) {
    checksum.Encode(sum);
    end.CalculateChecksum(this);
  }

  int_type package_checksum() const {
    return checksum.Decode();
  }

  ChecksumType checksum;
  FractalSysExEnd end;

 private:
  DISALLOW_COPY_AND_ASSIGN(ChecksumHeader);
};

typedef ChecksumHeader<Fractal16bit, uint16_t, PRESET_CHECKSUM>
    PresetChecksumHeader;
typedef ChecksumHeader<Fractal32bit, uint32_t, IR_END> IRChecksumHeader;
typedef ChecksumHeader<Fractal32bit, uint32_t, FIRMWARE_END>
    FirmwareChecksumHeader;

struct BankDumpRequest : public FractalSysExHeader {
  enum BankId {
    BANK_A = 0,
    BANK_B,
    BANK_C,
    SYSTEM_BANK,
  };

  explicit BankDumpRequest(BankId id)
      : FractalSysExHeader(BANK_DUMP_REQUEST),
        bank_id(static_cast<uint8_t>(id)) {
    end.CalculateChecksum(this);
  }

  uint8_t bank_id;
  FractalSysExEnd end;

 private:
  DISALLOW_COPY_AND_ASSIGN(BankDumpRequest);
};

struct GenericNoDataMessage : public FractalSysExHeader {
  explicit GenericNoDataMessage(FunctionId id)
      : FractalSysExHeader(id) {
    end.CalculateChecksum(this);
  }
  FractalSysExEnd end;

 private:
  DISALLOW_COPY_AND_ASSIGN(GenericNoDataMessage);
};

struct PresetDumpRequest : public FractalSysExHeader {
  explicit PresetDumpRequest(uint16_t preset_id)
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

 private:
  DISALLOW_COPY_AND_ASSIGN(PresetDumpRequest);
};

struct FirmwareBeginHeader : public FractalSysExHeader {
  explicit FirmwareBeginHeader(uint32_t total_count)
      : FractalSysExHeader(FIRMWARE_BEGIN), count(total_count) {
    end.CalculateChecksum(this);
  }
  Fractal28bit count;
  FractalSysExEnd end;

 private:
  DISALLOW_COPY_AND_ASSIGN(FirmwareBeginHeader);
};

struct FirmwareDataHeader : public FractalSysExHeader {
  explicit FirmwareDataHeader(uint16_t count)
      : FractalSysExHeader(FIRMWARE_DATA), value_count(count) {}
  Fractal14bit value_count;
  Fractal32bit values[1];  // Actual size determined by |value_count|.
  // checksum + F7 follows.

 private:
  DISALLOW_COPY_AND_ASSIGN(FirmwareDataHeader);
};

struct ReplyMessage : public FractalSysExHeader {
  explicit ReplyMessage() : FractalSysExHeader(REPLY) {}

  FunctionId reply_to() const {
    return static_cast<FunctionId>(reply_to_id);
  }

  uint8_t reply_to_id;  // Contains the function id of the original command.
  uint8_t error_id;  // 0 means success.

 private:
  DISALLOW_COPY_AND_ASSIGN(ReplyMessage);
};

#pragma pack(pop)

}  // namespace axefx

#endif  // AXE_FX_SYSEX_TYPES_H_
