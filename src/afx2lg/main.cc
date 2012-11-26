// Copyright (c) 2012, Tomas Gunnarsson
// All rights reserved.

#include "stdafx.h"

#include "axefx/axe_fx_sysex_parser.h"
#include "lg/lg_parser.h"

#include <climits>
#include <fstream>
#include <iostream>

bool ReadFileIntoBuffer(const std::string& path, std::auto_ptr<char>* out,
                        size_t* file_size) {
  std::ifstream f;
  f.open(path.c_str(), std::fstream::in | std::ios::binary);
  if (!f.is_open())
    return false;

  f.seekg(0, std::ios::end);
  std::streampos size = f.tellg();
  f.seekg(0, std::ios::beg);

  if (size >= INT_MAX)
    return false;

  *file_size = static_cast<size_t>(size);
  out->reset(new char[*file_size]);
  f.read(out->get(), *file_size);
  
  return true;
}

class LgSetupFileWriter : public lg::LgParserCallback {
 public:
  LgSetupFileWriter(const axefx::PresetMap& presets) : presets_(presets) {}
  ~LgSetupFileWriter() {}

  virtual void WriteLine(const char* line, int length) {
    std::cout << std::string(line, length);
  }

  virtual const axefx::PresetMap& GetPresetMap() {
    return presets_;
  }

 private:
  const axefx::PresetMap& presets_;
};

void PrintUsage() {
  std::cerr <<
    "Usage:\n\n"
    "  afx2lg -s=f1.syx [-r=<range>] -t=t.txt\n"
    "\n"
    "    -s     A .syx SysEx patch or bank file for AxeFx II.\n"
    "           You can specify multiple such files in one go.\n"
    "\n"
    "    -r     Preset ID range and/or a comma separated list of IDs.\n"
    "           Example: -r=0-12,30,45,20-25,100\n"
    "           If omitted, all presets are included\n"
    "\n"
    "    -t     Specify the template file that will be used to\n"
    "           generate a setup file that can be imported into\n"
    "           LG Control Center.  You create this file by exporting\n"
    "           your setup file from LG Control Center as a via the\n"
    "           'File->Export to...->Text...' command.\n"
    "\n"
    "The generated output will be written to stdout, so just pipe it\n"
    "to a file of your choosing.\n\n"
    "Example:\n\n"
    "  afx2lg -s=BankA.syx -s=lead.syx -s=BankB.syx -t=input.txt > out.txt\n"
    "\n"
    "Then you import the output file [out.txt] in Control Center by using\n"
    "the 'File->Import from...->Text...' command.\n\n";
}

class IDRange {
 public:
  IDRange(const std::string& str) : first_(-1), last_(-1) {
    std::string::size_type i = str.find('-');
    if (i == std::string::npos) {
      first_ = std::atoi(str.c_str());
    } else {
      first_ = std::atoi(str.substr(0, i).c_str());
      last_ = std::atoi(str.substr(i + 1).c_str());
    }

    if (last_ != -1 && first_ > last_)
      std::swap(first_, last_);
  }

  bool Matches(int id) const {
    if (last_ == -1)
      return id == first_;
    return id >= first_ && id <= last_;
  }

 private:
  int first_, last_;
};

class SysExFileParam {
 public:
  explicit SysExFileParam(const char* str) : path_(str) {}
  const std::string& path() const { return path_; }

  void SetRange(const char* range) {
    std::vector<std::string> entries;
    const char* pos = range;
    while (*pos != '\0') {
      if (*pos == ',') {
        entries.push_back(std::string(range, pos - range));
        range = pos + 1;
      }
      ++pos;\
    }
    entries.push_back(std::string(range, pos - range));
    std::vector<std::string>::iterator i = entries.begin();
    for (; i != entries.end(); ++i)
      ranges_.push_back(IDRange(*i));
  }

  bool ShouldIncludePreset(int id) const {
    if (ranges_.empty())
      return true;
    std::vector<IDRange>::const_iterator i = ranges_.begin();
    for (; i != ranges_.end(); ++i) {
      if (i->Matches(id))
        return true;
    }
    return false;
  }

 private:
  std::string path_;
  std::vector<IDRange> ranges_;
};

// TODO: Support a way to generate better and custom bank names.
bool ParseArgs(int argc,
               char* argv[],
               std::vector<SysExFileParam>* syx_files,
               std::string* input_template) {
  if (argc < 3)
    return false;

  SysExFileParam* prev_sysex = NULL;

  for (int i = 1; i < argc; ++i) {
    const char* arg = argv[i];
    if (arg[0] != '-' || strlen(arg) < 4 || arg[2] != '=') {
      std::cerr << "Unknown/malformed argument: '" << arg << "'\n\n";
      return false;
    } else if (arg[1] == 's') {
      syx_files->push_back(SysExFileParam(&arg[3]));
      prev_sysex = &syx_files->back();
    } else if (arg[1] == 'r') {
      if (!prev_sysex) {
        std::cerr << "Range parameter doesn't match a sysex file\n\n";
        return false;
      }
      prev_sysex->SetRange(&arg[3]);
    } else if (arg[1] == 't') {
      if (!input_template->empty()) {
        std::cerr << "The template file can only be specified once\n\n";
        return false;
      }
      *input_template = &arg[3];
    }
  }

  return !syx_files->empty() && !input_template->empty();
}

int main(int argc, char* argv[]) {
  std::vector<SysExFileParam> syx_files;
  std::string input_template;
  if (!ParseArgs(argc, argv, &syx_files, &input_template)) {
    PrintUsage();
    return -1;
  }

  axefx::PresetMap presets;
  axefx::SysExParser parser;
  for (size_t i = 0; i < syx_files.size(); ++i) {
    std::auto_ptr<char> buffer;
    size_t size = 0;
    if (ReadFileIntoBuffer(syx_files[i].path(), &buffer, &size)) {
      const uint8_t* b = reinterpret_cast<const uint8_t*>(buffer.get());
      if (!parser.ParseSysExBuffer(b, b + size)) {
        std::cerr << "Failed to parse " << syx_files[i].path() << std::endl;
        return -1;
      }
      axefx::PresetMap::const_iterator it = parser.presets().begin();
      for (; it != parser.presets().end(); ++it) {
        if (syx_files[i].ShouldIncludePreset(it->first))
          presets.insert(*it);
      }
    } else {

      std::cerr << "Failed to open " << syx_files[i].path() << std::endl;
      return -1;
    }
  }

  lg::LgParser lg_parser;
  std::auto_ptr<char> buffer;
  size_t size = 0;
  if (ReadFileIntoBuffer(input_template, &buffer, &size)) {
    LgSetupFileWriter callback(presets);
    lg_parser.ParseBuffer(&callback, buffer.get(), buffer.get() + size);
  } else {
    std::cerr << "Failed to open " << input_template << std::endl;
  }

	return 0;
}
