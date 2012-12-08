// Copyright (c) 2012, Tomas Gunnarsson
// All rights reserved.

#include "gtest/gtest.h"

#include "common/common_types.h"
#include "axefx/sysex_types.h"
#include "midi/midi_in.h"
#include "midi/midi_out.h"
#include "test_utils.h"

using std::placeholders::_1;
using std::placeholders::_2;

namespace midi {

typedef std::shared_ptr<common::ThreadLoop> SharedThreadLoop;

namespace {
// TODO: Make these common utility functions.  Right now a copy/paste.
shared_ptr<MidiIn> OpenAxeInput(const SharedThreadLoop& loop) {
  DeviceInfos in_devices;
  MidiIn::EnumerateDevices(&in_devices);
  if (in_devices.empty()) {
    std::cerr << "Didn't find any MIDI input devices\n";
    return nullptr;
  }

  shared_ptr<MidiIn> in_device;
  for (auto it = in_devices.begin(); !in_device && it != in_devices.end();
       ++it) {
    if ((*it)->name().find("AXE") != std::string::npos)
      in_device = MidiIn::Create(*it, loop);
  }

  if (!in_device)
    std::cerr << "Failed to find or open AxeFx's MIDI input device.\n";

  return in_device;
}

unique_ptr<MidiOut> OpenAxeOutput() {
  DeviceInfos out_devices;
  MidiOut::EnumerateDevices(&out_devices);

  if (out_devices.empty()) {
    std::cerr << "Didn't find any MIDI output devices\n";
    return nullptr;
  }

  unique_ptr<MidiOut> out_device;
  for (auto it = out_devices.begin(); !out_device && it != out_devices.end();
       ++it) {
    if ((*it)->name().find("AXE") != std::string::npos)
      out_device = MidiOut::Create(*it);
  }

  if (!out_device)
    std::cerr << "Failed to find or open AxeFx's MIDI output device.\n";

  return std::move(out_device);
}
}  // namespace

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
  axefx::GenericNoDataMessage request(axefx::PRESET_NAME);
  unique_ptr<Message> message(new Message(&request, sizeof(request)));

  // TODO: Use a timeout thread or something to catch timeouts and end the loop.
  shared_ptr<common::ThreadLoop> loop(new common::ThreadLoop());
  auto fn = std::bind(&common::ThreadLoop::Quit, loop);
  EXPECT_TRUE(axefx->Send(std::move(message), fn));
  EXPECT_TRUE(loop->Run());
}

void AssignToBufferAndQuit(std::vector<uint8_t>* array,
                           const shared_ptr<common::ThreadLoop>& loop,
                           const uint8_t* data,
                           size_t size) {
  array->assign(data, data + size);
  loop->Quit();
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
  loop->set_timeout(std::chrono::milliseconds(1000));
  shared_ptr<MidiIn> in_device;
  for (auto it = in_devices.begin(); !in_device && it != in_devices.end();
       ++it) {
    if ((*it)->name().find("AXE") != std::string::npos)
      in_device = MidiIn::Create(*it, loop);
  }

  ASSERT_TRUE(out_device);
  ASSERT_TRUE(in_device);

  axefx::GenericNoDataMessage request(axefx::PRESET_NAME);
  unique_ptr<Message> message(new Message(&request, sizeof(request)));

  std::vector<uint8_t> data;
  in_device->set_ondataavailable(
      std::bind(&AssignToBufferAndQuit, &data, loop, _1, _2));
  EXPECT_TRUE(out_device->Send(std::move(message), nullptr));
  EXPECT_TRUE(loop->Run());
  ASSERT_FALSE(data.empty());
  ASSERT_TRUE(axefx::IsFractalSysEx(&data[0], data.size()));
  auto p = reinterpret_cast<const axefx::FractalSysExHeader*>(&data[0]);
  ASSERT_EQ(axefx::PRESET_NAME, p->function());
  std::string name(reinterpret_cast<const char*>(p + 1),
                    reinterpret_cast<const char*>(&data[data.size() - 2]));
  EXPECT_FALSE(name.empty());
#ifdef _DEBUG
  std::cout << "preset name: " << name << std::endl;
#endif
}

// TODO: Disable by default since there's no definite end point.
// Used to test capturing preset names while scrolling through presets
// on the AxeFx unit.
// NOTE: There appears to be a race in how this works.
//  The problem is that the AxeFX returns the name of the active preset.
//  The active preset may however have changed when our request for the name
//  has arrived, so we might get the wrong name.
//  Perhaps there is a way to get a message+id from the axefx so that we can
//  be sure that the id matches the name.
TEST(Midi, DISABLED_PresetNameMonitor) {
  SharedThreadLoop loop(new common::ThreadLoop());
  shared_ptr<MidiIn> midi_in(OpenAxeInput(loop));
  ASSERT_TRUE(midi_in);
  unique_ptr<MidiOut> midi_out(OpenAxeOutput());
  ASSERT_TRUE(midi_out);

  std::vector<uint8_t> data;
  midi_in->set_ondataavailable(
      std::bind(&AssignToBufferAndQuit, &data, loop, _1, _2));

  while (true) {
    ASSERT_TRUE(loop->Run());
    ASSERT_FALSE(data.empty());
    ASSERT_TRUE(axefx::IsFractalSysEx(&data[0], data.size()));
    auto p = reinterpret_cast<const axefx::FractalSysExHeader*>(&data[0]);
    if (p->function() == axefx::PRESET_CHANGE) {
      axefx::GenericNoDataMessage request(axefx::PRESET_NAME);
      unique_ptr<Message> message(new Message(&request, sizeof(request)));
      ASSERT_TRUE(midi_out->Send(std::move(message), nullptr));
      auto pair = reinterpret_cast<const axefx::SeptetPair*>(p + 1);
      std::cout << pair->As16bit() << " ";
    } else if (p->function() == axefx::PRESET_NAME) {
      std::string name(reinterpret_cast<const char*>(p + 1),
                       reinterpret_cast<const char*>(&data[data.size() - 2]));
      EXPECT_FALSE(name.empty());
      std::cout << name << std::endl;
    }
  }
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
  axefx::BankDumpRequest request(axefx::BankDumpRequest::SYSTEM_BANK);
  unique_ptr<Message> message(new Message(&request, sizeof(request)));

  EXPECT_TRUE(out_device->Send(std::move(message), nullptr));

  // Set the timeout between buffer receives to be 1 second.
  // We'll quit 1 second after receiving the last message.
  loop->set_timeout(std::chrono::milliseconds(1000));
  EXPECT_FALSE(loop->Run());
}

}  // namespace midi
