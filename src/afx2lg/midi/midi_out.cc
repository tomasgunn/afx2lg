// Copyright (c) 2012, Tomas Gunnarsson
// All rights reserved.

#include "midi/midi_out.h"

namespace midi {

#ifndef _WIN32
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

}  // namespace midi
