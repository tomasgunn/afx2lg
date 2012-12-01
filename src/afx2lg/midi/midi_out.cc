// Copyright (c) 2012, Tomas Gunnarsson
// All rights reserved.

#include "midi/midi_out.h"

namespace midi {

MidiOut::MidiOut(const shared_ptr<MidiDeviceInfo>& device) : device_(device) {}

}  // namespace midi
