// Copyright (c) 2012, Tomas Gunnarsson
// All rights reserved.

#include "axefx/axe_fx_sysex_parser.h"
#include "lg/lg_parser.h"

#include <climits>
#include <fstream>
#include <iostream>

bool ReadFileIntoBuffer(const std::string& path, std::unique_ptr<char>* out,
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
  explicit LgSetupFileWriter(const axefx::PresetMap& presets)
      : presets_(presets) {}
  ~LgSetupFileWriter() {}

  virtual void WriteLine(const char* line, int length) {
    std::cout << std::string(line, length);
  }

  virtual const axefx::PresetMap& GetPresetMap() {
    return presets_;
  }

 private:
  const axefx::PresetMap& presets_;
  DISALLOW_COPY_AND_ASSIGN(LgSetupFileWriter);
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
  explicit SysExFileParam(const std::string& str) : path_(str) {}
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

bool FileExists(const std::string& path) {
  std::ifstream file(path);
  return file.good();
}

bool PromptUser(const std::string& prompt, std::string* ret) {
  std::cout << prompt;
  ret->clear();
  char ch;
  while (std::cin.get(ch)) {
    if (ch == 27)  // Esc
      return false;
    if (ch == '\n')
      break;
    ret->push_back(ch);
  }
  return true;
}

bool GetFileNameFromStdIn(const std::string& prompt, std::string* path) {
  do {
    if (!PromptUser(prompt, path))
      return false;
    if (FileExists(*path)) {
      break;
    } else if (!path->empty() && path->at(0) != '\"') {
      path->insert(0, "\"");
      path->append("\"");
      if (FileExists(*path))
        break;
    }
    std::cout << "Unable to open " << *path << std::endl;
  } while (true);

  return true;
}

// TODO: Support a way to generate better and custom bank names.
bool ParseArgs(int argc,
               char* argv[],
               std::vector<SysExFileParam>* syx_files,
               std::string* input_template,
               bool* did_prompt) {
  *did_prompt = false;
  SysExFileParam* prev_sysex = NULL;

  for (int i = 1; i < argc; ++i) {
    const char* arg = argv[i];
    if (arg[0] != '-' || strlen(arg) < 4 || arg[2] != '=') {
      std::cerr << "Unknown/malformed argument: '" << arg << "'\n\n";
      return false;
    } else if (arg[1] == '?') {
      // bail straight away and print the usage instructions.
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

  if (input_template->empty()) {
    *did_prompt = true;
    GetFileNameFromStdIn("I need a path to a LittleGiant exported text file: ",
        input_template);
  }

  if (syx_files->empty()) {
    *did_prompt = true;
    std::string path, range;
    GetFileNameFromStdIn("I need a path to a preset file or bank (.syx): ",
        &path);
    if (!PromptUser("Enter a preset range or * for all: ", &range))
      range.clear();
    SysExFileParam entry(path);
    if (!range.empty() && range != "*")
      entry.SetRange(range.c_str());
    syx_files->push_back(entry);
  }

  return !syx_files->empty() && !input_template->empty();
}

int main(int argc, char* argv[]) {
  std::vector<SysExFileParam> syx_files;
  std::string input_template;
  bool did_prompt = false;
  if (!ParseArgs(argc, argv, &syx_files, &input_template, &did_prompt)) {
    PrintUsage();
    return -1;
  }

  std::filebuf output_stream;
  std::streambuf* original_streambuf = nullptr;
  if (did_prompt) {
    while (!output_stream.is_open()) {
      std::string out_file;
      PromptUser("Enter an output file name:", &out_file);
      bool overwrite = false;
      if (FileExists(out_file)) {
        std::string answer;
        PromptUser("Overwrite the existing file (y/n)? ", &answer);
        if (answer != "y" && answer != "Y")
          continue;
      }
      output_stream.open(out_file, std::ios::out);
    }
    original_streambuf = std::cout.rdbuf(&output_stream);
  }

  axefx::PresetMap presets;
  axefx::SysExParser parser;
  for (size_t i = 0; i < syx_files.size(); ++i) {
    std::unique_ptr<char> buffer;
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
  std::unique_ptr<char> buffer;
  size_t size = 0;
  if (ReadFileIntoBuffer(input_template, &buffer, &size)) {
    LgSetupFileWriter callback(presets);
    lg_parser.ParseBuffer(&callback, buffer.get(), buffer.get() + size);
  } else {
    std::cerr << "Failed to open " << input_template << std::endl;
  }

  if (original_streambuf)
    std::cout.rdbuf(original_streambuf);

  return 0;
}
