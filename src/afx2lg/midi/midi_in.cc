// Copyright (c) 2012, Tomas Gunnarsson
// All rights reserved.

#include "midi/midi_in.h"

// todo: remove
#include "axefx/sysex_types.h"

#include <iostream>

using std::placeholders::_1;
using std::placeholders::_2;

static const uint8_t kSysExEnd = 0xF7;
static const uint8_t kSysExStart = 0xF0;

namespace midi {

#if !defined(OS_WIN) && !defined(OS_MACOSX)
// static
shared_ptr<MidiIn> MidiIn::Create(
    const shared_ptr<MidiDeviceInfo>& device,
    const shared_ptr<common::ThreadLoop>& worker_thread) {
  return nullptr;
}

// static
bool MidiIn::EnumerateDevices(DeviceInfos* devices) {
  return false;
}
#endif

MidiIn::MidiIn(const shared_ptr<MidiDeviceInfo>& device,
                const shared_ptr<common::ThreadLoop>& worker_thread)
    : device_(device), worker_(worker_thread) {
}

// static
shared_ptr<MidiIn> MidiIn::OpenAxeFx(
    const shared_ptr<common::ThreadLoop>& worker_thread) {
  DeviceInfos devices;
  EnumerateDevices(&devices);
  DeviceInfos::const_iterator found = devices.FindAxeFx();
  if (found == devices.end())
    return nullptr;
  return MidiIn::Create(*found, worker_thread);
}

SysExDataBuffer::SysExDataBuffer(const SysExDataBuffer::OnSysEx& on_sysex)
    : on_sysex_(std::move(on_sysex)) {
}

SysExDataBuffer::~SysExDataBuffer() {
}

void SysExDataBuffer::Attach(const shared_ptr<MidiIn>& midi_in) {
  midi_in->set_ondataavailable(
      std::bind(&SysExDataBuffer::OnData, this, _1, _2));
}

void SysExDataBuffer::OnData(const uint8_t* data, size_t size) {
  size_t i = 0u;
  size_t pos_begin = 0u;
  for (; i < size; ++i) {
    if (data[i] == kSysExEnd) {
      buffer_.insert(buffer_.end(), &data[pos_begin], &data[i + 1u]);
      ASSERT(buffer_[buffer_.size() - 1u] == kSysExEnd);
      if (buffer_[0] != kSysExStart) {
#ifndef NDEBUG
        std::cerr << "WRN: Received partial midi message.  Dropping.\n";
#endif
        Message::iterator found = buffer_.find(kSysExStart);
        if (found != buffer_.end()) {
#ifndef NDEBUG
          std::cerr << "\tFound another sysex start.\n";
#endif
          buffer_.erase(buffer_.begin(), found);
        } else {
          buffer_.clear();
        }
      }

#ifndef NDEBUG
      if (buffer_.size() > 202) {
        std::cout << "buffer size: " << buffer_.size() << "\n";
        ASSERT(buffer_[0] == kSysExStart);
        for (size_t x = 1; x < (buffer_.size() - 1); ++x) {
          ASSERT(buffer_[x] < 0xF0);
          if (x < (buffer_.size() - sizeof(axefx::kFractalMidiId))) {
            // This can actually happen on Mac.
            if (memcmp(&axefx::kFractalMidiId[0], &buffer_[x],
                       sizeof(axefx::kFractalMidiId)) != 0) {
              std::cerr << "WRN: Found a Fractal header in an unusually large "
                           "message. Preceding byte: " << (int) buffer_[x - 1]
                        << "function: "
                        << buffer_[x +sizeof(axefx::kFractalMidiId)] << "\n";
            }
          }
        }
      }
#endif
      if (!buffer_.empty()) {
        ASSERT(buffer_[0] == kSysExStart);
        on_sysex_(&buffer_);
      }
      buffer_.clear();
      pos_begin = i + 1u;
      ASSERT(pos_begin >= size || data[pos_begin] == kSysExStart);
    }
  }

  if (pos_begin < size)
    buffer_.insert(buffer_.end(), &data[pos_begin], &data[size]);
}

}  // namespace midi
