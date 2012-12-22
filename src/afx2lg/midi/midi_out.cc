// Copyright (c) 2012, Tomas Gunnarsson
// All rights reserved.

#include "midi/midi_out.h"

#include <algorithm>

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

bool Message::IsFractalMessageNoChecksum() const {
  return !empty() && axefx::IsFractalSysExNoChecksum(&at(0), size());
}

bool Message::IsFractalMessageWithChecksum() const {
  return !empty() && axefx::IsFractalSysEx(&at(0), size());
}

bool Message::IsFractalMessageType(axefx::FunctionId fn) const {
  // Not all message types have a checksum.
  if (!IsFractalMessageNoChecksum())
    return false;

  auto header = reinterpret_cast<const axefx::FractalSysExHeader*>(&at(0));
  return header->function() == fn;
}

Message::iterator Message::find(uint8_t i) {
  return std::find(begin(), end(), i);
}

ProgramChange::ProgramChange(uint8_t channel,
                             uint8_t bank_id,
                             uint8_t program) {
  ASSERT(channel < 16);
  ASSERT(bank_id < 0x80);
  ASSERT(program < 0x80);
  reserve(5);
  push_back(0xB0 | channel);  // 0xB0 == CC.
  push_back(0u);
  push_back(bank_id);
  push_back(0xC0 | channel);  // Program Change.
  push_back(program);
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
