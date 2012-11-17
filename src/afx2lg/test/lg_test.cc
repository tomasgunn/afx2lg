// Copyright (c) 2012, Tomas Gunnarsson
// All rights reserved.

#include "gtest/gtest.h"

#include "common_types.h"
#include "lg/lg_parser.h"
#include "test_utils.h"

class MockCallback : public lg::LgParserCallback {
 public:
  MockCallback() {}
  virtual ~MockCallback() {}

  virtual const PresetMap& GetPresetMap() { return map_; }
  virtual void WriteLine(const char* line, int length) {
    std::string str(line, length);
    // printf("%hs", str.c_str());
    lines_.push_back(str);
  }

  PresetMap map_;
  std::vector<std::string> lines_;
};


TEST(LittleGiant, BasicReadInputFile) {
  std::auto_ptr<byte> buffer;
  std::streampos file_size;
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
