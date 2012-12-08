// Copyright (c) 2012, Tomas Gunnarsson
// All rights reserved.

#pragma once
#ifndef MIDI_MIDI_IN_H_
#define MIDI_MIDI_IN_H_

#include "common/common_types.h"
#include "common/thread_loop.h"
#include "midi/midi_out.h"  // for MidiDeviceInfo.

#include <memory>

namespace midi {

using std::tr1::shared_ptr;

typedef std::function<void(const uint8_t*, size_t)> DataAvailable;

// Interface class for a midi-in connection + device enumeration.
class MidiIn {
 public:
  virtual ~MidiIn() {}

  // Create an instance of MidiIn.
  static shared_ptr<MidiIn> Create(
      const shared_ptr<MidiDeviceInfo>& device,
      const shared_ptr<common::ThreadLoop>& worker_thread);

  // Enumerate all midi output devices.
  static bool EnumerateDevices(DeviceInfos* devices);

  const shared_ptr<MidiDeviceInfo>& device() const { return device_; }

  void set_ondataavailable(const DataAvailable& data_available) {
    // TODO: Assert that we're on the worker thread.
    data_available_ = data_available;
  }

 protected:
  MidiIn(const shared_ptr<MidiDeviceInfo>& device,
         const shared_ptr<common::ThreadLoop>& worker_thread);

  shared_ptr<MidiDeviceInfo> device_;
  std::weak_ptr<common::ThreadLoop> worker_;
  DataAvailable data_available_;
};

}  // namespace midi

#endif  // MIDI_MIDI_IN_H_
