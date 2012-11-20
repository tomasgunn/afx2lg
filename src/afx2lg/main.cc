// Copyright (c) 2012, Tomas Gunnarsson
// All rights reserved.

#include "stdafx.h"

#include "axefx/axe_fx_sysex_parser.h"
#include "lg/lg_parser.h"

#include <atlfile.h>

bool ReadFileIntoBuffer(const std::string& path, std::auto_ptr<char>* out,
                        size_t* size) {
  CAtlFile file;
  HRESULT hr = file.Create(path.c_str(), GENERIC_READ, FILE_SHARE_READ,
                           OPEN_EXISTING);
  if (SUCCEEDED(hr)) {
    ULONGLONG file_size = 0;
    hr = file.GetSize(file_size);
    if (file_size > 0x1FFFFFFF) {
      hr = HRESULT_FROM_WIN32(ERROR_FILE_TOO_LARGE);
    } else {
      out->reset(new char[static_cast<DWORD>(file_size)]);
      if (!out->get()) {
        hr = E_OUTOFMEMORY;
      } else {
        DWORD read = 0;
        hr = file.Read(out->get(), static_cast<DWORD>(file_size), read);
        *size = read;
      }
    }
  }

  if (FAILED(hr)) {
    out->reset();
    fprintf(stderr, "Failed to open '%hs' (0x%08X)\n\n",
        path.c_str(), hr);
  }

  return SUCCEEDED(hr);
}

class LgSetupFileWriter : public lg::LgParserCallback {
 public:
  LgSetupFileWriter(const PresetMap& presets) : presets_(presets) {}
  ~LgSetupFileWriter() {}

  virtual void WriteLine(const char* line, int length) {
    std::string str(line, length);
    fprintf(stdout, "%hs", str.c_str());
  }

  virtual const PresetMap& GetPresetMap() {
    return presets_;
  }

 private:
  const PresetMap& presets_;
};

void PrintUsage() {
  const char kUsage[] =
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
  fprintf(stderr, "%hs", kUsage);
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
      fprintf(stderr, "Unknown/malformed argument: '%s'\n\n", arg);
      return false;
    } else if (arg[1] == 's') {
      syx_files->push_back(&arg[3]);
    } else if (arg[1] == 't') {
      if (!input_template->empty()) {
        fprintf(stderr, "The template file can only be specified once\n\n");
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
      const byte* b = reinterpret_cast<const byte*>(buffer.get());
      if (!parser.ParseSysExBuffer(b, b + size)) {
        fprintf(stderr, "Failed to parse '%hs'\n\n", syx_files[i].c_str());
        return -1;
      }
    } else {
      fprintf(stderr, "Failed to open '%hs'\n\n", syx_files[i].c_str());
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
    fprintf(stderr, "Failed to open file %s\n", input_template.c_str());
  }

	return 0;
}
