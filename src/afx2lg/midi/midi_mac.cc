// Copyright (c) 2012, Tomas Gunnarsson
// All rights reserved.

#include "midi/midi_mac.h"

#include <CoreMIDI/CoreMIDI.h>
#include <CoreServices/CoreServices.h>

namespace midi {

MacMidiClient::MacMidiClient() : client_(nullptr) {
  MIDIClientCreate(CFSTR("afx2lg"), NULL, NULL, &client_);
}

MacMidiClient::~MacMidiClient() {
  MIDIClientDispose(client_);
}

// static
MacMidiClient* MacMidiClient::instance() {
  // NOTE: This is not thread safe so we assume some control from the outside.
  static MacMidiClient client;
  return &client;
}

// This function was submitted by Douglas Casey Tucker and apparently
// derived largely from PortMidi.
CFStringRef EndpointName(MIDIEndpointRef endpoint, bool is_external) {
  CFMutableStringRef result = CFStringCreateMutable(NULL, 0);
  CFStringRef str = NULL;
  MIDIObjectGetStringProperty(endpoint, kMIDIPropertyName, &str);
  if (str != NULL) {
    CFStringAppend(result, str);
    CFRelease(str);
  }

  MIDIEntityRef entity = NULL;
  MIDIEndpointGetEntity(endpoint, &entity);
  if (!entity)
    return result;

  if (CFStringGetLength(result) == 0) {
    // endpoint name has zero length -- try the entity
    str = NULL;
    MIDIObjectGetStringProperty(entity, kMIDIPropertyName, &str);
    if (str != NULL) {
      CFStringAppend(result, str);
      CFRelease(str);
    }
  }

  // now consider the device's name
  MIDIDeviceRef device = NULL;
  MIDIEntityGetDevice(entity, &device);
  if (!device)
    return result;

  str = NULL;
  MIDIObjectGetStringProperty(device, kMIDIPropertyName, &str);
  if (CFStringGetLength(result) == 0) {
    CFRelease(result);
    return str;
  }

  if (str != NULL) {
    // if an external device has only one entity, throw away
    // the endpoint name and just use the device name
    if (is_external && MIDIDeviceGetNumberOfEntities(device) < 2) {
      CFRelease(result);
      return str;
    } else {
      if (CFStringGetLength(str) == 0) {
        CFRelease(str);
        return result;
      }
      // does the entity name already start with the device name?
      // (some drivers do this though they shouldn't)
      // if so, do not prepend
        if (CFStringCompareWithOptions(
                result, str, CFRangeMake(0, CFStringGetLength(str)), 0) !=
            kCFCompareEqualTo) {
        // prepend the device name to the entity name
        if (CFStringGetLength(result) > 0)
          CFStringInsert(result, 0, CFSTR(" "));
        CFStringInsert(result, 0, str);
      }
      CFRelease(str);
    }
  }
  return result;
}

// This function was submitted by Douglas Casey Tucker and apparently
// derived largely from PortMidi.
CFStringRef ConnectedEndpointName(MIDIEndpointRef endpoint) {
  CFMutableStringRef result = CFStringCreateMutable(NULL, 0);

  // Does the endpoint have connections?
  CFDataRef connections = NULL;
  bool anyStrings = false;
  OSStatus err = MIDIObjectGetDataProperty(
      endpoint, kMIDIPropertyConnectionUniqueID, &connections);
  if (connections != NULL) {
    // It has connections, follow them
    // Concatenate the names of all connected devices
    int count = CFDataGetLength(connections) / sizeof(MIDIUniqueID);
    if (count) {
      const SInt32* pid = (const SInt32 *)(CFDataGetBytePtr(connections));
      for (int i = 0; i < count; ++i, ++pid) {
        MIDIUniqueID id = EndianS32_BtoN(*pid);
        MIDIObjectRef connObject;
        MIDIObjectType connObjectType;
        err = MIDIObjectFindByUniqueID(id, &connObject, &connObjectType);
        if (err == noErr) {
          CFStringRef str;
          if (connObjectType == kMIDIObjectType_ExternalSource  ||
              connObjectType == kMIDIObjectType_ExternalDestination) {
            // Connected to an external device's endpoint (10.3 and later).
            str = EndpointName((MIDIEndpointRef)(connObject), true);
          } else {
            // Connected to an external device (10.2) (or something else, catch-
            str = NULL;
            MIDIObjectGetStringProperty(connObject, kMIDIPropertyName, &str);
          }
          if (str != NULL) {
            if (anyStrings) {
              CFStringAppend(result, CFSTR(", "));
            } else {
              anyStrings = true;
            }
            CFStringAppend(result, str);
            CFRelease(str);
          }
        }
      }
    }
    CFRelease(connections);
  }

  if (anyStrings)
    return result;

  // Here, either the endpoint had no connections, or we failed to obtain names 
  return EndpointName(endpoint, false);
}

}  // namespace midi
