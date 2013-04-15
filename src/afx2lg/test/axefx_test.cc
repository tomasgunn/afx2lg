// Copyright (c) 2012, Tomas Gunnarsson
// All rights reserved.

#include "gtest/gtest.h"

#include "axefx/axe_fx_sysex_parser.h"
#include "axefx/blocks.h"
#include "axefx/ir_data.h"
#include "axefx/preset.h"
#include "axefx/sysex_types.h"
#include "json/writer.h"
#include "test/test_utils.h"

#include <functional>

using std::placeholders::_1;

namespace axefx {

class AxeFxII : public testing::Test {
 protected:
  virtual void SetUp() {
    file_size_ = 0;
  }

  virtual void TearDown() {
  }

  bool ParseFile(const char* file_path) {
    bool ok = ReadTestFileIntoBuffer(file_path, &file_contents_, &file_size_);
    ASSERT(ok);
    if (ok) {
      ok = parser_.ParseSysExBuffer(
          file_contents_.get(), file_contents_.get() + file_size_, true);
    } else {
      file_contents_.reset();
      file_size_ = 0;
    }
    return ok;
  }

  bool MatchesFileContent(const std::vector<uint8_t>& data) const {
    if (data.size() != static_cast<size_t>(file_size_))
      return false;
    return memcmp(&data[0], file_contents_.get(), file_size_) == 0;
  }

  SysExParser parser_;
  std::unique_ptr<uint8_t> file_contents_;
  int file_size_;
};

TEST(FractalTypes, Fractal16bit) {
  Fractal16bit f = {0x45, 0x19, 0x3};
  EXPECT_EQ(0x0000ccc5, f.Decode());
  for (uint16_t i = 0; i < 0xffff; ++i) {
    f.Encode(i);
    EXPECT_EQ(i, f.Decode());
  }
}

TEST(FractalTypes, Fractal32bit) {
  Fractal32bit f = {0x4F, 0x10, 0x01, 0x11, 0x04};
  EXPECT_EQ(0x4F482042u, f.Decode());
  f.Encode(f.Decode());
  EXPECT_EQ(0x4Fu, f.b1);
  EXPECT_EQ(0x10u, f.b2);
  EXPECT_EQ(0x01u, f.b3);
  EXPECT_EQ(0x11u, f.b4);
  EXPECT_EQ(0x04u, f.b5);
  std::hash<int> hash;
  for (int i = 0; i < 0xffff; ++i) {
    uint32_t v = static_cast<uint32_t>(hash(i));
    f.Encode(v);
    ASSERT_EQ(v, f.Decode());
  }
}

TEST(FractalTypes, BlockSceneState) {
  // The high order byte represents X/Y state, low order is bypassed flag.
  BlockSceneState state(0x66AA);
  EXPECT_EQ(state.As16bit(), 0x66AA);
  bool bypassed = false;
  bool y_enabled = false;
  for (int i = 0; i < 8; ++i) {
    EXPECT_EQ(bypassed, state.IsBypassedInScene(i)) << "scene: " << i;
    EXPECT_EQ(y_enabled, state.IsConfigYEnabledInScene(i)) << "scene: " << i;
    bypassed = !bypassed;
    if ((i & 1) == 0)
      y_enabled = !y_enabled;
  }
  bypassed = state.IsBypassedInScene(0);
  y_enabled = state.IsConfigYEnabledInScene(0);
  EXPECT_FALSE(state.ScenesAreEqual(0, 1));
  state.CopyScene(0, 1);
  EXPECT_TRUE(state.ScenesAreEqual(0, 1));
  EXPECT_EQ(bypassed, state.IsBypassedInScene(1));
  EXPECT_EQ(y_enabled, state.IsConfigYEnabledInScene(1));
}

TEST_F(AxeFxII, ParseBankFile) {
  ASSERT_TRUE(ParseFile("axefx2/V7_Bank_A.syx"));
  EXPECT_EQ(128u, parser_.presets().size());
}

TEST_F(AxeFxII, ParseFw9bBankFile) {
  ASSERT_TRUE(ParseFile("axefx2/9b_A.syx"));
  const PresetMap& presets = parser_.presets();
  EXPECT_EQ(128u, presets.size());

#if !defined(NDEBUG) && 0
  PresetMap::const_iterator i = presets.begin();
  for (; i != presets.end(); ++i) {
    const Preset& p = *(i->second.get());
    const BlockParameters* amp1 = p.LookupBlock(BLOCK_AMP_1);
    if (amp1) {
      std::cout << "Preset: " << (*i).first << " " << p.name() << "\n";
      uint16_t amp_id = amp1->GetParamValue(DISTORT_TYPE,
          amp1->active_config() == CONFIG_X);
      if (amp1->global_block_index())
        std::cout << " - global=" << amp1->global_block_index() << "\n";
      std::cout << " - amp=" << amp_id << " (guessing='"
                << GetAmpName(amp_id) << "')\n";
    }
  }
#endif
}

TEST_F(AxeFxII, ParseMultipleBankFiles) {
  const char* files[] = {
    "axefx2/V7_Bank_A.syx",
    "axefx2/V7_Bank_B.syx",
    "axefx2/V7_Bank_C.syx",
  };

  for (size_t i = 0; i < arraysize(files); ++i)
    EXPECT_TRUE(ParseFile(files[i]));

  EXPECT_EQ(arraysize(files) * 128u, parser_.presets().size());
}

TEST_F(AxeFxII, ParseMultipleBankFilesV10) {
  const char* files[] = {
    "axefx2/v10/V10_Bank_A.syx",
    "axefx2/v10/V10_Bank_B.syx",
    "axefx2/v10/V10_Bank_C.syx",
  };

  for (size_t i = 0; i < arraysize(files); ++i)
    EXPECT_TRUE(ParseFile(files[i]));

  EXPECT_EQ(arraysize(files) * 128u, parser_.presets().size());
}

TEST_F(AxeFxII, ParseHugeBankFileV10) {
  EXPECT_TRUE(ParseFile("axefx2/v10/V10_All_Banks.syx"));
  EXPECT_EQ(3 * 128u, parser_.presets().size());
}

TEST_F(AxeFxII, ParsePresetFile) {
  ASSERT_TRUE(ParseFile("axefx2/p000318_DynamicJCM800.syx"));
  const PresetMap& presets = parser_.presets();
  EXPECT_EQ(1u, presets.size());
  PresetMap::const_iterator front = presets.begin();
  // This particular test file was saved from the edit buffer, so even though
  // the name suggests 318, the id will be -1 because of what's in the sysex
  // file.
  EXPECT_TRUE(front->second->from_edit_buffer());
  EXPECT_EQ("Dynamic JCM800", front->second->name());
}

void SerializeCallback(const std::vector<uint8_t>& data,
                       std::vector<uint8_t>* out) {
  ASSERT_TRUE(!data.empty());
  ASSERT_TRUE(data[0] == 0xF0);
  ASSERT_TRUE(data[data.size() - 1] == 0xF7);
  ASSERT_TRUE(IsFractalSysEx(&data[0], data.size()));
  out->insert(out->end(), data.begin(), data.end());
}

TEST_F(AxeFxII, SerializePresetFile) {
  // First read and parse a preset file.
  ASSERT_TRUE(ParseFile("axefx2/p000318_DynamicJCM800.syx"));
  ASSERT_EQ(1u, parser_.presets().size());
  std::vector<uint8_t> serialized;
  parser_.Serialize(std::bind(&SerializeCallback, _1, &serialized));
  EXPECT_FALSE(serialized.empty());
  if (!serialized.empty()) {
    SysExParser parser2;
    EXPECT_TRUE(parser2.ParseSysExBuffer(
        &serialized[0], &serialized[0] + serialized.size(), true));
    EXPECT_EQ(parser_.presets().size(), parser2.presets().size());
    EXPECT_EQ(parser_.presets().begin()->second->name(),
              parser2.presets().begin()->second->name());
  }
}

TEST_F(AxeFxII, ParseXyPresetFile) {
  ASSERT_TRUE(ParseFile("axefx2/xy_test2.syx"));
  const PresetMap& presets = parser_.presets();
  EXPECT_EQ(1u, presets.size());
  EXPECT_EQ("Y Is Default", presets.begin()->second->name());
}

TEST_F(AxeFxII, ParseScenesXYBypassFile) {
  ASSERT_TRUE(ParseFile("axefx2/one_amp_8scenes_xy_1.syx"));
  const PresetMap& presets = parser_.presets();
  ASSERT_EQ(1u, presets.size());

  Preset& p = *(presets.begin()->second.get());
  EXPECT_EQ("BYPASS", p.name());

  // Check the scenes, bypass and x/y state of Amp1.
  BlockParameters* amp1 = p.LookupBlock(BLOCK_AMP_1);
  ASSERT_TRUE(amp1 != NULL);
  BlockSceneState state = amp1->GetBypassState();
  bool bypassed = false;
  bool y_enabled = false;
  for (int i = 0; i < 8; ++i) {
    EXPECT_EQ(bypassed, state.IsBypassedInScene(i)) << "scene: " << i;
    EXPECT_EQ(y_enabled, state.IsConfigYEnabledInScene(i)) << "scene: " << i;
    bypassed = !bypassed;
    if ((i & 1) == 0)
      y_enabled = !y_enabled;
  }

  state.CopyScene(0, 1);
  EXPECT_FALSE(amp1->GetBypassState().IsEqual(state));
  amp1->SetBypassState(state);
  EXPECT_TRUE(amp1->GetBypassState().IsEqual(state));
}

#pragma pack(push)
#pragma pack(1)

struct DataBlock0x1234 {
  uint16_t id;  // 0x1234
  uint16_t unknown;  // 0x80
  uint16_t unknown2[80];  // ?
  uint16_t unk_counter_arr[128];  // counts up from 0-127
  uint16_t out1_vol_cc;
  uint16_t out2_vol_cc;
  uint16_t bypass_cc;
  uint16_t unknown3;  // 0
  uint16_t tempo_cc;
  uint16_t unk_ring_modulator_bypass_;  // !?!?!?
  uint16_t tuner_cc;
  uint16_t external_control_cc[12];
  uint16_t unknown4[20];  // 0's
  uint16_t block_bypass_cc[64];  // Starts with Amp1, Amp2, Cab1, Cab2 etc.  Ends with looper bypass(==33)
  uint16_t unknown5[46];
  uint16_t global_bypass_or_scenes_cc;
  uint16_t unknown6;
  uint16_t unknown7;
  uint16_t unknown7_1;
  uint16_t vol_incr_cc;
  uint16_t vol_decr_cc;
  uint16_t unknown8[384];  // All are 0x67.  Something related to presets?
  uint16_t looper_rec_cc;
  uint16_t looper_play_cc;
  uint16_t looper_once_cc;
  uint16_t looper_dub_cc;
  uint16_t looper_rev_cc;
  uint16_t looper_half_speed_cc;
  uint16_t looper_undo_cc;
  uint16_t unknown9[13];  // 0's
  uint16_t input_vol_cc;
  uint16_t unknown10[4];  // 0's
  uint16_t unknown11[20];  // Counts from 100-119.
  uint16_t unknown12[13];  // 0's
  uint16_t x_button_shortcut_to_block_id;  // guess.  default value 106.
  uint16_t y_button_shortcut_to_block_id;  // guess.  default value 106.
  uint16_t unknown13[2];
  uint16_t looper_metronome_cc;
};

#pragma pack(pop)

TEST_F(AxeFxII, ParseSystemBackup) {
  // This is currently done on a best effort basis.  There's plenty of stuff
  // in there that I have no idea what is :)
  ASSERT_TRUE(ParseFile("axefx2/system_backup.syx"));

  // The global system backup file contains 128 preset slots by default.
  // All are simple BYPASS presets by default with version=261.
  // My guess is that 261 means 'available' in this case (could coincide with
  // being one higher than the list of the highest shunt value).
  // The preset slot IDs are in the 'bank D' range, that is 384-511.
  // As global blocks get used, these slots get overwritten with the block
  // state.  Then the |version| field will contain the block id instead,
  // followed by regular block data.
  EXPECT_EQ(128u, parser_.presets().size());

  std::vector<unique_ptr<BlockParameters> > global_blocks;
  PresetParameters left_over_params;

  PresetMap::const_iterator i = parser_.presets().begin();
  for (; i != parser_.presets().end(); ++i) {
    const Preset& preset = *(i->second.get());
    EXPECT_TRUE(preset.is_global_setting());

    const PresetParameters& params = preset.params();
    PresetParameters::const_iterator p = params.begin();

    if (!left_over_params.empty()) {
      ASSERT_NE(BLOCK_TYPE_INVALID,
          GetBlockType(static_cast<AxeFxIIBlockID>(left_over_params[0])));
      size_t missing_values = left_over_params[1] - left_over_params.size() + 2;
      left_over_params.insert(left_over_params.end(), &params[0],
                              &params[missing_values]);
      unique_ptr<BlockParameters> block_params(new BlockParameters());
      size_t values_eaten = block_params->Initialize(&left_over_params[0],
          left_over_params.size());
      EXPECT_EQ(left_over_params.size(), values_eaten);
      global_blocks.push_back(std::move(block_params));
      left_over_params.clear();
      p += missing_values;
    }

    AxeFxIIBlockID id = static_cast<AxeFxIIBlockID>(*p);
    if (GetBlockType(id) != BLOCK_TYPE_INVALID) {
      while (p < (params.end() - 1) && *p) {
        if ((params.end() - p) < (p[1] - 2)) {
          left_over_params.assign(p, params.end());
          break;
        }
        unique_ptr<BlockParameters> block_params(new BlockParameters());
        size_t values_eaten = block_params->Initialize(&(*p), params.end() - p);
        ASSERT_NE(0U, values_eaten);
        global_blocks.push_back(std::move(block_params));
        p += values_eaten;
      }
    } else if (*p == 0x1234) {
      const DataBlock0x1234* data_block =
          reinterpret_cast<const DataBlock0x1234*>(&(*p));
      EXPECT_EQ(data_block->looper_metronome_cc, 122);
      EXPECT_EQ(data_block->x_button_shortcut_to_block_id, BLOCK_AMP_1);
      EXPECT_EQ(data_block->block_bypass_cc[0], 37);  // Amp1 cc
      EXPECT_EQ(data_block->global_bypass_or_scenes_cc, 34);
    }
  }
  EXPECT_EQ(710u, global_blocks.size());
  BlockParameters* amp1 = global_blocks[0].get();
  EXPECT_EQ(BLOCK_AMP_1, amp1->block());
#if defined(_DEBUG)
  uint16_t amp_id_x = amp1->GetParamValue(DISTORT_TYPE, true);
  uint16_t amp_id_y = amp1->GetParamValue(DISTORT_TYPE, false);
  if (amp1->global_block_index())
    std::cout << " - global=" << amp1->global_block_index() << std::endl;
  std::cout << " - amp=" << amp_id_x << "," << amp_id_y
            << "(x='" << GetAmpName(amp_id_x)<< ", "
            << "y='" << GetAmpName(amp_id_y) << ")\n";
#endif
}

TEST_F(AxeFxII, PresetToJson) {
  ASSERT_TRUE(ParseFile("axefx2/one_amp_8scenes_xy_1.syx"));
  const PresetMap& presets = parser_.presets();
  ASSERT_EQ(1u, presets.size());

  const Preset& p = *(presets.begin()->second.get());
  Json::Value preset;
  p.ToJson(&preset);
  Json::Value root;
  root["my_preset"] = preset;
  Json::StyledWriter writer;
  std::cout << writer.write(root);
}

TEST_F(AxeFxII, ParseIRFile) {
  ASSERT_TRUE(ParseFile("axefx2/FreakIR.syx"));
  EXPECT_TRUE(parser_.presets().empty());
  const IRDataArray& ir = parser_.ir_array();
  ASSERT_EQ(1u, ir.size());
  EXPECT_EQ("freakkitchen", ir[0]->name());
  EXPECT_TRUE(ir[0]->from_edit_buffer());
}

TEST_F(AxeFxII, SerializeIRFile) {
  // First read and parse a preset file.
  ASSERT_TRUE(ParseFile("axefx2/FreakIR.syx"));
  EXPECT_TRUE(parser_.presets().empty());
  const IRDataArray& ir = parser_.ir_array();
  ASSERT_EQ(1u, ir.size());
  std::vector<uint8_t> serialized;
  parser_.Serialize(std::bind(&SerializeCallback, _1, &serialized));
  EXPECT_FALSE(serialized.empty());
  if (!serialized.empty()) {
    EXPECT_EQ(static_cast<size_t>(file_size_), serialized.size());
    EXPECT_TRUE(MatchesFileContent(serialized));
  }
}

}  // namespace axefx
