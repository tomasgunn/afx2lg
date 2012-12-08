// Copyright (c) 2012, Tomas Gunnarsson
// All rights reserved.

#include "gtest/gtest.h"

#include "common/common_types.h"
#include "lg/lg_parser.h"
#include "test_utils.h"

class MockCallback : public lg::LgParserCallback {
 public:
  MockCallback() {}
  virtual ~MockCallback() {}

  virtual const axefx::PresetMap& GetPresetMap() { return map_; }
  virtual void WriteLine(const char* line, size_t length) {
    std::string str(line, length);
    // std::cout << str;
    lines_.push_back(str);
  }

  axefx::PresetMap map_;
  std::vector<std::string> lines_;
};

TEST(LittleGiant, BasicReadInputFile) {
  std::unique_ptr<uint8_t> buffer;
  int file_size;
  ASSERT_TRUE(ReadTestFileIntoBuffer("lg2/input.txt", &buffer,
                                     &file_size));
  MockCallback callback;
  lg::LgParser parser;
  parser.ParseBuffer(&callback, reinterpret_cast<const char*>(buffer.get()),
                     reinterpret_cast<const char*>(buffer.get()) + file_size);
  // The output will basically be the same as input.txt, but
  // contains no presets.
  EXPECT_FALSE(callback.lines_.empty());
}
