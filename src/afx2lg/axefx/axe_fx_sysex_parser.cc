// Copyright (c) 2012, Tomas Gunnarsson
// All rights reserved.

#include "axefx/axe_fx_sysex_parser.h"

#include "axefx/blocks.h"
#include "axefx/ir_data.h"
#include "axefx/preset.h"
#include "axefx/sysex_types.h"

#include <iostream>

namespace axefx {

SysExParser::SysExParser() {
}

SysExParser::~SysExParser() {
}

bool SysExParser::ParseSysExBuffer(const uint8_t* begin, const uint8_t* end,
                                   bool parse_parameter_data) {
  const uint8_t* sys_ex_begins = NULL;
  const uint8_t* pos = begin;

  shared_ptr<Preset> preset;
  unique_ptr<IRData> ir_data;

  while (pos < end) {
    if (pos[0] == kSysExStart) {
      ASSERT(!sys_ex_begins);
      sys_ex_begins = &pos[0];
    } else if (pos[0] == kSysExEnd) {
      ASSERT(sys_ex_begins);
      size_t size = (pos - sys_ex_begins) + 1;
      if (!IsFractalSysEx(sys_ex_begins, size)) {
#ifndef NDEBUG
        std::cerr << "This doesn't look like an AxeFx preset file\n";
#endif
        return false;
      }

      const FractalSysExHeader& header =
          *reinterpret_cast<const FractalSysExHeader*>(sys_ex_begins);
      if (header.model() != AXE_FX_II) {
        std::cerr << "Sorry, only AxeFx2 supported at this time: type="
                  << header.model_id << std::endl;
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
          auto checksum = static_cast<const PresetChecksumHeader*>(&header);
          if (preset &&
              preset->Finalize(checksum, size, !parse_parameter_data)) {
            ASSERT(preset->valid());
            presets_.insert(std::make_pair(preset->id(), preset));
          } else {
            // TODO: If we intend to be able to rewrite the sysex file
            // we must save this block as is and write it at this position
            // unchanged.  Perhaps if Finalize() fails, we could still store
            // the preset in the map, but not clear |params_| and set all
            // other member variables to invalid values. Writing the params_
            // as is, should still work.
          }
          preset.reset();
          break;
        }

        case IR_BEGIN: {
          ASSERT(!ir_data);
          auto ir_header = static_cast<const IRIdHeader&>(header);
          ir_data.reset(new IRData(ir_header));
          break;
        }

        case IR_DATA:
          ASSERT(ir_data);
          if (ir_data) {
            ir_data->AppendFromSysEx(
                static_cast<const IRBlockHeader&>(header), size);
          } else {
            return false;
          }
          break;

        case IR_END: {
          ASSERT(ir_data);
          auto checksum = static_cast<const IRChecksumHeader*>(&header);
          if (!ir_data || checksum->checksum.Decode() != ir_data->Checksum()) {
            std::cerr
                << "Invalid/corrupt IR data or not meant for the AxeFx II\n";
            return false;
          }

          ir_array_.push_back(std::move(ir_data));
          break;
        }

        case FIRMWARE_BEGIN: {
          auto fw_header = static_cast<const FirmwareBegin&>(header);
          if (!fw_header.end.VerifyChecksum(&fw_header)) {
            std::cerr << "Checksum error in firmware header.\n";
            return false;
          }
          std::cout << "Total value count: " << fw_header.count.Decode() << "\n";
          break;
        }

        case FIRMWARE_DATA: {
          auto fw_data = static_cast<const FirmwareData&>(header);
          // |value_count| will usually be 32.
          std::cout << "Value count: " << fw_data.value_count.Decode() << "\n";
          break;
        }

        case FIRMWARE_END: {
          auto fw_checksum = static_cast<const FirmwareChecksum&>(header);
          std::cout << "Checksum: " << fw_checksum.checksum.Decode() << "\n";
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

  if (preset.get() && presets_.empty()) {
    // This is a possible bug in the AxeFx (experienced with 9.02) where
    // a parameter checksum won't be included with a preset dump.
    // To work around this for now, we skip verifying the checksum.
    // We don't do this for full banks though since we'd rather not save
    // a bogus bank.
    if (preset->Finalize(NULL, 0, !parse_parameter_data)) {
      ASSERT(preset->valid());
      presets_.insert(std::make_pair(preset->id(), preset));
      preset.reset();
    }
  }

  ASSERT(!preset.get());  // Half way through parsing a preset?
  ASSERT(!presets_.empty() ^ !ir_array_.empty());
  ASSERT(!sys_ex_begins);

  return true;
}

bool SysExParser::Serialize(const SysExCallback& callback) const {
  for (auto& entry: presets_) {
    if (!entry.second->Serialize(callback))
      return false;
  }

  for (auto& entry: ir_array_) {
    if (!entry->Serialize(callback))
      return false;
  }

  return true;
}

}  // namespace axefx
