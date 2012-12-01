// Copyright (c) 2012, Tomas Gunnarsson
// All rights reserved.

#include "gtest/gtest.h"

#include "common_types.h"
#include "midi/midi_in.h"
#include "midi/midi_out.h"
#include "test_utils.h"

#include <windows.h>
#include <WinUser.h>

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
  for (auto it = devices.begin(); it != devices.end(); ++it) {
    unique_ptr<MidiIn> m(MidiIn::Create(*it));
    ASSERT_TRUE(m);
#ifdef _DEBUG
    std::cout << "opened " << m->device()->name() << std::endl;
#endif
  }
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

  EXPECT_TRUE(axefx->Send(std::move(message)));
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

  unique_ptr<MidiIn> in_device;
  for (auto it = in_devices.begin(); !in_device && it != in_devices.end();
       ++it) {
    if ((*it)->name().find("AXE") != std::string::npos)
      in_device = MidiIn::Create(*it);
  }

  ASSERT_TRUE(out_device);
  ASSERT_TRUE(in_device);

  const uint8_t kGetPresetName[] =
      {0xF0, 0x00, 0x01, 0x74, 0x03, 0x0F, 0x09, 0xF7};
  unique_ptr<Message> message(
      new Message(&kGetPresetName[0],
                  &kGetPresetName[arraysize(kGetPresetName)]));

  EXPECT_TRUE(out_device->Send(std::move(message)));
  Sleep(1000);
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

  unique_ptr<MidiIn> in_device;
  for (auto it = in_devices.begin(); !in_device && it != in_devices.end();
       ++it) {
    if ((*it)->name().find("AXE") != std::string::npos)
      in_device = MidiIn::Create(*it);
  }

  ASSERT_TRUE(out_device);
  ASSERT_TRUE(in_device);

  const uint8_t kGetSystemBank[] =
      {0xF0, 0x00, 0x01, 0x74, 0x03, 0x1c, 0x03, 0x19, 0xF7};
  unique_ptr<Message> message(
      new Message(&kGetSystemBank[0],
                  &kGetSystemBank[arraysize(kGetSystemBank)]));

  EXPECT_TRUE(out_device->Send(std::move(message)));

  Sleep(60 * 1000);
}

}  // namespace midi
