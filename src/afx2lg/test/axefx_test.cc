// Copyright (c) 2012, Tomas Gunnarsson
// All rights reserved.

#include "gtest/gtest.h"

#include "axefx/axe_fx_sysex_parser.h"
#include "axefx/blocks.h"
#include "axefx/preset.h"
#include "axefx/sysex_types.h"
#include "test/test_utils.h"

namespace axefx {

class AxeFxII : public testing::Test {
 protected:
  virtual void SetUp() {
  }

  virtual void TearDown() {
  }

  bool ParseFile(const char* file_path) {
    std::auto_ptr<uint8_t> buffer;
    int file_size;
    bool ok = ReadTestFileIntoBuffer(file_path, &buffer, &file_size);
    ASSERT(ok);
    if (ok) {
      EXPECT_TRUE(ok = parser_.ParseSysExBuffer(buffer.get(),
                                                buffer.get() + file_size));
    }
    return ok;
  }

  SysExParser parser_;
};

TEST(FractalTypes, Fractal16bit) {
  Fractal16bit f = {0x45, 0x19, 0x3};
  EXPECT_EQ(0x0000ccc5, f.As16bit());
}

TEST(FractalTypes, BlockSceneState) {
  // The high order byte represents X/Y state, low order is bypassed flag.
  BlockSceneState state(0x66AA);
  bool bypassed = false;
  bool y_enabled = false;
  for (int i = 0; i < 8; ++i) {
    EXPECT_EQ(bypassed, state.IsBypassedInScene(i)) << "scene: " << i;
    EXPECT_EQ(y_enabled, state.IsConfigYEnabledInScene(i)) << "scene: " << i;
    bypassed = !bypassed;
    if ((i & 1) == 0)
      y_enabled = !y_enabled;
  }
}

TEST_F(AxeFxII, ParseBankFile) {
  ASSERT_TRUE(ParseFile("axefx2/V7_Bank_A.syx"));
  EXPECT_EQ(128, parser_.presets().size());
}

TEST_F(AxeFxII, ParseFw9bBankFile) {
  ASSERT_TRUE(ParseFile("axefx2/9b_A.syx"));
  const PresetMap& presets = parser_.presets();
  EXPECT_EQ(128, presets.size());

#ifdef _DEBUG
  PresetMap::const_iterator i = presets.begin();
  for (; i != presets.end(); ++i) {
    const Preset& p = *(i->second.get());
    shared_ptr<BlockParameters> amp1 = p.LookupBlock(BLOCK_AMP_1);
    if (amp1) {
      printf("Preset: %i %s\n", (*i).first, p.name().c_str());
      int amp_id = amp1->GetParamValue(DISTORT_TYPE,
                                       amp1->active_config() == CONFIG_X);
      if (amp1->global_block_index())
        printf(" - global=%i\n", amp1->global_block_index());
      printf(" - amp=%i (guessing='%s')\n", amp_id, GetAmpName(amp_id));
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

  for (int i = 0; i < arraysize(files); ++i)
    EXPECT_TRUE(ParseFile(files[i]));

  EXPECT_EQ(arraysize(files) * 128, parser_.presets().size());
}

TEST_F(AxeFxII, ParsePresetFile) {
  ASSERT_TRUE(ParseFile("axefx2/p000318_DynamicJCM800.syx"));
  const PresetMap& presets = parser_.presets();
  EXPECT_EQ(1, presets.size());
  PresetMap::const_iterator front = presets.begin();
  // This particular test file was saved from the edit buffer, so even though
  // the name suggests 318, the id will be -1 because of what's in the sysex
  // file.
  EXPECT_TRUE(front->second->from_edit_buffer());
  EXPECT_EQ("Dynamic JCM800", front->second->name());
}

TEST_F(AxeFxII, ParseXyPresetFile) {
  ASSERT_TRUE(ParseFile("axefx2/xy_test2.syx"));
  const PresetMap& presets = parser_.presets();
  EXPECT_EQ(1, presets.size());
  EXPECT_EQ("Y Is Default", presets.begin()->second->name());
}

TEST_F(AxeFxII, ParseScenesXYBypassFile) {
  ASSERT_TRUE(ParseFile("axefx2/one_amp_8scenes_xy_1.syx"));
  const PresetMap& presets = parser_.presets();
  ASSERT_EQ(1, presets.size());

  const Preset& p = *(presets.begin()->second.get());
  EXPECT_EQ("BYPASS", p.name());

  // Check the scenes, bypass and x/y state of Amp1.
  shared_ptr<BlockParameters> amp1 = p.LookupBlock(BLOCK_AMP_1);
  ASSERT_TRUE(amp1);
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
}

TEST_F(AxeFxII, ParseSystemBackup) {
  // This is currently done on a best effort basis.  There's plenty of stuff
  // in there that I have no idea what is :)
  ASSERT_TRUE(ParseFile("axefx2/system_backup.syx"));
  EXPECT_EQ(1, parser_.presets().size());
#ifdef _DEBUG
  PresetMap::const_iterator i = parser_.presets().begin();
  for (; i != parser_.presets().end(); ++i) {
    const Preset& p = *(i->second.get());
    EXPECT_EQ("BYPASS", p.name());
    shared_ptr<BlockParameters> amp1 = p.LookupBlock(BLOCK_AMP_1);
    std::cout << "Preset: " << i->first << " " << p.name() << std::endl;
    if (amp1) {
      int amp_id = amp1->GetParamValue(DISTORT_TYPE,
                                       amp1->active_config() == CONFIG_X);
      if (amp1->global_block_index())
        std::cout << " - global=" << amp1->global_block_index() << std::endl;
      std::cout << " - amp=" << amp_id << "(guessing='" << GetAmpName(amp_id)
                << ")\n";
    }
  }
#endif
}

}  // namespace axefx
