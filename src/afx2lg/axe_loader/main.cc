// Copyright (c) 2012, Tomas Gunnarsson
// All rights reserved.

#include "common/common_types.h"

#include "axefx/axe_fx_sysex_parser.h"
#include "axefx/preset.h"
#include "axefx/sysex_types.h"
#include "common/file_utils.h"
#include "midi/midi_in.h"
#include "midi/midi_out.h"

#include <climits>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <queue>

using base::FileExists;
using base::ReadFileIntoBuffer;
using base::SharedThreadLoop;

using std::placeholders::_1;
using std::placeholders::_2;

typedef std::queue<unique_ptr<midi::Message> > MessageQueue;

void PrintUsage() {
  std::cerr <<
      "Usage:\n\n"
      "  axeloader <path to .syx file>\n"
      "\n"
      "\n"
      "For single presets, the utility will load the preset file into the\n"
      "edit buffer.  The preset will not be stored or overwrite an existing\n"
      "preset regardless of the default target ID of the preset file.\n"
      "You must store the preset yourself where you want it.\n"
      "\n"
      "When sending preset archives (backup files) to the AxeFx, all presets\n"
      "in the archive will be restored to their original location.\n"
      "NOTE: This means overwriting the current preset at that location.\n"
      "\n"
      "IR/Cab files are sent directly to the AxeFx.  This means that they'll\n"
      "be stored in the currently selected cab slot.\n"
      "\n"
      "Firmware files can be sent to the AxeFx but you'll be prompted before\n"
      "the data is sent\n"
      "\n";
}

bool ParseArgs(int argc, char* argv[], std::string* path) {
  if (argc < 2) {
    std::cerr << "Missing path to preset file\n";
    return false;
  }

  *path = argv[1];

  return true;
}

void SerializeCallback(const std::vector<uint8_t>& data, MessageQueue* out) {
  unique_ptr<midi::Message> message(
      new midi::Message(static_cast<const midi::Message&>(data)));
  out->push(std::move(message));
}

unique_ptr<midi::Message> PopMessage(MessageQueue* q) {
  unique_ptr<midi::Message> m(std::move(q->front()));
  q->pop();
  return m;
}

void SendMessage(const SharedThreadLoop& loop,
                 midi::MidiOut* midi_out,
                 MessageQueue* q,
                 std::function<void()>& on_complete) {
  if (q->empty()) {
    loop->Quit();
  } else {
    midi_out->Send(PopMessage(q), on_complete);
  }
}

void QueueNext(const SharedThreadLoop& loop,
               midi::MidiOut* midi_out,
               MessageQueue* q) {
  if (q->empty()) {
    loop->Quit();
  } else {
    std::function<void()> on_complete(std::bind(&QueueNext, loop, midi_out, q));
    loop->QueueTask(std::bind(&SendMessage, loop, midi_out, q, on_complete));
    std::cout << "#";
  }
}

void Wait() {
  std::cin.sync();
  std::cout << "Press Enter to continue." << std::endl;
  std::cin.get();
}

// TODO: Move this function to a common utility file.
bool IsTempoOrTuner(midi::Message* msg) {
  return msg->IsFractalMessageType(axefx::TEMPO_HEARTBEAT) ||
         msg->IsFractalMessageType(axefx::TUNER_DATA);
}

// TODO: Move this function to a common utility file.
void AssignToBufferAndQuit(midi::Message* new_message,
                           const shared_ptr<base::ThreadLoop>& loop,
                           midi::Message* message) {
  if (!new_message->IsSysEx()) {
    std::cerr << "Ignoring non-sysex (partial?) message\n";
    return;
  }

  new_message->swap(*message);
  ASSERT(!message->empty());
  loop->Quit();
}

bool SwitchToFwUpdatePage(const shared_ptr<midi::MidiIn>& midi_in,
                          const unique_ptr<midi::MidiOut>& midi_out,
                          const SharedThreadLoop& loop) {
  // Switch to the FW update screen.
  std::cout << "Switching unit to firmware update mode.\n";

  axefx::GenericNoDataMessage request(axefx::FIRMWARE_UPDATE);
  unique_ptr<midi::Message> message(
      new midi::Message(&request, sizeof(request)));

  midi::Message data;
  midi::SysExDataBuffer buffer(
      std::bind(&AssignToBufferAndQuit, _1, loop, &data));
  midi::ScopedBufferAttach scoped_attach(midi_in, &buffer);
  if (!midi_out->Send(std::move(message), nullptr)) {
    std::cerr << "Failed to send a midi message.\n";
    return false;
  }

  // Wait until we get a confirmation.
  while (loop->Run()) {
    if (!IsTempoOrTuner(&data)) {
      if (axefx::IsFractalSysEx(&data[0], data.size()))
        break;
    }
    data.clear();
  }

  if (!axefx::IsFractalSysEx(&data[0], data.size())) {
    std::cerr << "Didn't receive a valid confirmation message\n";
    return false;
  }

  auto p = reinterpret_cast<const axefx::FractalSysExHeader*>(&data[0]);
  auto r = static_cast<const axefx::ReplyMessage*>(p);
  if (p->function() != axefx::REPLY ||
      r->reply_to() != axefx::FIRMWARE_UPDATE ||
      r->error_id != 0) {
    std::cerr << "Failed to switch to fw update mode. "
                 "You may need to reboot the AxeFx\n";
    return false;
  }

  return true;
}

int main(int argc, char* argv[]) {
  std::string path;
  if (!ParseArgs(argc, argv, &path)) {
    PrintUsage();
    Wait();
    return -1;
  }

  unique_ptr<uint8_t[]> buffer;
  size_t size = 0u;
  if (!ReadFileIntoBuffer(path, &buffer, &size)) {
    std::cerr << "Failed to open file '" << path << "'\n";
    Wait();
    return -1;
  }

  axefx::SysExParser parser;
  if (!parser.ParseSysExBuffer(buffer.get(), buffer.get() + size, false)) {
    std::cerr << "Failed to parse preset file.\n";
    Wait();
    return -1;
  }

  if (parser.type() == axefx::SysExParser::PRESET) {
    shared_ptr<axefx::Preset> p(parser.presets().begin()->second);
    p->SetAsEditBuffer();
  } else if (parser.type() == axefx::SysExParser::FIRMWARE) {
    std::cout << "Warning: This is a firmware file. Are you sure you want to "
        "send it to the AxeFx? (y/n)" << std::endl;
    std::string str;
    std::cin >> str;
    if (str.length() != 1 || (str[0] != 'y' && str[0] != 'Y'))
      return 0;
  }

  std::cout << "Opening MIDI devices...\n";

  SharedThreadLoop loop(new base::ThreadLoop());
  shared_ptr<midi::MidiIn> midi_in(midi::MidiIn::OpenAxeFx(loop));
  unique_ptr<midi::MidiOut> midi_out(midi::MidiOut::OpenAxeFx());
  if (!midi_in || !midi_out) {
    std::cerr << "Failed to open AxeFx midi devices\n";
    Wait();
    return -1;
  }

  if (parser.type() == axefx::SysExParser::FIRMWARE) {
    if (!SwitchToFwUpdatePage(midi_in, midi_out, loop)) {
      Wait();
      return -1;
    }
  }

  std::cout << "Sending data...\n";

  MessageQueue messages;
  if (!parser.Serialize(std::bind(&SerializeCallback, _1, &messages))) {
    std::cerr << "An error occurred while sending sysex data.\n";
    Wait();
    return -1;
  }

  QueueNext(loop, midi_out.get(), &messages);
  loop->Run();

  std::cout << "\n\nAll done\n";

  return 0;
}
