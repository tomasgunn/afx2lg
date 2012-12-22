// Copyright (c) 2012, Tomas Gunnarsson
// All rights reserved.

#include "midi/midi_in.h"

#include <windows.h>  // must come first.
#include <mmsystem.h>

#include <iostream>

namespace midi {
class MidiInWin : public MidiIn {
 public:
  MidiInWin(const shared_ptr<MidiDeviceInfo>& device,
            const shared_ptr<common::ThreadLoop>& worker_thread)
      : MidiIn(device, worker_thread),
        midi_in_(NULL),
        headers_(),
        buffers_() {
  }

  virtual ~MidiInWin() {
    Close();
  }

  bool Init(const shared_ptr<MidiInWin>& shared_this) {
    weak_this_ = shared_this;
    ASSERT(shared_this.get() == this);

    MMRESULT res = midiInOpen(&midi_in_, device_->id(),
        reinterpret_cast<DWORD_PTR>(&MidiInCallback),
        reinterpret_cast<DWORD_PTR>(this), CALLBACK_FUNCTION);

    if (res == MMSYSERR_NOERROR) {
      for (int i = 0; i < arraysize(headers_); ++i) {
        headers_[i].dwBufferLength = arraysize(buffers_[i]);
        headers_[i].lpData = &buffers_[i][0];
        res = midiInPrepareHeader(midi_in_, &headers_[i], sizeof(headers_[i]));
        ASSERT(res == MMSYSERR_NOERROR);
        res = midiInAddBuffer(midi_in_, &headers_[i], sizeof(headers_[i]));
        ASSERT(res == MMSYSERR_NOERROR);
      }

      if (res == MMSYSERR_NOERROR)
        res = midiInStart(midi_in_);
    }

    return res == MMSYSERR_NOERROR;
  }

  void Close() {
    if (midi_in_) {
      midiInStop(midi_in_);
      midiInReset(midi_in_);  // Must call before unpreparing the buffers.
      for (int i = 0; i < arraysize(headers_); ++i)
        midiInUnprepareHeader(midi_in_, &headers_[i], sizeof(headers_[i]));

      midiInClose(midi_in_);
      midi_in_ = NULL;
    }
  }

 protected:
  static void OnProcessBuffer(const std::weak_ptr<MidiInWin>& me, MIDIHDR* header) {
    std::shared_ptr<MidiInWin> locked(me.lock());
    if (locked && locked->midi_in_) {
      if (locked->data_available_ != nullptr) {
        locked->data_available_(
            reinterpret_cast<const uint8_t*>(&header->lpData[0]),
            header->dwBytesRecorded);
      }
      MMRESULT res = midiInAddBuffer(locked->midi_in_, header, sizeof(*header));
      ASSERT(res == MMSYSERR_NOERROR);
    }
  }

  void OnLongData(MIDIHDR* header) {
    if (header->dwBytesRecorded) {
      // We need to add the buffer asynchronously. We can't do it from here or
      // we'll deadlock.
      shared_ptr<common::ThreadLoop> worker(worker_.lock());
      if (worker) {
        worker->QueueTask(std::bind(&MidiInWin::OnProcessBuffer, weak_this_,
                          header));
      }
    } else {
      // We get this call when exiting for each buffer.  Let's not add it
      // back, or we will crash :)
    }
  }

  void OnCallback(UINT status, DWORD_PTR msg) {
    switch (status) {
      case MIM_OPEN:
      case MIM_CLOSE:
        break;
      case MIM_DATA:
        break;
      case MIM_LONGDATA:
        OnLongData(reinterpret_cast<MIDIHDR*>(msg));
        break;
      case MIM_LONGERROR:
        break;
      default:
#ifdef _DEBUG
        std::cout << __FUNCTION__ << " " << status << std::endl;
#endif
        break;
    }
  }

  static void CALLBACK MidiInCallback(HMIDIIN midi_in, UINT status, 
      DWORD_PTR instance, DWORD_PTR msg, DWORD time_stamp) {
    MidiInWin* me = reinterpret_cast<MidiInWin*>(instance);
    ASSERT(midi_in == me->midi_in_ || me->midi_in_ == NULL);
    me->OnCallback(status, msg);
  }

  HMIDIIN midi_in_;
  static const int kHeaderCount = 64;
  MIDIHDR headers_[kHeaderCount];
  char buffers_[kHeaderCount][4096];
  std::weak_ptr<MidiInWin> weak_this_;
};

// static
shared_ptr<MidiIn> MidiIn::Create(
    const shared_ptr<MidiDeviceInfo>& device,
    const shared_ptr<common::ThreadLoop>& worker_thread) {
  shared_ptr<MidiInWin> ret(new MidiInWin(device, worker_thread));
  if (!ret->Init(ret))
    ret.reset();
  return ret;
}

// static
bool MidiIn::EnumerateDevices(DeviceInfos* devices) {
  UINT count = midiInGetNumDevs();
  for (UINT i = 0; i < count; ++i) {
    MIDIINCAPSA caps = {0};
    if (midiInGetDevCapsA(i, &caps, sizeof(caps)) == MMSYSERR_NOERROR) {
      devices->push_back(
          shared_ptr<MidiDeviceInfo>(new MidiDeviceInfo(i, caps.szPname)));
    }
  }
  return true;
}

}  // namespace midi
