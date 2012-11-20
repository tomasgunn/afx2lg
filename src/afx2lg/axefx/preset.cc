// Copyright (c) 2012, Tomas Gunnarsson
// All rights reserved.

#include "axefx/preset.h"

#include "axefx/blocks.h"
#include "axefx/sysex_types.h"

#include <algorithm>

namespace axefx {

const int kInvalidPresetId = -1;
const int kPresetIdBuffer = -2;

Preset::Preset() : id_(kInvalidPresetId) {}
Preset::~Preset() {}

bool Preset::valid() const {
  return id_ != kInvalidPresetId;
}

bool Preset::from_edit_buffer() const {
  return id_ == kPresetIdBuffer;
}

bool Preset::SetPresetId(const PresetIdHeader& header, int size) {
  ASSERT(header.function() == PRESET_ID);
  ASSERT(size == (sizeof(header) + kSysExTerminationByteCount));
  ASSERT(header.unknown.As16bit() == 0x10);  // <- not sure what this is.
  if (header.preset_number.ms == 0x7f && header.preset_number.ls == 0x0) {
    // This is a special case that means the preset is destined for (or comes
    // from) the edit buffer.  In this case, we just set the id to -1.
    // http://forum.fractalaudio.com/axe-fx-ii-discussion/58581-help-loading-presets-using-sysex-librarian.html#post732659
    id_ = kPresetIdBuffer;
  } else {
    id_ = header.preset_number.As16bit();
    if (id_ < 0 || id_ > (0x80 * 3))
      id_ = kInvalidPresetId;
  }

  return valid();
}

bool Preset::AddParameterData(const ParameterBlockHeader& header, int size) {
  ASSERT(valid());
  bool ret = params_.AppendFromSysEx(header, size);
  if (!ret)
    id_ = kInvalidPresetId;
  return ret;
}

bool Preset::Finalize(const PresetChecksumHeader& header, int size) {
  ASSERT(valid());

  if (header.checksum.As16bit() != params_.Checksum())
    return false;

  // TODO: Refactor

  // Values (indexes are inclusive):
  // 0 == 516 for fw9 and higher. 514 for older.  Version field? or multiply by
  //    2 to get value count?
  // 1 == 0 ?
  // 2-32 == Preset name.
  // 33-35 == 0 ?
  // 36-?  == 8 value entries per effect block.  First value of each such block
  //          is the block id.  Possibly the second is the one-based row?
  // 132 == First effect or modifier block.
  //        All blocks have 'id' as their first value.
  //        Parameter count is the second value.  This is used to get to the
  //        offset of the next parameter block.
  //        Parameters are stored twice for blocks that support X/Y.
  //        First X, then Y.
  //        Note that X/Y is only supported for these block types:
  //          Amp, Cab, Chorus, Delay, Drive, Flanger, Pitch Shifter, Phaser,
  //          Reverb, and Wahwah
  //        So, e.g. multidelay will not have Y state.
  //        What about the bypass,x/y state that now has per-scene info?
  //

  PresetParameters::const_iterator p = params_.begin();
  uint16_t version = *p;
  if (version != 0x204 && version != 0x202) {
    fprintf(stderr, "Unsupported syx file - 0x%04X\n", version);
    return false;
  }
  //============================================================================
  ASSERT(p[1] == 0);
  p += 2;
  name_.assign(p, p + 31);
  std::string::size_type index = name_.length() - 1;
  while (index > 0 && name_[index] == ' ' || name_[index] == '\0')
    --index;  
  name_.resize(index + 1);
  p += 31;
  printf("================== %i %hs =============================\n",
      id_, name_.c_str());
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
      uint16_t input_mask = *(p++);
      // |input_mask| is a bit mask of 4 bits that shows how the current block
      // receives its input from the previous column.
      // 0010 means the current block connects with the block at x-1,1.
      // 1001 means connections with blocks x-1,0 and x-1,3.
    }
  }
  //============================================================================

  // The per-block parameter blobs aren't necessarily stored in the same order
  // they appear in the matrix.
  int block_count = 0;
  while (p < params_.end() && *p) {
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
#ifdef _DEBUG
      if (block_id == 0x6A) {
        // Little debug test for reading the bypass and X/Y state.
        // Scenes are also supported.
        BlockSceneState bypass_xy(p[28]);
        printf(" Found %hs type_x=%hs type_y=%hs\n",
            block_name,
            GetAmpName(p[0]),
            GetAmpName(p[(params / 2) + 0]));
      }
#endif
    }
    p += params;
    block_count++;
  }
  ASSERT(blocks.empty());

  return true;
}

}  // namespace axefx
