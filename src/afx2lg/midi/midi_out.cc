// Copyright (c) 2012, Tomas Gunnarsson
// All rights reserved.

#include "midi/midi_out.h"

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

Message::Message(const axefx::FractalSysExHeader* header, size_t size)
    : std::vector<uint8_t>(reinterpret_cast<const uint8_t*>(header),
                           reinterpret_cast<const uint8_t*>(header) + size) {
}

MidiOut::MidiOut(const shared_ptr<MidiDeviceInfo>& device) : device_(device) {}

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
