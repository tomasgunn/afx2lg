// Copyright (c) 2012, Tomas Gunnarsson
// All rights reserved.

#pragma once
#ifndef MIDI_MIDI_IN_H_
#define MIDI_MIDI_IN_H_

#include "common/common_types.h"
#include "common/thread_loop.h"
#include "midi/midi_out.h"  // for MidiDeviceInfo.

namespace midi {

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

// This is an in-between class that receives callbacks from a MidiIn
// implementation and watches for an end-of-sysex byte and forwards whole
// sysex buffers over to a supplied callback.
// The callback of this class is slightly different from the |DataAvailable|
// callback so that the caller can swap the Message buffer over to another
// container without having to allocate more memory.
class SysExDataBuffer {
 public:
  typedef std::function<void(Message*)> OnSysEx;

  SysExDataBuffer(const OnSysEx& on_sysex);
  ~SysExDataBuffer();

  void Attach(const shared_ptr<MidiIn>& midi_in);

 private:
  void OnData(const uint8_t* data, size_t size);

  OnSysEx on_sysex_;
  Message buffer_;
};

}  // namespace midi

#endif  // MIDI_MIDI_IN_H_
