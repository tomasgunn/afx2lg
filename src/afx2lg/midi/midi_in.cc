// Copyright (c) 2012, Tomas Gunnarsson
// All rights reserved.

#include "midi/midi_in.h"

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

}  // namespace midi
