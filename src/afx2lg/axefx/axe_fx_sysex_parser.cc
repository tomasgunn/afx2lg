// Copyright (c) 2012, Tomas Gunnarsson
// All rights reserved.

#include "axefx/axe_fx_sysex_parser.h"

#include "axefx/blocks.h"
#include "axefx/sysex_types.h"

#include <algorithm>

namespace axefx {

SysExParser::SysExParser() {
}

SysExParser::~SysExParser() {
}

void SysExParser::ParsePresetId(const PresetIdHeader& header,
                                int size,
                                Preset* preset) {
  ASSERT(header.function() == PRESET_ID);
  ASSERT(size == (sizeof(header) + kSysExTerminationByteCount));
  if (header.preset_number.ms == 0x7f && header.preset_number.ls == 0x0) {
    // This is a special case that means the preset is destined for (or comes
    // from) the edit buffer.  In this case, we just set the id to -1.
    // http://forum.fractalaudio.com/axe-fx-ii-discussion/58581-help-loading-presets-using-sysex-librarian.html#post732659
    preset->id = -1;
  } else {
    preset->id = header.preset_number.As16bit();
  }
  ASSERT(header.unknown.As16bit() == 0x10);  // <- not sure what this is.
}

void SysExParser::ParsePresetProperties(int* preset_chunk_id,
                                        const PresetProperty& header,
                                        int size,
                                        Preset* preset) {
  ASSERT(header.function() == PRESET_PROPERTY);

  const Fractal16bit* values = reinterpret_cast<const Fractal16bit*>(&header.function_id);
  const int value_count = (size - sizeof(FractalSysExHeader) - 1) / sizeof(values[0]);

  // As far as I've seen, property blocks always start with this 16bit/3byte ID.
  ASSERT(values[0].As16bit() == 0x2078);

  if (values[0].As16bit() == 0x2078 && values[1].b2 == 0x04) {
    const PresetName* preset_name =
        reinterpret_cast<const PresetName*>(&values[3]);
    ASSERT(reinterpret_cast<const uint8_t*>(preset_name) + sizeof(PresetName)
           < reinterpret_cast<const uint8_t*>(&header) + size);
    preset->name = preset_name->ToString();
#if defined(_DEBUG)
    // In v7, the second triplet is 02 04 00 (2.2)
    // In v9 beta, this changed to 04 04 00 (2.4).
    // Mabe this is a version number of some sorts?
    printf("Preset %i %hs - (version number? %i.%i (%i/%X))\n",
        preset->id, preset->name.c_str(),
        values[1].As16bit() >> 8, values[1].As16bit() & 0xff,
        values[1].As16bit(),
        values[1].As16bit());
#endif
  } else if (*preset_chunk_id == 2) {
    // Effect blocks can be enumerated twice.  First in the first
    // property sysex property value - and this is optional.
    // Secondly in the 2nd property.
    //
    // If enumerated in the first property value, each block will have an
    // 8 16bit value (8 * 3 bytes) entry.  Before the blocks, there is a
    // section of 4 values.  Probably has something to do with the grid,
    // which has 4 rows.
    printf("Value count: %i - val[0]=0x%X\n",
        value_count, values[1].As16bit());
#if 0
    const BlockState* state = reinterpret_cast<const BlockState*>(&values[5]);
    while (reinterpret_cast<const uint8_t*>(state + 1) <
           reinterpret_cast<const uint8_t*>(&header) + size) {
      if (state->block_id.As16bit()) {
        printf("block (%i/%X): %hs\n",
          state->block_id.As16bit(), state->block_id.As16bit(),
          GetBlockName(state->block_id.As16bit()));
      }
      ++state;
    }
#endif
    for (int i = 0; i <= value_count; ++i) {
      if (values[i].As16bit() && values[i].As16bit() != 2) {
        printf("block (%i/%X): %hs\n",
            values[i].As16bit(), values[i].As16bit(),
            GetBlockName(values[i].As16bit()));
      }
    }
  } else if (*preset_chunk_id == 3) {
    // When enumerated in the 2nd property sysex, all the block parameters
    // will be included, so each entry will be variable in length.
    // It's possible that values[1] will then include a hint as to how long
    // the parameter block is.
    printf("Value count: %i - val[0]=0x%X\n",
        value_count, values[1].As16bit());
    // While skimming over stuff we don't know what is.
    bool found_blocks = false;
    for (int i = 20; i < value_count; ++i) {
      // Bypass and X/Y states can be stored in the higher order byte.
      int block_id = (values[i].As16bit() & 0xFFF);
/*      if (!found_blocks &&
          (block_id < kFirstBlockId || block_id > kLastBlockId)) {
        continue;
      }*/

      found_blocks = true;

      if (block_id == 0x6A) {
        /*
        // For < fw9 versions, bypass and X/Y state are stored in values[i].b3.
        bool x = (values[i].b3 & 0x2) == 0;  // y = !x
        bool bypassed = (values[i].b3 & 0x1) != 0;
        */
        BlockSceneState bypass_xy(values[i + 30].As16bit());
        int param_count = values[i+1].As16bit() / sizeof(uint16_t);

        printf(" @%i Found %hs type=%hs (%i) %i (preset %i %hs) - params=%i\n",
            i, GetBlockName(block_id), GetAmpName(values[i+2].As16bit()),
            values[i+2].As16bit(),
            values[i+1].As16bit(),
            preset->id, preset->name.c_str(),
            param_count);
        i += (param_count - 1);
      /*} else if (values[i].As16bit() == 0x6B) {
        printf("  %i Found Amp2 type=%hs (%i) (preset %i %hs)\n",
            i, GetAmpName(values[i+2].As16bit()),
            values[i+2].As16bit(), preset->id, preset->name.c_str());*/
      } else {
        int param_count = values[i+1].As16bit() / sizeof(uint16_t);
        printf(" Block: %hs - %i params\n",
            GetBlockName(values[i].b1), param_count);
        i += (param_count - 1);
      }
    }
  }
}

void SysExParser::AppendBlockData(BlockData* block_data,
                                  const FractalSysExHeader& header,
                                  int size) {
  const Fractal16bit* values = reinterpret_cast<const Fractal16bit*>(&header.function_id);
  // As far as I've seen, property blocks always start with this 16bit/3byte ID.
  ASSERT(values[0].As16bit() == 0x2078);
  int value_count = (size - sizeof(FractalSysExHeader) - 1) / sizeof(values[0]);

  block_data->reserve(block_data->size() + value_count);
  ASSERT(values[value_count].b2 == kSysExEnd);
  for (int i = 1; i < value_count; ++i)
    block_data->push_back(values[i].As16bit());
}

void SysExParser::ParseBlockData(const BlockData& data,
                                 Preset* preset) {
  if (data.empty()) {
    ASSERT(false);
    return;
  }

  // Values (indexes are inclusive):
  // 0 == 516 for fw9 and higher. 514 for older.  Version field? or multiply by
  //    2 to get value count?
  // 1 == 0 ?
  // 2-32 == Preset name.
  // 33-35 == 0 ?
  // 36-?  == 8 value entries per effect block.  First value of each such block
  //          is the block id.  Possibly the second is the one-based row?
  // 132 == 10 <- This is the id of the terminating 8 value block.
  // 140 == First effect block.
  //        All blocks have 'id' as their first value.
  //        Parameter count is the second value.  This is used to get to the
  //        offset of the next effect parameter block.
  //        Guess: Parameters are stored twice, first X, then Y?
  //        Note that X/Y is only supported for these block types:
  //          Amp, Cab, Chorus, Delay, Drive, Flanger, Pitch Shifter, Phaser, Reverb, and Wahwah
  //        So, e.g. multidelay will not have Y state.
  //        What about the bypass,x/y state that now has per-scene info?
  //

  BlockData::const_iterator p = data.begin();
  uint16_t version = *p;
  if (version != 0x204 && version != 0x202) {
    fprintf(stderr, "Unsupported syx file - 0x%04X\n", version);
    return;
  }
  //============================================================================
  ASSERT(p[1] == 0);
  p += 2;
  preset->name.assign(p, p + 31);
  std::string::size_type index = preset->name.length() - 1;
  while (index > 0 && preset->name[index] == ' ' || preset->name[index] == '\0')
    --index;  
  preset->name.resize(index + 1);
  p += 31;
  printf("================== %i %hs =============================\n",
      preset->id, preset->name.c_str());
  //============================================================================
  ++p;
  // TODO: Keep the blocks variable only in debug mode as a sanity check that
  // what's in the matrix also exists in the parameter section.
  std::vector<uint16_t> blocks;
  for (int x = 0; x < 12; ++x) {
    for (int y = 0; y < 4; ++y) {
      if (*p && !IsShunt(*p)) {
        blocks.push_back(*p);
        printf("  Block(%i,%i): %hs\n", x, y, GetBlockName(*p));
      }
      ++p;
      ++p;  // This value is usually 2... not sure what that means.
    }
  }
  //============================================================================

  // The per-block parameter blobs aren't necessarily stored in the same order
  // they appear in the matrix.
  int block_count = 0;
  while (p < data.end() && *p) {
    uint16_t block_id = *(p++);
    uint16_t params = *(p++);
    if (block_id >= 1 && block_id < kFirstBlockId) {
      // Optional modifier section.
      // The AxeFx supports 16 modifiers
      // (http://wiki.fractalaudio.com/index.php?title=Controllers_and_modifiers)
      // I'm guessing their IDs are 1-16, but only check for IDs smaller than the
      // lowest known effect block id.
      printf("  Skipping modifier %i\n", *p);
    } else {
      if ((block_id >> 8) != 0) {
        // In versions before fw9, the X/Y state was stored in the  highest bit
        // but I've seen AxeEdit save files like this even with the version being
        // 0x204, so no check for that.
        uint8_t state = block_id >> 8;
        block_id &= 0x00FF;
        bool y_config_active = (state & 0x80) != 0;
        uint8_t global_block_index = state & 0x0F;
        printf("  block state (old way): 0x%02X\n", state);
      }
      if (!blocks.empty()) {
        std::vector<uint16_t>::iterator it = std::find(blocks.begin(), blocks.end(), block_id);
        if (it != blocks.end())
          blocks.erase(it);
      }
      const char* block_name = GetBlockName(block_id);
      printf("  %i parameter block for: %hs\n", params, block_name);
    }
    p += params;
    block_count++;
  }
  ASSERT(blocks.empty());
}

void SysExParser::ParseFractalSysEx(BlockData* block_data,
                                    const FractalSysExHeader& header,
                                    int size,
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
      AppendBlockData(block_data, header, size);
      break;

    case PRESET_EPILOGUE:
      ParseBlockData(*block_data, preset);

      if (!preset->name.empty()) {
        presets_.insert(std::make_pair(preset->id, *preset));
        preset->id = -1;
        preset->name;
      }
      // Reset the counter for parsing the next preset.
      //*preset_chunk_id = -1;
      block_data->clear();
      break;

    default:
      fprintf(stderr, "*** Unknown function id: %i", header.function_id);
      break;
  }
}

void SysExParser::ParseSingleSysEx(BlockData* block_data,
                                   const uint8_t* sys_ex,
                                   int size,
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

  ParseFractalSysEx(block_data,
      *reinterpret_cast<const FractalSysExHeader*>(sys_ex), size, preset);
}

void SysExParser::ParseSysExBuffer(const uint8_t* begin, const uint8_t* end) {
  BlockData block_data;
  const uint8_t* sys_ex_begins = NULL;
  const uint8_t* pos = begin;
  Preset preset;
  preset.id = -1;
  int preset_chunk_id = 0;
  while (pos < end) {
    if (pos[0] == kSysExStart) {
      ASSERT(!sys_ex_begins);
      sys_ex_begins = &pos[0];
    } else if (pos[0] == kSysExEnd) {
      ASSERT(sys_ex_begins);
      ParseSingleSysEx(&block_data, sys_ex_begins,
                       (pos - sys_ex_begins) + 1, &preset);
      sys_ex_begins = NULL;
      ++preset_chunk_id;
    }
    ++pos;
  }
  ASSERT(!sys_ex_begins);
}

}  // namespace axefx
