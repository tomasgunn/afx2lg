#include "gtest/gtest.h"

#include "common_types.h"
#include "axefx/axe_fx_sysex_parser.h"

#include <memory>
#include <fstream>
#include <string>

using testing::internal::FilePath;
extern std::string g_process_path;

// This assumes that the working directory is the same as the build folder.
FilePath GetTestFilePath(const std::string& test_file) {
  FilePath ret(g_process_path);
  ret.Set(ret.RemoveFileName());
  ret.Set(FilePath::ConcatPaths(ret, FilePath("../test/data")));
  ret.Set(FilePath::ConcatPaths(ret, FilePath(test_file)));
  return ret;
}

bool ReadTestFileIntoBuffer(const std::string& file,
                            std::auto_ptr<byte>* buffer,
                            size_t* file_size) {
  FilePath path(GetTestFilePath(file));
  std::ifstream f;
  f.open(path.c_str(), std::fstream::in | std::ios::binary);
  if (!f.is_open())
    return false;

  f.seekg(0, std::ios::end);
  *file_size = f.tellg();
  f.seekg(0, std::ios::beg);

  buffer->reset(new byte[*file_size]);
  f.read(reinterpret_cast<char*>(buffer->get()), *file_size);
  
  return true;
}

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
