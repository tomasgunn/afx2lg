#include <stdafx.h>
#include "axe_fx_sysex_parser.h"

namespace {
const byte kSysExStart = 0xF0;
const byte kSysExEnd = 0xF7;
const byte kFractalMidiId[] = { 0x00, 0x01, 0x74 };
// checksum + kSysExEnd == 2 bytes.
const int kSysExTerminationByteCount = 2;

byte CalculateChecksum(const byte* sys_ex, int size) {
  byte checksum = 0;
  int i = 0;
  for (; i < size - 2; ++i)
    checksum ^= sys_ex[i];
  checksum &= 0x7F;
  return checksum;
}

// http://wiki.fractalaudio.com/axefx2/index.php?title=MIDI_SysEx
bool VerifyChecksum(const byte* sys_ex, int size) {
  return size >= 3 &&
         sys_ex[0] == kSysExStart &&
         sys_ex[size - 1] == kSysExEnd &&
         sys_ex[size - 2] == CalculateChecksum(sys_ex, size);
}

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
  byte ms, ls;

  uint16_t As16bit() const {
    ASSERT((ms & 0x80) == 0);
    ASSERT((ls & 0x80) == 0);
    return (static_cast<uint16_t>(ls) | (static_cast<uint16_t>(ms) << 7));
  }
};

// Wide char stored in 3 septets.
struct SeptetChar {
  byte b1, b2, b3;

  wchar_t AsChar() const {
    ASSERT((b1 & 0xFC) == 0);
    ASSERT((b2 & 0x80) == 0);
    ASSERT((b3 & 0x80) == 0);
    return b3 | static_cast<uint16_t>(b2) << 7 |
           static_cast<uint16_t>(b1) << 14;
  }
};

}  // namespace

struct AxeFxSysExParser::FractalSysExHeader {
  byte sys_ex_start;  // kSysExStart.
  byte manufacture_id[arraysize(kFractalMidiId)];  // kFractalMidiId.
  byte model_id;  // kAxeFx2ModelId
  byte function_id;

  AxeFxModel model() const {
    return static_cast<AxeFxModel>(model_id);
  }

  FunctionId function() const {
    return static_cast<FunctionId>(function_id);
  }
};

struct AxeFxSysExParser::PresetIdHeader : public FractalSysExHeader {
  SeptetPair preset_number;
  SeptetPair unknown;
};

struct AxeFxSysExParser::PresetProperty : public FractalSysExHeader {
  const byte payload[1];
};

struct PresetName {
  // According to the manual the preset name can be as long as 23 characters.
  // In sysex files I've seen, the name is padded with space but appears
  // to be able to be as long as 31 chars + zero terminator.
  SeptetChar chars[23];

  std::wstring ToString() const {
    std::wstring ret;
    ret.reserve(arraysize(chars));
    wchar_t name_buffer[arraysize(chars) + 1] = {0};
    int i = 0;
    while (i < arraysize(chars)) {
      wchar_t ch = chars[i++].AsChar();
      // Names can be zero terminated before the 23'rd char.
      // E.g. default preset 293, "BandTaps" (len=8).
      if (!ch)
        break;
      ret.push_back(ch);
    }
    std::wstring::size_type index = ret.find_last_not_of(L' ');
    index == std::wstring::npos ?
        ret.resize(ret.length()) : ret.resize(index + 1);
    return ret;
  }
};

#pragma pack(pop)

AxeFxSysExParser::AxeFxSysExParser() {
}

AxeFxSysExParser::~AxeFxSysExParser() {
}

void AxeFxSysExParser::ParsePresetId(const PresetIdHeader& header,
                                     int size,
                                     Preset* preset) {
  ASSERT(header.function() == PRESET_ID);
  ASSERT(size == (sizeof(header) + kSysExTerminationByteCount));
  preset->id = header.preset_number.As16bit();
  ASSERT(header.unknown.As16bit() == 0x10);  // <- not sure what this is.
}

void AxeFxSysExParser::ParsePresetProperties(const PresetProperty& header,
                                             int size,
                                             Preset* preset) {
  ASSERT(header.function() == PRESET_PROPERTY);
  // TODO(tommi): Figure this out.
  const byte something[] = { 0x40, 0x00, 0x02, 0x04, 0x00, 0x00 };
  if (memcmp(&header.payload[0], something, arraysize(something)) == 0) {
    const PresetName* preset_name = reinterpret_cast<const PresetName*>(
        &header.payload[0] + arraysize(something));
    ASSERT(reinterpret_cast<const byte*>(preset_name) + sizeof(PresetName) <
              reinterpret_cast<const byte*>(&header) + size);
    preset->name = preset_name->ToString();
  } else {
#if defined(_DEBUG) && 0
    ATLTRACE(_T("Unknown property. %02X %02X %02X %02X %02X %02X\n"),
        header.payload[0], header.payload[1], header.payload[2],
        header.payload[3], header.payload[4], header.payload[5]);
#endif
  }
}

void AxeFxSysExParser::ParsePresetEpilogue(const FractalSysExHeader& header,
                                           int size,
                                           Preset* preset) {
  // TBD
}

void AxeFxSysExParser::ParseFractalSysEx(
    const FractalSysExHeader& header, int size,
    Preset* preset) {
  ASSERT(size > 0);
  if (header.model() != AXE_FX_II) {
    fprintf(stderr, "Not an AxeFx2 SysEx: %i\n", header.model_id);
    return;
  }

  switch (header.function()) {
    case PRESET_ID:
      ParsePresetId(static_cast<const PresetIdHeader&>(header), size, preset);
      break;

    case PRESET_PROPERTY:
      ParsePresetProperties(static_cast<const PresetProperty&>(header), size,
                            preset);
      break;

    case PRESET_EPILOGUE:
      ParsePresetEpilogue(header, size, preset);
      if (preset->id != -1) {
        presets_.insert(std::make_pair(preset->id, *preset));
        preset->id = -1;
        preset->name;
      }
      break;

    default:
      fprintf(stderr, "*** Unknown function id: %i", header.function_id);
      break;
  }
}

void AxeFxSysExParser::ParseSingleSysEx(const byte* sys_ex, int size,
                                        Preset* preset) {
  ASSERT(sys_ex[0] == kSysExStart);
  ASSERT(sys_ex[size - 1] == kSysExEnd);

  if (size < (sizeof(kFractalMidiId) + kSysExTerminationByteCount) ||
      memcmp(&sys_ex[1], &kFractalMidiId[0], sizeof(kFractalMidiId)) != 0) {
    fprintf(stderr, "Not a Fractal sysex.\n");
    return;
  }

  if (!VerifyChecksum(sys_ex, size)) {
    fprintf(stderr, "Invalid checksum.\n");
    return;
  }

  ParseFractalSysEx(*reinterpret_cast<const FractalSysExHeader*>(sys_ex), size,
                    preset);
}

void AxeFxSysExParser::ParseSysExBuffer(const byte* begin, const byte* end) {
  const byte* sys_ex_begins = NULL;
  const byte* pos = begin;
  Preset preset;
  preset.id = -1;
  while (pos < end) {
    if (pos[0] == kSysExStart) {
      ASSERT(!sys_ex_begins);
      sys_ex_begins = &pos[0];
    } else if (pos[0] == kSysExEnd) {
      ASSERT(sys_ex_begins);
      ParseSingleSysEx(sys_ex_begins, (pos - sys_ex_begins) + 1, &preset);
      sys_ex_begins = NULL;
    }
    ++pos;
  }
  ASSERT(!sys_ex_begins);
}
