// Copyright (c) 2012, Tomas Gunnarsson
// All rights reserved.

#pragma once
#ifndef MIDI_MIDI_OUT_H_
#define MIDI_MIDI_OUT_H_

#include "common_types.h"

#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace midi {

using std::tr1::shared_ptr;
using std::unique_ptr;

class MidiDeviceInfo {
 public:
  MidiDeviceInfo(int id, const std::string& name) : id_(id), name_(name) {}
  ~MidiDeviceInfo() {}

  int id() const { return id_; }
  const std::string& name() const { return name_; }

 private:
  int id_;
  std::string name_;
};

typedef std::vector<shared_ptr<MidiDeviceInfo> > DeviceInfos;
typedef std::vector<uint8_t> Message;

// Interface class for a midi-out connection + device enumeration.
class MidiOut {
 public:
  virtual ~MidiOut() {}

  // Create an instance of MidiOut.
  static unique_ptr<MidiOut> Create(const shared_ptr<MidiDeviceInfo>& device);

  // Enumerate all midi output devices.
  static bool EnumerateDevices(DeviceInfos* devices);

  // Send assumes ownership of the message.
  virtual bool Send(unique_ptr<Message> message,
                    const std::function<void()>& on_complete) = 0;

  const shared_ptr<MidiDeviceInfo>& device() const { return device_; }

 protected:
  MidiOut(const shared_ptr<MidiDeviceInfo>& device);
  shared_ptr<MidiDeviceInfo> device_;
};

}  // namespace midi

#endif  // MIDI_MIDI_OUT_H_
