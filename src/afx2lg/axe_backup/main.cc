// Copyright (c) 2012, Tomas Gunnarsson
// All rights reserved.

#include "common/common_types.h"

#include "axefx/preset.h"
#include "axefx/sysex_types.h"
#include "json/writer.h"
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

typedef std::shared_ptr<common::ThreadLoop> SharedThreadLoop;

void PrintUsage() {
  std::cerr <<
      "Usage:\n\n"
      "  axebackup [-a] [-b] [-c] [-s] [-j]\n"
      "\n"
      "    -a     Creates a backup of bank A (presets 0-127).\n"
      "           The file will be stored in the current directory with the\n"
      "           name BankA_yyyymmdd_hhmm.syx.\n"
      "\n"
      "    -b,-c  Same as -a but for banks B and C.\n"
      "\n"
      "    -s     Back up system data and global blocks.\n"
      "\n"
      "    -j     Writes out a JSON file for each bank.\n"
      "\n"
      "If no arguments are given, a backup will be created for all banks and\n"
      "system data.\n\n";
}

struct Options {
  // Constructor sets the program defaults.
  Options()
      : bank_a(true), bank_b(true), bank_c(true), system(true), json(false) {}

  bool bank_a;
  bool bank_b;
  bool bank_c;
  bool system;

  bool json;
};

bool ParseArgs(int argc, char* argv[], Options* options) {
  if (argc < 2)
    return true;

  options->bank_a = false;
  options->bank_b = false;
  options->bank_c = false;
  options->system = false;
  options->json = false;

  struct {
    const char* name;
    bool* flag;
  } const flags[] = {
    { "-a", &options->bank_a },
    { "-b", &options->bank_b },
    { "-c", &options->bank_c },
    { "-s", &options->system },
    { "-j", &options->json },
  };

  for (int i = 1; i < argc; ++i) {
    std::string arg(argv[i]);
    bool option_known = false;
    for (size_t j = 0; !option_known && j < arraysize(flags); ++j) {
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
#if defined(OS_WIN)
  localtime_s(&timeinfo, &raw_time);
#else
  timeinfo = *localtime(&raw_time);
#endif
  std::ostringstream stream;
  // Same as "%d%02d%02d_%02d%02d".
  stream << std::setfill('0') << std::setw(2) << (1900 + timeinfo.tm_year)
         << (timeinfo.tm_mon + 1) << timeinfo.tm_mday << '_' << timeinfo.tm_hour
         << timeinfo.tm_min;
  return stream.str();
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

class BackupWriter {
 public:
  BackupWriter(std::ofstream* file, const SharedThreadLoop& loop)
      : file_(file), loop_(loop), bytes_written_(0u), failed_(false) {
  }

  ~BackupWriter() {
  }

  bool failed() const { return !bytes_written_ || failed_; }
  size_t preset_count() const { return presets_.size(); }

  void OnSysEx(midi::Message* msg) {
    if (failed_)
      return;

    // Check if the message is recognized as a Fractal message.
    // We'll start by ignoring the message checksum.
    if (!msg->IsFractalMessageNoChecksum()) {
      if (bytes_written_) {
        std::ostringstream stream;
        stream << "Received an unrecognized message (size=" << msg->size() <<
            ").\nAre there any other MIDI devices connected to the AxeFx?";
        OnError(stream.str());
      } else {
#ifndef NDEBUG
        std::cout << "Ignoring unrecognized/partial message.\n";
#endif
      }
      return;
    }

    auto header =
        reinterpret_cast<const axefx::FractalSysExHeader*>(&msg->at(0));
    if (header->function() == axefx::TEMPO_HEARTBEAT) {
      if (bytes_written_) {
        file_->close();
        loop_->Quit();
      } else {
#ifndef NDEBUG
        std::cerr
            << "Received tempo message before starting to receive dump.\n";
#endif
      }
      return;
    }

    // From this point on all messages that we expect, should have a
    // valid checksum.
    if (!msg->IsFractalMessageWithChecksum()) {
      std::ostringstream stream;
      stream << "Checksum error on message with function="
             << header->function();
      OnError(stream.str());
      return;
    }

    switch (header->function()) {
      case axefx::PRESET_ID: {
        auto preset_hdr = static_cast<const axefx::PresetIdHeader*>(header);
        std::cout << preset_hdr->preset_number.As16bit() << ": ";

        current_preset_.reset(new axefx::Preset());
        if (!current_preset_->SetPresetId(*preset_hdr, msg->size())) {
          OnError("Failed to parse preset ID header");
          return;
        }
        break;
      }

      case axefx::PRESET_PARAMETERS: {
        if (!current_preset_.get()) {
          OnError("Received out of band preset parameters.");
          return;
        }
        auto param_header =
            static_cast<const axefx::ParameterBlockHeader*>(header);
        if (!current_preset_->AddParameterData(*param_header, msg->size())) {
          OnError("Failed to parse preset parameters");
          return;
        }
        break;
      }

      case axefx::PRESET_CHECKSUM: {
        if (!current_preset_.get()) {
          OnError("Received out of band preset checksum.");
          return;
        }

        auto checksum = static_cast<const axefx::PresetChecksumHeader*>(header);
        if (!current_preset_->Finalize(checksum, msg->size(), true)) {
          ASSERT(current_preset_->valid());
          OnError("Preset checksum verification failed.\n");
          return;
        }
        std::cout << current_preset_->name() << " <verified>\n";
        presets_.push_back(std::move(current_preset_));
        break;
      }

      default:
        OnError("Unrecognized function: " + std::to_string(header->function()));
        return;
    }

    file_->write(reinterpret_cast<const char*>(&msg->at(0)), msg->size());
    bytes_written_ += msg->size();
  }

  std::string ToJson() {
    Json::Value presets;
    for (auto& p : presets_) {
      Json::Value preset;
      p->ToJson(&preset);
      presets.append(preset);
    }
    Json::Value root;
    root["bank"] = presets;
    Json::StyledWriter writer;
    return writer.write(root);
  }

 private:
  void OnError(const std::string& err) {
    if (bytes_written_ == 0u) {
      // Treat errors as just warnings before we actually start to
      // receive data that we expect.  On Mac there can be 'leftovers' in
      // the midi driver that it will give us when we connect.
      std::cerr << "Warning: " << err << std::endl;
      return;
    }
    failed_ = true;
    std::cerr << "Error: " << err << std::endl;
    file_->close();
    loop_->Quit();
  }

  std::ofstream* file_;
  SharedThreadLoop loop_;
  size_t bytes_written_;
  bool failed_;
  unique_ptr<axefx::Preset> current_preset_;
  std::vector<unique_ptr<axefx::Preset> > presets_;
};

int main(int argc, char* argv[]) {
  Options options;
  if (!ParseArgs(argc, argv, &options)) {
    PrintUsage();
    return -1;
  }

  std::cout << "Opening MIDI devices...\n";

  SharedThreadLoop loop(new common::ThreadLoop());
  shared_ptr<midi::MidiIn> midi_in(midi::MidiIn::OpenAxeFx(loop));
  unique_ptr<midi::MidiOut> midi_out(midi::MidiOut::OpenAxeFx());
  if (!midi_in || !midi_out) {
    std::cerr << "Failed to open AxeFx midi devices\n";
    return -1;
  }

  std::string date(GetDate());
  struct {
    std::string description;
    BankDumpRequest::BankId bank_id;
    std::string name;
    std::string json_name;
    bool enabled;
    std::ofstream file;
    std::ofstream json;
  } files[] = {
    { "Bank A", BankDumpRequest::BANK_A,
      "BankA_" + date + ".syx",
      "BankA_" + date + ".json",
      options.bank_a },
    { "Bank B", BankDumpRequest::BANK_B,
      "BankB_" + date + ".syx",
      "BankB_" + date + ".json",
      options.bank_b },
    { "Bank C", BankDumpRequest::BANK_C,
      "BankC_" + date + ".syx",
      "BankC_" + date + ".json",
      options.bank_c },
    { "System Bank", BankDumpRequest::SYSTEM_BANK,
      "System_" + date + ".syx",
      "System_" + date + ".json",
      options.system },
  };

  // Set up a map from midi message that requests dump, to output file.
  for (size_t i = 0; i < arraysize(files); ++i) {
    std::ofstream& f = files[i].file;
    std::ofstream& j = files[i].json;
    if (files[i].enabled &&
        CreateOutputFile(&files[i].name, &f) &&
        CreateOutputFile(&files[i].json_name, &j)) {
      std::cout << "\nWriting " << files[i].description << " to "
                << files[i].name << ".\n";
      BackupWriter writer(&f, loop);
      midi::SysExDataBuffer sysex_buffer(
          std::bind(&BackupWriter::OnSysEx, &writer, _1));
      sysex_buffer.Attach(midi_in);
      BankDumpRequest request(files[i].bank_id);
      unique_ptr<midi::Message> message(
          new midi::Message(&request, sizeof(request)));
      if (midi_out->Send(std::move(message), nullptr)) {
        // Run until we get a timeout.  When we time out, we assume that the
        // transmission is done.
        loop->set_timeout(std::chrono::milliseconds(3 * 1000));
        loop->Run();

        if (writer.failed()) {
          std::cerr <<
            "\nErrors were detected in the backup data.\n\n"
            "It is possible that using other apps, typing or using"
            " the mouse (especially on Mac) can cause MIDI data from"
            " the AxeFx to be dropped and therefore damaging the backup.\n\n"
            "Please try again and make sure the AxeFx isn't busy before you do."
            "\n\n";
          return -1;
        } else if (writer.preset_count() != 128) {
          std::cerr << "Error: Received an unexpected number of presets: "
                    << writer.preset_count() << ".\n";
          return -1;
        }

        j << writer.ToJson();
        j.flush();
        std::cout << "\n" << "Backup " << files[i].name << " ready.\n";
      } else {
        std::cerr << "Failed to send bank request.\n";
        return -1;
      }
    }
  }

  std::cout << "All done\n";

  return 0;
}
