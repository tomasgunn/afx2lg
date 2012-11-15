#include "gtest/gtest.h"

#include "axefx/axe_fx_sysex_parser.h"
#include "common_types.h"
#include "test_utils.h"

TEST(AxeFxII, ParseBankFile) {
  std::auto_ptr<byte> buffer;
  size_t file_size;
  ASSERT_TRUE(ReadTestFileIntoBuffer("axefx2/V7_Bank_A.syx", &buffer,
                                     &file_size));
  AxeFxSysExParser parser;
  parser.ParseSysExBuffer(buffer.get(), buffer.get() + file_size);
  const PresetMap& presets = parser.presets();
  EXPECT_EQ(128, presets.size());
}

TEST(AxeFxII, ParseMultipleBankFiles) {
  AxeFxSysExParser parser;
  const char* files[] = {
    "axefx2/V7_Bank_A.syx",
    "axefx2/V7_Bank_B.syx",
    "axefx2/V7_Bank_C.syx",
  };
  for (int i = 0; i < arraysize(files); ++i) {
    std::auto_ptr<byte> buffer;
    size_t file_size;
    ASSERT_TRUE(ReadTestFileIntoBuffer(files[i], &buffer, &file_size));
    parser.ParseSysExBuffer(buffer.get(), buffer.get() + file_size);
  }

  const PresetMap& presets = parser.presets();
  EXPECT_EQ(arraysize(files) * 128, presets.size());
}
