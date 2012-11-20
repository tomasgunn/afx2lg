// Copyright (c) 2012, Tomas Gunnarsson
// All rights reserved.

#include "gtest/gtest.h"

#include "axefx/axe_fx_sysex_parser.h"
#include "axefx/blocks.h"
#include "axefx/sysex_types.h"
#include "test/test_utils.h"

TEST(AxeFxII, Fractal16bit) {
  axefx::Fractal16bit f = {0x45, 0x19, 0x3};
  EXPECT_EQ(0x0000ccc5, f.As16bit());
}

TEST(AxeFxII, BlockSceneState) {
  // The high order byte represents X/Y state, low order is bypassed flag.
  axefx::BlockSceneState state(0x66AA);
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

TEST(AxeFxII, ParseBankFile) {
  std::auto_ptr<uint8_t> buffer;
  int file_size;
  ASSERT_TRUE(ReadTestFileIntoBuffer("axefx2/V7_Bank_A.syx", &buffer,
                                     &file_size));
  axefx::SysExParser parser;
  EXPECT_TRUE(parser.ParseSysExBuffer(buffer.get(), buffer.get() + file_size));
  const PresetMap& presets = parser.presets();
  EXPECT_EQ(128, presets.size());
}

TEST(AxeFxII, ParseFw9bBankFile) {
  std::auto_ptr<uint8_t> buffer;
  int file_size;
  ASSERT_TRUE(ReadTestFileIntoBuffer("axefx2/9b_A.syx", &buffer,
                                     &file_size));
  axefx::SysExParser parser;
  EXPECT_TRUE(parser.ParseSysExBuffer(buffer.get(), buffer.get() + file_size));
  const PresetMap& presets = parser.presets();
  EXPECT_EQ(128, presets.size());
}

TEST(AxeFxII, ParseMultipleBankFiles) {
  const char* files[] = {
    "axefx2/V7_Bank_A.syx",
    "axefx2/V7_Bank_B.syx",
    "axefx2/V7_Bank_C.syx",
  };

  axefx::SysExParser parser;
  for (int i = 0; i < arraysize(files); ++i) {
    std::auto_ptr<uint8_t> buffer;
    int file_size;
    ASSERT_TRUE(ReadTestFileIntoBuffer(files[i], &buffer, &file_size));
    EXPECT_TRUE(parser.ParseSysExBuffer(buffer.get(),
                                        buffer.get() + file_size));
  }
  const PresetMap& presets = parser.presets();
  EXPECT_EQ(arraysize(files) * 128, presets.size());
}

TEST(AxeFxII, ParsePresetFile) {
  std::auto_ptr<uint8_t> buffer;
  int file_size;
  ASSERT_TRUE(ReadTestFileIntoBuffer("axefx2/p000318_DynamicJCM800.syx",
                                     &buffer, &file_size));

  axefx::SysExParser parser;
  EXPECT_TRUE(parser.ParseSysExBuffer(buffer.get(), buffer.get() + file_size));
  const PresetMap& presets = parser.presets();
  EXPECT_EQ(1, presets.size());
  PresetMap::const_iterator front = presets.begin();
  // This particular test file was saved from the edit buffer, so even though
  // the name suggests 318, the id will be -1 because of what's in the sysex
  // file.
  EXPECT_EQ(-1, front->first);
  EXPECT_EQ("Dynamic JCM800", front->second.name);
}

TEST(AxeFxII, ParseXyPresetFile) {
  std::auto_ptr<uint8_t> buffer;
  int file_size;
  // This file has Amp1 set to Y by default.
  ASSERT_TRUE(ReadTestFileIntoBuffer("axefx2/xy_test2.syx",
                                     &buffer, &file_size));
  axefx::SysExParser parser;
  EXPECT_TRUE(parser.ParseSysExBuffer(buffer.get(), buffer.get() + file_size));
  const PresetMap& presets = parser.presets();
  EXPECT_EQ(1, presets.size());
  EXPECT_EQ("Y Is Default", presets.begin()->second.name);
}

TEST(AxeFxII, ParseScenesXYBypassFile) {
  std::auto_ptr<uint8_t> buffer;
  int file_size;
  ASSERT_TRUE(ReadTestFileIntoBuffer("axefx2/one_amp_8scenes_xy_1.syx",
                                     &buffer, &file_size));
  axefx::SysExParser parser;
  EXPECT_TRUE(parser.ParseSysExBuffer(buffer.get(), buffer.get() + file_size));
  const PresetMap& presets = parser.presets();
  EXPECT_EQ(1, presets.size());
  EXPECT_EQ("BYPASS", presets.begin()->second.name);
  // TODO: Check the scenes, bypass and x/y state of Amp1.
}
