// Copyright (c) 2012, Tomas Gunnarsson
// All rights reserved.

#include "gtest/gtest.h"

#include "common/common_types.h"
#include "midi/midi_in.h"
#include "midi/midi_out.h"
#include "test_utils.h"

namespace midi {

TEST(MidiOut, EnumerateDevices) {
  DeviceInfos devices;
  EXPECT_TRUE(MidiOut::EnumerateDevices(&devices));
#ifdef _DEBUG
  for (auto it = devices.begin(); it != devices.end(); ++it)
    std::cout << (*it)->name() << std::endl;
#endif
}

TEST(MidiIn, EnumerateDevices) {
  DeviceInfos devices;
  EXPECT_TRUE(MidiIn::EnumerateDevices(&devices));
#ifdef _DEBUG
  for (auto it = devices.begin(); it != devices.end(); ++it)
    std::cout << (*it)->name() << std::endl;
#endif
}

TEST(MidiOut, OpenOutputDevice) {
  DeviceInfos devices;
  EXPECT_TRUE(MidiOut::EnumerateDevices(&devices));
  ASSERT_FALSE(devices.empty());

  for (auto it = devices.begin(); it != devices.end(); ++it) {
    unique_ptr<MidiOut> m(MidiOut::Create(*it));
    ASSERT_TRUE(m);
#ifdef _DEBUG
    std::cout << "opened " << m->device()->name() << std::endl;
#endif
  }
}

TEST(MidiIn, OpenInputDevice) {
  DeviceInfos devices;
  EXPECT_TRUE(MidiIn::EnumerateDevices(&devices));
  ASSERT_FALSE(devices.empty());
  shared_ptr<common::ThreadLoop> loop(new common::ThreadLoop());
  for (auto it = devices.begin(); it != devices.end(); ++it) {
    shared_ptr<MidiIn> m(MidiIn::Create(*it, loop));
    ASSERT_TRUE(m);
#ifdef _DEBUG
    std::cout << "opened " << m->device()->name() << std::endl;
#endif
  }

  EXPECT_TRUE(loop->empty());
}

TEST(MidiOut, SendSysExToAxeFx) {
  DeviceInfos devices;
  EXPECT_TRUE(MidiOut::EnumerateDevices(&devices));
  ASSERT_FALSE(devices.empty());

  unique_ptr<MidiOut> axefx;
  for (auto it = devices.begin(); !axefx && it != devices.end(); ++it) {
    if ((*it)->name().find("AXE") != std::string::npos)
      axefx = MidiOut::Create(*it);
  }
  ASSERT_TRUE(axefx);

  // const uint8_t kGetPreset[] = {0xf0, 0x00, 0x01, 0x74, 0x03, 0x13, 0x15, 0xf7};
  // const uint8_t kGetPreset[] = {0xf0, 0x00, 0x01, 0x74, 0x03, 0x1e, 0x01, 0x19, 0xf7};
  const uint8_t kGetPresetName[] =
      {0xF0, 0x00, 0x01, 0x74, 0x03, 0x0F, 0x09, 0xF7};
  unique_ptr<Message> message(
      new Message(&kGetPresetName[0],
                  &kGetPresetName[arraysize(kGetPresetName)]));

  // TODO: Use a timeout thread or something to catch timeouts and end the loop.
  shared_ptr<common::ThreadLoop> loop(new common::ThreadLoop());
  auto fn = std::bind(&common::ThreadLoop::Quit, loop);
  EXPECT_TRUE(axefx->Send(std::move(message), fn));
  EXPECT_TRUE(loop->Run());
}

TEST(Midi, GetPresetName) {
  DeviceInfos out_devices, in_devices;
  EXPECT_TRUE(MidiOut::EnumerateDevices(&out_devices));
  EXPECT_TRUE(MidiIn::EnumerateDevices(&in_devices));
  ASSERT_FALSE(out_devices.empty());
  ASSERT_FALSE(in_devices.empty());

  unique_ptr<MidiOut> out_device;
  for (auto it = out_devices.begin(); !out_device && it != out_devices.end();
       ++it) {
    if ((*it)->name().find("AXE") != std::string::npos)
      out_device = MidiOut::Create(*it);
  }

  shared_ptr<common::ThreadLoop> loop(new common::ThreadLoop());
  shared_ptr<MidiIn> in_device;
  for (auto it = in_devices.begin(); !in_device && it != in_devices.end();
       ++it) {
    if ((*it)->name().find("AXE") != std::string::npos)
      in_device = MidiIn::Create(*it, loop);
  }

  ASSERT_TRUE(out_device);
  ASSERT_TRUE(in_device);

  const uint8_t kGetPresetName[] =
      {0xF0, 0x00, 0x01, 0x74, 0x03, 0x0F, 0x09, 0xF7};
  unique_ptr<Message> message(
      new Message(&kGetPresetName[0],
                  &kGetPresetName[arraysize(kGetPresetName)]));

  // TODO: Set up expectation for received data.
  // Then quit the loop when the data is received or a timeout occurs.
  auto fn = std::bind(&common::ThreadLoop::Quit, loop);
  EXPECT_TRUE(out_device->Send(std::move(message), fn));

  EXPECT_TRUE(loop->Run());
}

TEST(Midi, GetSystemBankDump) {
  DeviceInfos out_devices, in_devices;
  EXPECT_TRUE(MidiOut::EnumerateDevices(&out_devices));
  EXPECT_TRUE(MidiIn::EnumerateDevices(&in_devices));
  ASSERT_FALSE(out_devices.empty());
  ASSERT_FALSE(in_devices.empty());

  unique_ptr<MidiOut> out_device;
  for (auto it = out_devices.begin(); !out_device && it != out_devices.end();
       ++it) {
    if ((*it)->name().find("AXE") != std::string::npos)
      out_device = MidiOut::Create(*it);
  }

  shared_ptr<common::ThreadLoop> loop(new common::ThreadLoop());
  shared_ptr<MidiIn> in_device;
  for (auto it = in_devices.begin(); !in_device && it != in_devices.end();
       ++it) {
    if ((*it)->name().find("AXE") != std::string::npos)
      in_device = MidiIn::Create(*it, loop);
  }

  ASSERT_TRUE(out_device);
  ASSERT_TRUE(in_device);

  const uint8_t kGetSystemBank[] =
      {0xF0, 0x00, 0x01, 0x74, 0x03, 0x1c, 0x03, 0x19, 0xF7};
  unique_ptr<Message> message(
      new Message(&kGetSystemBank[0],
                  &kGetSystemBank[arraysize(kGetSystemBank)]));

  EXPECT_TRUE(out_device->Send(std::move(message), nullptr));

  // Set the timeout between buffer receives to be 1 second.
  // We'll quit 1 second after receiving the last message.
  loop->set_timeout(std::chrono::milliseconds(1000));
  EXPECT_FALSE(loop->Run());
}

}  // namespace midi
