// Copyright (c) 2012, Tomas Gunnarsson
// All rights reserved.

#include "common/common_types.h"

#include "axefx/axe_fx_sysex_parser.h"
#include "axefx/preset.h"
#include "axefx/sysex_types.h"
#include "axys/MainWnd.h"
#include "midi/midi_in.h"
#include "midi/midi_out.h"

#include "juce/JuceHeader.h"

#include <climits>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <queue>

using juce::String;
/*
using std::placeholders::_1;
using std::placeholders::_2;

typedef std::shared_ptr<common::ThreadLoop> SharedThreadLoop;
typedef std::queue<unique_ptr<midi::Message> > MessageQueue;

void PrintUsage() {
  std::cerr <<
      "Usage:\n\n"
      "  axeloader <path to preset file>\n"
      "\n"
      "\n"
      "The utility will load a preset file into the AxeFx' edit buffer.\n"
      "The preset will not be stored or overwrite an existing preset\n"
      "regardless of the default target ID of the preset file.  You must\n"
      "store the preset yourself where you want it.\n\n";
}

// TODO: Move to common and remove duplicates.
bool FileExists(const std::string& path) {
  std::ifstream file(path);
  return file.good();
}

// TODO: Move to common and remove duplicates.
bool ReadFileIntoBuffer(const std::string& path, unique_ptr<uint8_t>* buffer,
                        size_t* file_size) {
  std::ifstream f;
  f.open(path, std::fstream::in | std::ios::binary);
  if (!f.is_open()) {
    std::cerr << "Failed to open \"" << path << "\".\n";
    std::cerr << "If the path contains spaces, try using quotes around it.\n";
    return false;
  }

  f.seekg(0, std::ios::end);
  std::streampos size = f.tellg();
  f.seekg(0, std::ios::beg);

  if (size >= INT_MAX) {
    std::cerr << "Sorry, that file is too big.\n";
    return false;
  }

  *file_size = static_cast<size_t>(size);
  buffer->reset(new uint8_t[*file_size]);
  f.read(reinterpret_cast<char*>(buffer->get()), *file_size);
  
  return true;
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
  std::cout << "Press any key to continue." << std::endl;
  std::cin.get();
}

int main(int argc, char* argv[]) {
  std::string path;
  if (!ParseArgs(argc, argv, &path)) {
    PrintUsage();
    Wait();
    return -1;
  }

  unique_ptr<uint8_t> buffer;
  size_t size = 0u;
  if (!ReadFileIntoBuffer(path, &buffer, &size)) {
    Wait();
    return -1;
  }

  std::cout << "Opening MIDI devices...\n";

  SharedThreadLoop loop(new common::ThreadLoop());
  shared_ptr<midi::MidiIn> midi_in(midi::MidiIn::OpenAxeFx(loop));
  unique_ptr<midi::MidiOut> midi_out(midi::MidiOut::OpenAxeFx());
  if (!midi_in || !midi_out) {
    std::cerr << "Failed to open AxeFx midi devices\n";
    Wait();
    return -1;
  }

  axefx::SysExParser parser;
  if (!parser.ParseSysExBuffer(buffer.get(), buffer.get() + size, false)) {
    std::cerr << "Failed to parse preset file.\n";
    Wait();
    return -1;
  }

  if (parser.presets().size() == 1u) {
    shared_ptr<axefx::Preset> p(parser.presets().begin()->second);
    p->SetAsEditBuffer();
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
*/

class MainWnd  : public DocumentWindow {
 public:
  MainWnd()
      : DocumentWindow(
          "Axys", Colours::lightgrey,
          DocumentWindow::minimiseButton | DocumentWindow::closeButton,
          true) {
    // Create an instance of our main content component, and add it to our window..
    setContentOwned(new MainView(), true);
    centreWithSize(getWidth(), getHeight());

    setVisible(true);
  }

  ~MainWnd() {
    // (the content component will be deleted automatically)
  }

  void closeButtonPressed() {
    JUCEApplication::quit();
  }
};

class AxysApp : public juce::JUCEApplication {
 public:
  AxysApp() {}
  ~AxysApp() {}

  virtual void initialise(const String& cmd) {
    main_wnd_.reset(new MainWnd());
  }

  virtual void shutdown() {
    main_wnd_.reset();
  }

  virtual const String getApplicationName() {
    return "Axys";
  }

  virtual const String getApplicationVersion() {
    return "0.0.0.1";
  }

  virtual bool moreThanOneInstanceAllowed() {
    return true;
  }

  virtual void anotherInstanceStarted (const String& cmd) {
  }

 private:
  std::unique_ptr<MainWnd> main_wnd_;
};


START_JUCE_APPLICATION(AxysApp)
