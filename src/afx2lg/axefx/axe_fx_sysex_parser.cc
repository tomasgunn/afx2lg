// Copyright (c) 2012, Tomas Gunnarsson
// All rights reserved.

#include "axefx/axe_fx_sysex_parser.h"

#include "axefx/blocks.h"
#include "axefx/preset.h"
#include "axefx/sysex_types.h"

namespace axefx {

SysExParser::SysExParser() {
}

SysExParser::~SysExParser() {
}

bool SysExParser::ParseSysExBuffer(const uint8_t* begin, const uint8_t* end) {
  const uint8_t* sys_ex_begins = NULL;
  const uint8_t* pos = begin;
  shared_ptr<Preset> preset;
  while (pos < end) {
    if (pos[0] == kSysExStart) {
      ASSERT(!sys_ex_begins);
      sys_ex_begins = &pos[0];
    } else if (pos[0] == kSysExEnd) {
      ASSERT(sys_ex_begins);
      int size = (pos - sys_ex_begins) + 1;
      if (!IsFractalSysEx(sys_ex_begins, size)) {
        ASSERT(false);
        fprintf(stderr, "This doesn't look like a sysex file for AxeFx\n");
        return false;
      }

      const FractalSysExHeader& header =
          *reinterpret_cast<const FractalSysExHeader*>(sys_ex_begins);
      if (header.model() != AXE_FX_II) {
        // TODO: Replace all these fprintf's with an error callback.
        fprintf(stderr,
            "Sorry, only AxeFx2 supported at this time: type=%i\n",
            header.model_id);
        return false;
      }

      switch (header.function()) {
        case PRESET_ID:
          ASSERT(!preset.get());
          preset.reset(new Preset());
          if (!preset->SetPresetId(static_cast<const PresetIdHeader&>(header),
                                   size)) {
            return false;
          }
          break;

        case PRESET_PARAMETERS: {
          ASSERT(preset);
          const ParameterBlockHeader& param_header =
              static_cast<const ParameterBlockHeader&>(header);
          if (!preset.get() || !preset->AddParameterData(param_header, size))
            return false;
          break;
        }

        case PRESET_CHECKSUM: {
          ASSERT(preset);
          const PresetChecksumHeader& checksum =
              static_cast<const PresetChecksumHeader&>(header);
          if (!preset || !preset->Finalize(checksum, size))
            return false;

          ASSERT(preset->valid());
          presets_.insert(std::make_pair(preset->id(), preset));
          preset.reset();
          break;
        }

        default:
          ASSERT(false);
          return false;
      }
      sys_ex_begins = NULL;
    }
    ++pos;
  }

  ASSERT(!presets_.empty());
  ASSERT(!sys_ex_begins);

  return true;
}

}  // namespace axefx
