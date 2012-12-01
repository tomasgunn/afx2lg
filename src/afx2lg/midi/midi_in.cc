// Copyright (c) 2012, Tomas Gunnarsson
// All rights reserved.

#include "midi/midi_in.h"

namespace midi {

MidiIn::MidiIn(const shared_ptr<MidiDeviceInfo>& device) : device_(device) {}

}  // namespace midi
