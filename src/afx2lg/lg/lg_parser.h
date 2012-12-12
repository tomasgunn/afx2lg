// Copyright (c) 2012, Tomas Gunnarsson
// All rights reserved.

#pragma once

#ifndef LG_PARSER_H_
#define LG_PARSER_H_

// TODO: it's not that clean to have LG depend on AxeFx.
#include "axefx/axe_fx_sysex_parser.h"
#include "axefx/preset.h"
#include "lg/lg_entry.h"

namespace lg {

class LgParserCallback {
 public:
  virtual const axefx::PresetMap& GetPresetMap() = 0;
  virtual void WriteLine(const char* line, size_t length) = 0;
};

class LgParser {
 public:
  LgParser();
  ~LgParser();

  void ParseBuffer(LgParserCallback* callback, const char* begin,
      const char* end);

 protected:
  typedef std::vector<shared_ptr<LgEntry> > Entries;
  typedef std::vector<shared_ptr<Patch> > Patches;
  typedef std::vector<shared_ptr<Bank> > Banks;
  typedef std::vector<shared_ptr<BankList> > BankLists;

  void ProcessLine(const char* line, const char* end);
  shared_ptr<LgEntry> CreateEntry(const char* line);
  void ConnectBanksToBankLists();
  void ConnectPatchesToBanks();
  Patches::value_type LookupPatch(const std::string& name);
  Patches::value_type LookupPatch(int preset_id);
  Entries::iterator GetPatchInsertPoint();

 private:
  Entries entries_;
  shared_ptr<LgEntry> current_entry_;

  Patches patches_;
  Banks banks_;
  BankLists bank_lists_;
};

}  // namespace lg

#endif  // LG_PARSER_H_
