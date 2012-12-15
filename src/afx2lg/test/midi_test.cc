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
using std::placeholders::_3;

namespace midi {

typedef std::shared_ptr<common::ThreadLoop> SharedThreadLoop;

TEST(MidiOut, EnumerateDevices) {
  DeviceInfos devices;
  EXPECT_TRUE(MidiOut::EnumerateDevices(&devices));
#ifndef NDEBUG
  for (auto it = devices.begin(); it != devices.end(); ++it)
    std::cout << (*it)->name() << std::endl;
#endif
}

TEST(MidiIn, EnumerateDevices) {
  DeviceInfos devices;
  EXPECT_TRUE(MidiIn::EnumerateDevices(&devices));
#ifndef NDEBUG
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
    ASSERT_TRUE(m.get() != NULL);
#ifndef NDEBUG
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
    ASSERT_TRUE(m.get() != NULL);
#ifndef NDEBUG
    std::cout << "opened " << m->device()->name() << std::endl;
#endif
  }

  EXPECT_TRUE(loop->empty());
}

TEST(MidiOut, SendSysExToAxeFx) {
  unique_ptr<MidiOut> axefx(MidiOut::OpenAxeFx());
  ASSERT_TRUE(axefx.get() != NULL);

  axefx::GenericNoDataMessage request(axefx::PRESET_NAME);
  unique_ptr<Message> message(new Message(&request, sizeof(request)));

  // TODO: Use a timeout thread or something to catch timeouts and end the loop.
  shared_ptr<common::ThreadLoop> loop(new common::ThreadLoop());
  auto fn = std::bind(&common::ThreadLoop::Quit, loop);
  EXPECT_TRUE(axefx->Send(std::move(message), fn));
  EXPECT_TRUE(loop->Run());
}

bool IsTempoMessage(Message* msg) {
  if (!msg->IsFractalMessage()) {
    std::cerr << "Not a valid Fractal message\n";
    return false;
  }

  auto header = reinterpret_cast<const axefx::FractalSysExHeader*>(&msg->at(0));
  return header->function() == axefx::TEMPO_HEARTBEAT;
}

void AssignToBufferAndQuit(Message* new_message,
                           const shared_ptr<common::ThreadLoop>& loop,
                           Message* message) {
  if (!new_message->IsSysEx()) {
    std::cerr << "Ignoring non-sysex (partial?) message\n";
    return;
  }
  if (IsTempoMessage(new_message)) {
    std::cerr << "Ignoring tempo message\n";
    return;
  }
  std::cout << "Received message of size : " << new_message->size() << "\n";
  new_message->swap(*message);
  std::cout << "Quitting loop.  message size: " << message->size() << "\n";
  loop->Quit();
}

TEST(Midi, GetPresetName) {
  SharedThreadLoop loop(new common::ThreadLoop());
  shared_ptr<MidiIn> in_device(MidiIn::OpenAxeFx(loop));
  ASSERT_TRUE(in_device.get() != NULL);
  unique_ptr<MidiOut> out_device(MidiOut::OpenAxeFx());
  ASSERT_TRUE(out_device.get() != NULL);

  loop->set_timeout(std::chrono::milliseconds(5000));

  axefx::GenericNoDataMessage request(axefx::PRESET_NAME);
  unique_ptr<Message> message(new Message(&request, sizeof(request)));

  Message data;
  SysExDataBuffer buffer(
      std::bind(&AssignToBufferAndQuit, _1, loop, &data));
  buffer.Attach(in_device);
  EXPECT_TRUE(out_device->Send(std::move(message), nullptr));
  EXPECT_TRUE(loop->Run());
  ASSERT_FALSE(data.empty()) << "size: " << data.size();
  ASSERT_TRUE(axefx::IsFractalSysEx(&data[0], data.size()));
  auto p = reinterpret_cast<const axefx::FractalSysExHeader*>(&data[0]);
  ASSERT_EQ(axefx::PRESET_NAME, p->function());
  std::string name(reinterpret_cast<const char*>(p + 1),
                   reinterpret_cast<const char*>(&data[data.size() - 2]));
  EXPECT_FALSE(name.empty());
#ifndef NDEBUG
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
  shared_ptr<MidiIn> midi_in(MidiIn::OpenAxeFx(loop));
  ASSERT_TRUE(midi_in.get() != NULL);
  unique_ptr<MidiOut> midi_out(MidiOut::OpenAxeFx());
  ASSERT_TRUE(midi_out.get() != NULL);

  midi::Message data;
  midi::SysExDataBuffer buffer(
      std::bind(&AssignToBufferAndQuit, _1, loop, &data));
  buffer.Attach(midi_in);

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
  SharedThreadLoop loop(new common::ThreadLoop());
  shared_ptr<MidiIn> midi_in(MidiIn::OpenAxeFx(loop));
  ASSERT_TRUE(midi_in.get() != NULL);
  unique_ptr<MidiOut> midi_out(MidiOut::OpenAxeFx());
  ASSERT_TRUE(midi_out.get() != NULL);

  axefx::BankDumpRequest request(axefx::BankDumpRequest::SYSTEM_BANK);
  unique_ptr<Message> message(new Message(&request, sizeof(request)));

  // Set the timeout between buffer receives to be 1 second.
  // We'll quit 1 second after receiving the last message.
  loop->set_timeout(std::chrono::milliseconds(1000));
  midi::Message data;
  midi::SysExDataBuffer buffer(
      std::bind(&AssignToBufferAndQuit, _1, loop, &data));
  buffer.Attach(midi_in);

  EXPECT_TRUE(midi_out->Send(std::move(message), nullptr));

  // TODO: Tempo messages can keep the loop running indefinetly.
  // For now we have to run the test with sysex set to none.
  // Need to fix that.
  size_t msg_count = 0;
  while (loop->Run())
    ++msg_count;

  std::cout << "Received sysex count: " << msg_count << std::endl;
}

namespace {
class MockMidiIn : public MidiIn {
 public:
  MockMidiIn() : MidiIn(nullptr, nullptr), message_count_(0) {}
  ~MockMidiIn() {}

  void ReportBytes(const uint8_t* data, size_t size) {
    if (size)
      data_available_(data, size);
  }

  int message_count_;
};

void VerifyIsSysEx(Message* msg) {
  EXPECT_FALSE(msg->empty());
  EXPECT_EQ(msg->at(0), axefx::kSysExStart);
  EXPECT_EQ(msg->at(msg->size() - 1), axefx::kSysExEnd);
  EXPECT_TRUE(axefx::IsFractalSysEx(&msg->at(0), msg->size()));
}
}  // namespace

TEST(SysExDataBuffer, Basic) {
  std::unique_ptr<uint8_t> buffer;
  int file_size;
  ASSERT_TRUE(ReadTestFileIntoBuffer("axefx2/9b_A.syx", &buffer, &file_size));

  int original_message_count = 0;
  shared_ptr<MockMidiIn> midi_in(new MockMidiIn());
  SysExDataBuffer sysex_buffer(&VerifyIsSysEx);
  sysex_buffer.Attach(midi_in);
  const int chunk_sizes[] = {1, 10, 16, 65, 100, 1000};
  uint8_t* end = buffer.get() + file_size;
  for (size_t i = 0; i < arraysize(chunk_sizes); ++i) {
    uint8_t* pos = buffer.get();
    while (pos < (end - chunk_sizes[i])) {
      midi_in->ReportBytes(pos, chunk_sizes[i]);
      pos += chunk_sizes[i];
    }
    midi_in->ReportBytes(pos, end - pos);

    if (!original_message_count)
      original_message_count = midi_in->message_count_;

    EXPECT_EQ(midi_in->message_count_, original_message_count);
  }
}

}  // namespace midi
