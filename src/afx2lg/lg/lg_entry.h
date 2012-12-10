// Copyright (c) 2012, Tomas Gunnarsson
// All rights reserved.

#pragma once

#ifndef LG_ENTRY_H_
#define LG_ENTRY_H_

#include "axefx/preset.h"

#include <memory>
#include <vector>

namespace lg {

using std::shared_ptr;

class LgParserCallback;

class LgEntry {
 public:
  typedef std::vector<std::string> Lines;

  LgEntry() {}
  virtual ~LgEntry() {}

  virtual void AppendLine(const char* line, const char* eol);
  virtual void WriteLines(LgParserCallback* callback);

  const Lines& lines() const { return lines_; }

 protected:
  Lines lines_;
};

class NamedEntry : public LgEntry {
 public:
  NamedEntry() {}
  ~NamedEntry() {}

  const std::string& name() const { return name_; }
  virtual void SetName(const std::string& name);

  virtual void AppendLine(const char* line, const char* eol);

 protected:
  std::string name_;
};

class BankList : public NamedEntry {
 public:
  BankList() {}
  virtual ~BankList() {}

  virtual void AppendLine(const char* line, const char* eol);
  virtual void WriteLines(LgParserCallback* callback);

  void AppendBank(const std::string& bank_name);

 private:
};

class Bank : public NamedEntry {
 public:
  Bank() {}
  virtual ~Bank() {}

  virtual void AppendLine(const char* line, const char* eol);
  virtual void WriteLines(LgParserCallback* callback);
  void SetBankList(const shared_ptr<BankList>& bank_list);
  void OnPatchNameChange(const std::string& old_name,
                         const std::string& new_name);
  std::vector<std::string> GetPatchNames() const;
  void SetInheritedFrom(const std::string& bank_name);
  void RemoveNonPatchEntries();

  const shared_ptr<BankList>& bank_list() { return bank_list_; }

 private:
  shared_ptr<BankList> bank_list_;
  std::string inherited_from_name_;
  std::string default_preset_;
};

class Patch : public NamedEntry {
 public:
  Patch()
     : channel_(1), preset_(0), bank_id_(0), cc_index_(-1), pc_index_(-1) {}
  virtual ~Patch() {}
  virtual void AppendLine(const char* line, const char* eol);
  virtual void SetName(const std::string& name);
  void Update(const axefx::Preset& p);
  void SetPreset(int preset_number);
  void SetBank(const shared_ptr<Bank>& bank);

  int preset() const;
  const shared_ptr<Bank> bank() const { return bank_; }

 private:
  int channel_;
  int preset_;
  int bank_id_;
  int cc_index_;
  int pc_index_;
  shared_ptr<Bank> bank_;
};

}  // namespace lg

#endif  // LG_ENTRY_H_
