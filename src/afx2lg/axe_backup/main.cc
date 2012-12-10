// Copyright (c) 2012, Tomas Gunnarsson
// All rights reserved.

#include "common/common_types.h"

#include "axefx/sysex_types.h"
#include "midi/midi_in.h"
#include "midi/midi_out.h"

#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>

using axefx::BankDumpRequest;
using std::placeholders::_1;
using std::placeholders::_2;
using std::shared_ptr;
using std::unique_ptr;

typedef std::shared_ptr<common::ThreadLoop> SharedThreadLoop;

void PrintUsage() {
  std::cerr <<
      "Usage:\n\n"
      "  axebackup [-a] [-b] [-c] [-s]\n"
      "\n"
      "    -a     Creates a backup of bank A (presets 0-127).\n"
      "           The file will be stored in the current directory with the\n"
      "           name BankA_yyyymmdd_hhmm.syx.\n"
      "\n"
      "    -b,-c  Same as -a but for banks B and C.\n"
      "\n"
      "    -s     Back up system data and global blocks.\n"
      "\n"
      "If no arguments are given, a backup will be created for all banks and\n"
      "system data.\n\n";
}

struct Options {
  // Constructor sets the program defaults.
  Options() : bank_a(true), bank_b(true), bank_c(true), system(true) {}

  bool bank_a;
  bool bank_b;
  bool bank_c;
  bool system;
};

bool ParseArgs(int argc, char* argv[], Options* options) {
  if (argc < 2)
    return true;

  options->bank_a = false;
  options->bank_b = false;
  options->bank_c = false;
  options->system = false;

  struct {
    const char* name;
    bool* flag;
  } const flags[] = {
    { "-a", &options->bank_a },
    { "-b", &options->bank_b },
    { "-c", &options->bank_c },
    { "-s", &options->system },
  };

  for (int i = 1; i < argc; ++i) {
    std::string arg(argv[i]);
    bool option_known = false;
    for (int j = 0; !option_known && j < arraysize(flags); ++j) {
      if (arg.compare(flags[j].name) == 0) {
        *flags[j].flag = true;
        option_known = true;
      }
    }
    if (!option_known) {
      if (arg.compare("-?") != 0)
        std::cerr << "Unknown option: " << arg << std::endl;
      return false;
    }
  }
  return true;
}

std::string GetDate() {
  time_t raw_time;
  tm timeinfo = {0};
  time(&raw_time);
#ifdef _WIN32
  localtime_s(&timeinfo, &raw_time);
#else
  timeinfo = *localtime(&raw_time);
#endif
  std::stringstream stream;
  // Same as "%d%02d%02d_%02d%02d".
  stream << std::setfill('0') << std::setw(2) << timeinfo.tm_year
         << timeinfo.tm_mon << timeinfo.tm_mday << '_' << timeinfo.tm_hour
         << timeinfo.tm_min;
  return stream.str();
}

shared_ptr<midi::MidiIn> OpenAxeInput(const SharedThreadLoop& loop) {
  midi::DeviceInfos in_devices;
  midi::MidiIn::EnumerateDevices(&in_devices);
  if (in_devices.empty()) {
    std::cerr << "Didn't find any MIDI input devices\n";
    return nullptr;
  }

  shared_ptr<midi::MidiIn> in_device;
  for (auto it = in_devices.begin(); !in_device && it != in_devices.end();
       ++it) {
    if ((*it)->name().find("AXE") != std::string::npos)
      in_device = midi::MidiIn::Create(*it, loop);
  }

  if (!in_device)
    std::cerr << "Failed to find or open AxeFx's MIDI input device.\n";

  return in_device;
}

unique_ptr<midi::MidiOut> OpenAxeOutput() {
  midi::DeviceInfos out_devices;
  midi::MidiOut::EnumerateDevices(&out_devices);

  if (out_devices.empty()) {
    std::cerr << "Didn't find any MIDI output devices\n";
    return nullptr;
  }

  unique_ptr<midi::MidiOut> out_device;
  for (auto it = out_devices.begin(); !out_device && it != out_devices.end();
       ++it) {
    if ((*it)->name().find("AXE") != std::string::npos)
      out_device = midi::MidiOut::Create(*it);
  }

  if (!out_device)
    std::cerr << "Failed to find or open AxeFx's MIDI output device.\n";

  return std::move(out_device);
}

// TODO: Move to common and remove duplicates.
bool FileExists(const std::string& path) {
  std::ifstream file(path);
  return file.good();
}

bool CreateOutputFile(std::string* name, std::ofstream* file) {
  if (FileExists(*name)) {
    size_t i = name->find_last_of('.');
    int count = 0;
    std::string temp;
    do {
      temp = name->substr(0, i);
      temp += "(" + std::to_string(++count) + ")";
      temp += name->substr(i);
    } while (FileExists(temp));
    *name = temp;
  }
  file->open(*name, std::ios::out | std::ios::binary);
  return file->good();
}

void WriteToFile(std::ofstream* file, const SharedThreadLoop& loop,
                 const uint8_t* data, size_t size) {
  // Ignore tempo messages.  If we receive one, we interpret it as being an
  // indication that we've stopped receiving data.
  if (axefx::IsFractalSysExNoChecksum(data, size)) {
    const auto header =
        reinterpret_cast<const axefx::FractalSysExHeader*>(data);
    if (header->function() == axefx::TEMPO_HEARTBEAT) {
      if (file->tellp() > std::ofstream::pos_type(0)) {
        file->close();
        loop->Quit();
      } else {
        std::cerr
            << "Received tempo message before starting to receive dump.\n";
      }
      return;
    } else if (header->function() == axefx::PRESET_ID) {
      const auto preset_hdr = static_cast<const axefx::PresetIdHeader*>(header);
      std::cout << "\nPreset " << preset_hdr->preset_number.As16bit() << ": ";
    }
  }
  file->write(reinterpret_cast<const char*>(data), size);
  std::cout << "#";
}

int main(int argc, char* argv[]) {
  Options options;
  if (!ParseArgs(argc, argv, &options)) {
    PrintUsage();
    return -1;
  }

  std::cout << "Opening MIDI devices...\n";

  SharedThreadLoop loop(new common::ThreadLoop());
  shared_ptr<midi::MidiIn> midi_in(OpenAxeInput(loop));
  unique_ptr<midi::MidiOut> midi_out(OpenAxeOutput());
  if (!midi_in || !midi_out)
    return -1;

  std::string date(GetDate());
  struct {
    std::string description;
    BankDumpRequest::BankId bank_id;
    std::string name;
    std::ofstream file;
    bool enabled;
  } files[] = {
    { "Bank A", BankDumpRequest::BANK_A, "BankA_" + date + ".syx",
      std::ofstream(), options.bank_a },
    { "Bank B", BankDumpRequest::BANK_B, "BankB_" + date + ".syx",
      std::ofstream(), options.bank_b },
    { "Bank C", BankDumpRequest::BANK_C, "BankC_" + date + ".syx",
      std::ofstream(), options.bank_c },
    { "System Bank", BankDumpRequest::SYSTEM_BANK, "System_" + date + ".syx",
      std::ofstream(), options.system },
  };

  // Set up a map from midi message that requests dump, to output file.
  for (int i = 0; i < arraysize(files); ++i) {
    std::ofstream& f = files[i].file;
    if (files[i].enabled && CreateOutputFile(&files[i].name, &f)) {
      std::cout << "\nWriting " << files[i].description << " to "
                << files[i].name << ".\n";
      BankDumpRequest request(files[i].bank_id);
      unique_ptr<midi::Message> message(
          new midi::Message(&request, sizeof(request)));
      midi_in->set_ondataavailable(std::bind(&WriteToFile, &f, loop, _1, _2));
      if (midi_out->Send(std::move(message), nullptr)) {
        // Run until we get a timeout.  When we time out, we assume that the
        // transmission is done.
        loop->set_timeout(std::chrono::milliseconds(1000));
        loop->Run();
        std::cout << "\n" << files[i].name << " ready.\n";
      } else {
        std::cerr << "Failed to send bank request.\n";
        return -1;
      }
    }
  }

  std::cout << "All done\n";

  return 0;
}