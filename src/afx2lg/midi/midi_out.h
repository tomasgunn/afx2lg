// Copyright (c) 2012, Tomas Gunnarsson
// All rights reserved.

#pragma once
#ifndef MIDI_MIDI_OUT_H_
#define MIDI_MIDI_OUT_H_

#include "common/common_types.h"

#include <functional>
#include <string>
#include <vector>

namespace axefx {
struct FractalSysExHeader;
}  // namespace axefx

namespace midi {

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

class DeviceInfos : public std::vector<shared_ptr<MidiDeviceInfo> > {
 public:
  DeviceInfos();
  ~DeviceInfos();

  const_iterator FindAxeFx() const;
};

class Message : public std::vector<uint8_t> {
 public:
  Message();
  Message(const axefx::FractalSysExHeader* header, size_t size);

  bool IsSysEx() const;
  bool IsFractalMessageNoChecksum() const;
  bool IsFractalMessageWithChecksum() const;

  // Returns npos on failure, index of the found item on success.
  iterator find(uint8_t i);

 private:
  Message(const Message&);
  Message& operator=(const Message&);
};

// Interface class for a midi-out connection + device enumeration.
class MidiOut {
 public:
  virtual ~MidiOut() {}

  static unique_ptr<MidiOut> OpenAxeFx();

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

// Used for owning a message buffer and deliver a callback when
// a message has been sent.
class MessageBufferOwner {
 public:
  MessageBufferOwner(unique_ptr<Message>& message,
                     const std::function<void()>& on_complete);
  ~MessageBufferOwner();

  void CancelCallback();

 private:
  std::function<void()> on_complete_;
  unique_ptr<Message> message_;
};

}  // namespace midi

#endif  // MIDI_MIDI_OUT_H_
