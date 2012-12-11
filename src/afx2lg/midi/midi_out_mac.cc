// Copyright (c) 2012, Tomas Gunnarsson
// All rights reserved.

#include "midi/midi_out.h"

#include "midi/midi_mac.h"

#include <CoreMIDI/CoreMIDI.h>
#include <CoreAudio/HostTime.h>
#include <CoreServices/CoreServices.h>

#include <iostream>

namespace midi {
class MidiOutMac : public MidiOut {
 public:
  MidiOutMac(const shared_ptr<MidiDeviceInfo>& device)
      : MidiOut(device),
        midi_out_(NULL),
        end_point_(NULL) {
  }

  virtual ~MidiOutMac() {
    Close();
  }

  bool Init() {
    end_point_ = MIDIGetDestination(device_->id());
    if (!end_point_)
      return false;

    OSStatus result = MIDIOutputPortCreate(
        MacMidiClient::instance()->client(), CFSTR("afx2lg_out"), &midi_out_);
    return result == noErr;
  }

  void Close() {
    end_point_ = NULL;
    if (midi_out_) {
      MIDIPortDispose(midi_out_);
      midi_out_ = NULL;
    }
  }

  // MidiOut implementation.

  virtual bool Send(unique_ptr<Message> message,
                    const std::function<void()>& on_complete) {
    ASSERT(!message->empty());

    MIDISysexSendRequest* sysex = new MIDISysexSendRequest();
    memset(sysex, 0, sizeof(*sysex));
    sysex->destination = end_point_;
    sysex->data = &(message->at(0));
    sysex->bytesToSend = message->size();
    sysex->complete = false;
    sysex->completionProc = &OnDone;

    auto owner = new MessageBufferOwner(message, on_complete);
    sysex->completionRefCon = owner;
    OSStatus result = MIDISendSysex(sysex);

    if (result != noErr) {
      owner->CancelCallback();
      delete owner;
      delete sysex;
    }

    return result == noErr;
  }

 protected:
  static void OnDone(MIDISysexSendRequest* request) {
    MessageBufferOwner* buffer =
        reinterpret_cast<MessageBufferOwner*>(request->completionRefCon);
    delete buffer;
    delete request;
  }

  MIDIPortRef midi_out_;
  MIDIEndpointRef end_point_;
};

// static
unique_ptr<MidiOut> MidiOut::Create(const shared_ptr<MidiDeviceInfo>& device) {
  unique_ptr<MidiOutMac> ret(new MidiOutMac(device));
  if (!ret->Init())
    ret.reset();
  return std::move(ret);
}

// static
bool MidiOut::EnumerateDevices(DeviceInfos* devices) {
  ItemCount count = MIDIGetNumberOfDestinations();
  for (ItemCount i = 0; i < count; ++i) {
    MIDIEndpointRef dest = MIDIGetDestination(i);
    CFStringRef name = ConnectedEndpointName(dest);
    char name_buffer[0xff] = {0};
    CFStringGetCString(name, name_buffer, sizeof(name_buffer),
        kCFStringEncodingUTF8 );
    CFRelease(name);
    devices->push_back(
        shared_ptr<MidiDeviceInfo>(new MidiDeviceInfo(i, name_buffer)));
  }
  return true;
}

}  // namespace midi
