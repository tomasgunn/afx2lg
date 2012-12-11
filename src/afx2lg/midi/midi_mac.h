// Copyright (c) 2012, Tomas Gunnarsson
// All rights reserved.

#pragma once
#ifndef MIDI_MIDI_MAC_H_
#define MIDI_MIDI_MAC_H_

#include "common/common_types.h"

#include <CoreMIDI/CoreMIDI.h>

namespace midi {

class MacMidiClient {
 public:
  MacMidiClient();
  ~MacMidiClient();

  MIDIClientRef client() const { return client_; }

  static MacMidiClient* instance();

 private:
  MIDIClientRef client_;
};

CFStringRef ConnectedEndpointName( MIDIEndpointRef endpoint );

}  // namespace midi

#endif  // MIDI_MIDI_MAC_H_
