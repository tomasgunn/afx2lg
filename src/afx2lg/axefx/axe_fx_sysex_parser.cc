// Copyright (c) 2012, Tomas Gunnarsson
// All rights reserved.

#include "axefx/axe_fx_sysex_parser.h"

#include "axefx/blocks.h"
#include "axefx/ir_data.h"
#include "axefx/preset.h"

#include <iostream>

namespace axefx {

FirmwareData::FirmwareData(const FirmwareBeginHeader& header)
    : expected_total_words_(header.count.Decode()) {
}

void FirmwareData::AddData(const FirmwareDataHeader& header) {
  uint16_t count = header.value_count.Decode();
  for (uint16_t i = 0; i < count; ++i)
    data_.push_back(header.values[i].Decode());
}

bool FirmwareData::Verify(const FirmwareChecksumHeader& header) {
  if (data_.size() != expected_total_words_) {
    std::cerr << "Firmware data corrupt.  Expected " << expected_total_words_
              << " words, but got " << data_.size() << ".\n";
    return false;
  }

  uint32_t checksum = CalculateChecksum(data_);
  if (checksum != header.package_checksum()) {
    std::cerr << "Firmware checksum doesn't match.  Header says: " << std::hex
              << header.package_checksum() << ", calculated: " << checksum
              << ".\n";
    return false;
  }
  return true;
}

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
          const auto& ir_header = static_cast<const IRIdHeader&>(header);
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
          ASSERT(!firmware_.get());
          firmware_.reset(new FirmwareData(
              static_cast<const FirmwareBeginHeader&>(header)));
          break;
        }

        case FIRMWARE_DATA: {
          ASSERT(firmware_.get());
          if (!firmware_.get()) {
            std::cerr << "Received out of band firmware data.\n";
            return false;
          }
          const auto& fw_data = static_cast<const FirmwareDataHeader&>(header);
          firmware_->AddData(fw_data);
          break;
        }

        case FIRMWARE_END: {
          ASSERT(firmware_.get());
          if (!firmware_.get()) {
            std::cerr << "Received out of band firmware checksum.";
            return false;
          }
          const auto& fw_checksum =
              static_cast<const FirmwareChecksumHeader&>(header);
          if (!firmware_->Verify(fw_checksum))
            return false;
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
#ifndef NDEBUG
  // The expectation is that we were only parsing one type of syx data stream.
  int success_count = 0;
  if (!presets_.empty())
    ++success_count;
  if (!ir_array_.empty())
    ++success_count;
  if (firmware_)
    ++success_count;
  ASSERT(success_count == 1);
#endif
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
