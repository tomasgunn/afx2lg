// Copyright (c) 2012, Tomas Gunnarsson
// All rights reserved.

#include "midi/midi_in.h"

#include <windows.h>  // must come first.
#include <mmsystem.h>

#include <iostream>

#pragma warning(disable: 4351)

namespace midi {
class MidiInWin : public MidiIn {
 public:
  MidiInWin(const shared_ptr<MidiDeviceInfo>& device)
      : MidiIn(device),
        midi_in_(NULL),
        headers_(),
        buffers_() {
  }

  virtual ~MidiInWin() {
    Close();
  }

  bool Init() {
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
#if 1
      // Doing this will cause a crash in the driver at shutdown :-(
      for (int i = 0; i < arraysize(headers_); ++i)
        midiInUnprepareHeader(midi_in_, &headers_[i], sizeof(headers_[i]));
#endif

      midiInClose(midi_in_);
      midi_in_ = NULL;
    }
  }

  // MidiIn implementation.


 protected:
  void OnLongData(MIDIHDR* header) {
    if (header->dwBytesRecorded) {
      std::cout << std::hex;
      for (DWORD i = 0; i < header->dwBytesRecorded; ++i) {
        std::cout << (static_cast<uint32_t>(header->lpData[i]) & 0xFF) << " ";
      }
      std::cout << std::dec << std::endl;
      MMRESULT res = midiInAddBuffer(midi_in_, header, sizeof(*header));
      ASSERT(res == MMSYSERR_NOERROR);
    } else {
      std::cout << "No bytes recorded, not pushing buffer back, assuming shutdown.";
    }
  }

  void OnCallback(UINT status, DWORD_PTR msg) {
    switch (status) {
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
        std::cout << __FUNCTION__ << " " << msg << std::endl;
#endif
        break;
    }
  }

  static void CALLBACK MidiInCallback(HMIDIIN midi_in, UINT status, 
      DWORD_PTR instance, DWORD_PTR msg, DWORD time_stamp) {
    if (status == MIM_LONGERROR) {
      // We can get callbacks here at exitprocess time when everything has
      // been closed and deleted :-/
      return;
    }
    MidiInWin* me = reinterpret_cast<MidiInWin*>(instance);
    ASSERT(midi_in == me->midi_in_ || me->midi_in_ == NULL);
    me->OnCallback(status, msg);
  }


  HMIDIIN midi_in_;
  static const int kHeaderCount = 64;
  MIDIHDR headers_[kHeaderCount];
  char buffers_[kHeaderCount][4096];
};

// static
unique_ptr<MidiIn> MidiIn::Create(const shared_ptr<MidiDeviceInfo>& device) {
  unique_ptr<MidiInWin> ret(new MidiInWin(device));
  if (!ret->Init())
    ret.reset();
  return std::move(ret);
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
