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
    "  afx2lg -s=f1.syx [-s=f2.syx [-s=f.syx [...]]] -t=t.txt\n"
    "\n"
    "    -s     A .syx SysEx patch or bank file for AxeFx II.\n"
    "           You can specify multiple such file in one go.\n"
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
    "  afx2lg -s=BankA.syx -s=BankB.syx -s=BankC.syx -t=input.txt > out.txt\n\n"
    "Then you import the output file [out.txt] in Control Center by using\n"
    "the 'File->Import from...->Text...' command.\n\n";
}

bool ParseArgs(int argc,
               char* argv[],
               std::vector<std::string>* syx_files,
               std::string* input_template) {
  if (argc < 3)
    return false;

  for (int i = 1; i < argc; ++i) {
    const char* arg = argv[i];
    if (arg[0] != '-' || strlen(arg) < 4 || arg[2] != '=') {
      std::cerr << "Unknown/malformed argument: '" << arg << "'\n\n";
      return false;
    } else if (arg[1] == 's') {
      syx_files->push_back(&arg[3]);
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
  std::vector<std::string> syx_files;
  std::string input_template;
  if (!ParseArgs(argc, argv, &syx_files, &input_template)) {
    PrintUsage();
    return -1;
  }

  axefx::SysExParser parser;
  for (size_t i = 0; i < syx_files.size(); ++i) {
    std::auto_ptr<char> buffer;
    size_t size = 0;
    if (ReadFileIntoBuffer(syx_files[i], &buffer, &size)) {
      const uint8_t* b = reinterpret_cast<const uint8_t*>(buffer.get());
      if (!parser.ParseSysExBuffer(b, b + size)) {
        std::cerr << "Failed to parse " << syx_files[i] << std::endl;
        return -1;
      }
    } else {
      std::cerr << "Failed to open " << syx_files[i] << std::endl;
      return -1;
    }
  }

  lg::LgParser lg_parser;
  std::auto_ptr<char> buffer;
  size_t size = 0;
  if (ReadFileIntoBuffer(input_template, &buffer, &size)) {
    LgSetupFileWriter callback(parser.presets());
    lg_parser.ParseBuffer(&callback, buffer.get(), buffer.get() + size);
  } else {
    std::cerr << "Failed to open " << input_template << std::endl;
  }

	return 0;
}
