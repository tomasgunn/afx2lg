// Copyright (c) 2012, Tomas Gunnarsson
// All rights reserved.

#include "midi/midi_in.h"

namespace midi {

MidiIn::MidiIn(const shared_ptr<MidiDeviceInfo>& device,
                const shared_ptr<common::ThreadLoop>& worker_thread)
    : device_(device), worker_(worker_thread) {
}

}  // namespace midi
