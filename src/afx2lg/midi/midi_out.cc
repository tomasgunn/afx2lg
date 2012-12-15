// Copyright (c) 2012, Tomas Gunnarsson
// All rights reserved.

#include "midi/midi_out.h"

#include "axefx/sysex_types.h"

namespace midi {

#if !defined(OS_WIN) && !defined(OS_MACOSX)
// static
unique_ptr<MidiOut> MidiOut::Create(const shared_ptr<MidiDeviceInfo>& device) {
  return nullptr;
}

// static
bool MidiOut::EnumerateDevices(DeviceInfos* devices) {
  return false;
}

#endif

DeviceInfos::DeviceInfos() {}
DeviceInfos::~DeviceInfos() {}

DeviceInfos::const_iterator DeviceInfos::FindAxeFx() const {
  for (auto it = begin(); it != end(); ++it) {
    if ((*it)->name().find("AXE") != std::string::npos)
      return it;
  }
  return end();
}

Message::Message() {}

Message::Message(const axefx::FractalSysExHeader* header, size_t size)
    : std::vector<uint8_t>(reinterpret_cast<const uint8_t*>(header),
                           reinterpret_cast<const uint8_t*>(header) + size) {
}

bool Message::IsSysEx() const {
  return !empty() && at(0) == axefx::kSysExStart;
}

bool Message::IsFractalMessage() const {
  return !empty() && axefx::IsFractalSysExNoChecksum(&at(0), size());
}

MidiOut::MidiOut(const shared_ptr<MidiDeviceInfo>& device) : device_(device) {}

// static
unique_ptr<MidiOut> MidiOut::OpenAxeFx() {
  DeviceInfos devices;
  EnumerateDevices(&devices);
  DeviceInfos::const_iterator found = devices.FindAxeFx();
  if (found == devices.end())
    return nullptr;

  return Create(*found);
}

MessageBufferOwner::MessageBufferOwner(
    unique_ptr<Message>& message, const std::function<void()>& on_complete)
    : on_complete_(on_complete), message_(std::move(message)) {
}

MessageBufferOwner::~MessageBufferOwner() {
  if (on_complete_ != nullptr)
    on_complete_();
}

void MessageBufferOwner::CancelCallback() {
  on_complete_ = nullptr;
}

}  // namespace midi
