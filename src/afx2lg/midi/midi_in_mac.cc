// Copyright (c) 2012, Tomas Gunnarsson
// All rights reserved.

#include "midi/midi_in.h"

#include "midi/midi_mac.h"

#include <CoreMIDI/CoreMIDI.h>
// #include <CoreAudio/HostTime.h>
// #include <CoreServices/CoreServices.h>

#include <iostream>

namespace midi {

class MidiInMac : public MidiIn {
 public:
  MidiInMac(const shared_ptr<MidiDeviceInfo>& device,
            const shared_ptr<base::ThreadLoop>& worker_thread)
      : MidiIn(device, worker_thread),
        midi_in_(NULL) {
  }

  virtual ~MidiInMac() {
    Close();
  }

  bool Init(const shared_ptr<MidiInMac>& shared_this) {
    weak_this_ = shared_this;
    ASSERT(shared_this.get() == this);
    OSStatus result = MIDIInputPortCreate(
        MacMidiClient::instance()->client(), CFSTR("afx2lg_in"),
        &MidiInCallback, this, &midi_in_);
    if (result != noErr)
      return false;

    MIDIEndpointRef end_point = MIDIGetSource(device_->id());
    if (!end_point)
      return false;

    result = MIDIPortConnectSource(midi_in_, end_point, NULL);
    return result == noErr;
  }

  void Close() {
    if (midi_in_) {
      MIDIPortDispose(midi_in_);
      midi_in_ = NULL;
    }
  }

 protected:
  static void OnProcessBuffer(
      const std::weak_ptr<MidiInMac>& me,
      uint8_t* buffer,
      size_t size) {
    std::shared_ptr<MidiInMac> locked(me.lock());
    if (locked) {
      if (locked->data_available_ != nullptr) {
        locked->data_available_(buffer, size);
      }
    }
    delete [] buffer;
  }

  void OnCallback(const MIDIPacketList* packets) {
    shared_ptr<base::ThreadLoop> worker(worker_.lock());
    if (!worker)
      return;

    size_t count = packets->numPackets;
    const MIDIPacket* packet = &packets->packet[0];
    size_t total_size = 0;
    while (count--) {
      total_size += packet->length;
      packet = MIDIPacketNext(packet);
    }

    uint8_t* buffer = new uint8_t[total_size];
    uint8_t* pos = buffer;
    count = packets->numPackets;
    packet = &packets->packet[0];
    while (count--) {
      memcpy(pos, &packet->data[0], packet->length);
      pos += packet->length;
      packet = MIDIPacketNext(packet);
    }

    worker->QueueTask(
        std::bind(&MidiInMac::OnProcessBuffer, weak_this_, buffer, total_size));
  }

  static void MidiInCallback(
      const MIDIPacketList* packets, void* context, void* src_context) {
    MidiInMac* me = reinterpret_cast<MidiInMac*>(context);
    me->OnCallback(packets);
  }

  MIDIPortRef midi_in_;
  std::weak_ptr<MidiInMac> weak_this_;
};

// static
shared_ptr<MidiIn> MidiIn::Create(
    const shared_ptr<MidiDeviceInfo>& device,
    const shared_ptr<base::ThreadLoop>& worker_thread) {
  shared_ptr<MidiInMac> ret(new MidiInMac(device, worker_thread));
  if (!ret->Init(ret))
    ret.reset();
  return ret;
}

// static
bool MidiIn::EnumerateDevices(DeviceInfos* devices) {
  ItemCount count = MIDIGetNumberOfSources();
  for (ItemCount i = 0; i < count; ++i) {
    MIDIEndpointRef src = MIDIGetSource(i);
    CFStringRef name = ConnectedEndpointName(src);
    char name_buffer[0xff] = {0};
    CFStringGetCString(name, name_buffer, sizeof(name_buffer),
        kCFStringEncodingUTF8);
    CFRelease(name);
    devices->push_back(
        shared_ptr<MidiDeviceInfo>(new MidiDeviceInfo(i, name_buffer)));
  }
  return true;
}

}  // namespace midi
