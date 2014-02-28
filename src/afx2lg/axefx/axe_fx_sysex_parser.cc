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
  data_.reserve(expected_total_words_);
}

void FirmwareData::AddData(const FirmwareDataHeader& header) {
  uint16_t count = header.value_count.Decode();
  ASSERT(reinterpret_cast<const uint8_t*>(&header)[
      sizeof(FirmwareDataHeader) + ((count - 1) * sizeof(Fractal32bit)) + 1] == 0xF7);
  for (uint16_t i = 0; i < count; ++i) {
    uint32_t value = header.values[i].Decode();
    data_.push_back(value);
#if !defined(NDEBUG)
    // There appears to be a bug in the encoder that's used to encode firmware
    // data, which causes the 4 upper most bits of the 5byte midi data to be
    // used when they should be 0.  This means that we can't compare bit for bit
    // the output of our serialization and the original fw file.
    Fractal32bit test(value);
    ASSERT(test.Decode() == value);
    test.b5 |= (header.values[i].b5 & 0x70);
    int mysterious_value = (header.values[i].b5 & 0x70) >> 4;
    if (!mysterious_value) {
      // std::cout << "mysterious_value is 0 for index " << i << "\n";
    } else {
      ASSERT(mysterious_value == 7);
    }
    ASSERT(memcmp(&test, &header.values[i], sizeof(test)) == 0);
#endif
  }
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

bool FirmwareData::Serialize(const SysExCallback& callback) const {
  ASSERT(expected_total_words_ == data_.size());

  // TODO: This is very similar to the IRData::Serialize call.
  // Refactor to maintain the common parts in a single place.

  // Write the firmware header;
  std::vector<uint8_t> data;
  data.resize(sizeof(FirmwareBeginHeader));
  new (&data[0]) FirmwareBeginHeader(static_cast<uint32_t>(data_.size()));
  callback(data);

  // Write all the firmware data, 32 words at a time.
  const uint16_t kFirmwareWordsPerHeader = 32u;
  data.resize(
      sizeof(FirmwareDataHeader) +
      (sizeof(Fractal32bit) * (kFirmwareWordsPerHeader - 1)) +
      sizeof(FractalSysExEnd));

  auto* header = new (&data[0]) FirmwareDataHeader(kFirmwareWordsPerHeader);
  uint16_t value_index = 0;
  for (size_t i = 0; i < data_.size(); ++i) {
    value_index = i % kFirmwareWordsPerHeader;
    header->values[value_index].Encode(data_[i]);
    if (value_index == (kFirmwareWordsPerHeader - 1)) {
      auto checksum = new (&header->values[value_index + 1]) FractalSysExEnd();
      checksum->CalculateChecksum(header);
      callback(data);
      memset(&header->values[0], 0,
             kFirmwareWordsPerHeader * sizeof(header->values[0]));
    }
  }

  if (value_index != (kFirmwareWordsPerHeader - 1)) {
    header->value_count.Encode(value_index + 1);
    auto checksum = new (&header->values[value_index + 1]) FractalSysExEnd();
    checksum->CalculateChecksum(header);
    data.resize(data.size() -
                ((kFirmwareWordsPerHeader - (value_index + 1)) *
                 sizeof(Fractal32bit)));
    callback(data);
  }

  // Write the Checksum.
  data.resize(sizeof(FirmwareChecksumHeader));
  new (&data[0]) FirmwareChecksumHeader(CalculateChecksum(data_));
  callback(data);

  return true;
}

SysExParser::SysExParser() : type_(UNKNOWN) {
}

SysExParser::~SysExParser() {
}

bool SysExParser::ParseSysExBuffer(const uint8_t* begin, const uint8_t* end,
                                   bool parse_parameter_data) {
  ASSERT(!firmware_);
  ASSERT(ir_array_.empty());
  ASSERT(presets_.empty() || (type_ == PRESET || type_ == PRESET_ARCHIVE));

  const uint8_t* sys_ex_begins = NULL;
  const uint8_t* pos = begin;

  shared_ptr<Preset> preset;
  unique_ptr<IRData> ir_data;
  unique_ptr<FirmwareData> firmware;

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
            std::cerr << "Failed to parse preset data." << std::endl;
            return false;
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
          ASSERT(!firmware.get());
          firmware.reset(new FirmwareData(
              static_cast<const FirmwareBeginHeader&>(header)));
          break;
        }

        case FIRMWARE_DATA: {
          ASSERT(firmware.get());
          if (!firmware.get()) {
            std::cerr << "Received out of band firmware data.\n";
            return false;
          }
          const auto& fw_data = static_cast<const FirmwareDataHeader&>(header);
          firmware->AddData(fw_data);
          break;
        }

        case FIRMWARE_END: {
          ASSERT(firmware.get());
          if (!firmware.get()) {
            std::cerr << "Received out of band firmware checksum.";
            return false;
          }
          const auto& fw_checksum =
              static_cast<const FirmwareChecksumHeader&>(header);
          if (!firmware->Verify(fw_checksum))
            return false;
          firmware_.swap(firmware);
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
  ASSERT(!sys_ex_begins);

  // The expectation is that we were only parsing one type of syx data stream.
  int success_count = 0;
  if (!presets_.empty()) {
    type_ = presets_.size() == 1 ? PRESET : PRESET_ARCHIVE;
    ++success_count;
  }

  if (!ir_array_.empty()) {
    type_ = IR;
    ++success_count;
  }

  if (firmware_) {
    type_ = FIRMWARE;
    ++success_count;
  }

  if (success_count == 0)
    return false;

  ASSERT(success_count == 1);

  if (success_count != 1) {
    type_ = UNKNOWN;
    std::cerr << "Received a mix bag of data. Not safe to continue...\n";
    return false;
  }

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

  if (firmware_) {
    if (!firmware_->Serialize(callback))
      return false;
  }

  return true;
}

}  // namespace axefx
