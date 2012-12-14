// Copyright (c) 2012, Tomas Gunnarsson
// All rights reserved.

#include "common/common_types.h"

#include "lg/lg_parser.h"
#include "lg/lg_utils.h"

#include <algorithm>
#include <iostream>
#include <string>

// TODO: Use MIDICHANNEL variables to get a hint for what channel to assume?
// TODO: Use DEFAULT_BANKLIST to choose a banklist to add banks to.
//    alternatively figure out from the default patch what bank to use as
//    a bank template and find the banklist that has the default bank and add
//    generated banks to that list.

namespace lg {

LgParser::LgParser() {
}

LgParser::~LgParser() {
}

void LgParser::ParseBuffer(LgParserCallback* callback,
                           const char* begin,
                           const char* end) {
  const char* pos = begin;
  while (pos < end) {
    const char* bol = pos;
    if (!FindEol(&pos, end))
      break;
    ProcessLine(bol, pos);
    ++pos;
  }

  // This isn't really a supported scenario right now.
  ASSERT(!patches_.empty());
  if (patches_.empty()) {
    // TODO: Create a default patch, default bank and banklist.
    std::cerr << "No patches found in setup file.  Cannot continue.\n";
    return;
  }

  // Begin quick-n-dirty O(n^2) algos. :-|

  ConnectBanksToBankLists();
  ConnectPatchesToBanks();

  // Prepare a template bank that'll be used to create new banks.
  shared_ptr<Bank> template_bank(
      new Bank(*patches_[0]->bank().get()));
  template_bank->RemoveNonPatchEntries();
  template_bank->SetInheritedFrom(template_bank->name());
  template_bank->SetName("template");

  // Cache the patch names that are in the template.  We'll assign new presets
  // to the new banks using these names.
  std::vector<std::string> template_patch_names(
      template_bank->GetPatchNames());
  std::vector<std::string>::const_iterator current_patch =
      template_patch_names.begin();

  shared_ptr<Bank> new_bank;
  Entries::iterator patch_insert_point = GetPatchInsertPoint();
  // Keep track of how many banks we have now, before we start creating banks,
  // so we'll know how many to add to the entry list.
  size_t bank_size = banks_.size();
  size_t bank_id = bank_size;

  ReservedNames taken_names;
  const axefx::PresetMap& presets = callback->GetPresetMap();
  axefx::PresetMap::const_iterator it = presets.begin();
  for (; it != presets.end(); ++it) {
    Patches::value_type p = LookupPatch(it->second->id());
    if (p.get()) {
      p->Update(*it->second.get());
    } else {
      // Use a template to add a patch.
      p.reset(new Patch(*patches_[0].get()));
      p->SetBank(shared_ptr<Bank>());
      p->Update(*it->second.get());

      if (!new_bank.get()) {
        new_bank.reset(new Bank(*template_bank.get()));
        // Generate a new bank name.
        std::string bank("Bank:");
        bank += std::to_string(bank_id++);
        new_bank->SetName(bank);
        new_bank->bank_list()->AppendBank(new_bank->name());
        banks_.push_back(new_bank);
      }

      p->SetBank(new_bank);
      new_bank->OnPatchNameChange(*current_patch, p->name());
      ++current_patch;
      if (current_patch == template_patch_names.end()) {
        current_patch = template_patch_names.begin();
        new_bank.reset();
      }

      if (taken_names.find(p->name()) != taken_names.end())
        p->SetName(GenerateUniqueName(taken_names, p->name()));
      taken_names.insert(p->name());

      patches_.push_back(p);
      patch_insert_point = entries_.insert(patch_insert_point, p) + 1;
    }
  }

  if (banks_.size() > bank_size) {
    Entries::iterator pos = std::find(entries_.begin(), entries_.end(),
                                      banks_[bank_size - 1]);
    ++pos;
    entries_.insert(pos, banks_.begin() + bank_size, banks_.end());
  }

  for (Entries::const_iterator it = entries_.begin();
       it != entries_.end(); ++it) {
    (*it)->WriteLines(callback);
  }
}

void LgParser::ConnectBanksToBankLists() {
  BankLists::const_iterator bl = bank_lists_.begin();
  for (; bl!= bank_lists_.end(); ++bl) {
    const LgEntry::Lines& lines = (*bl)->lines();
    if (lines.size() > 1) {
      LgEntry::Lines::const_iterator l = lines.begin() + 1;
      for (; l != lines.end(); ++l) {
        for (Banks::iterator b = banks_.begin(); b != banks_.end(); ++b) {
          if ((*b)->name() == *l)
            (*b)->SetBankList(*bl);
        }
      }
    }
  }
}

void LgParser::ConnectPatchesToBanks() {
  for (Banks::iterator b = banks_.begin(); b != banks_.end(); ++b) {
    std::vector<std::string> patch_names = (*b)->GetPatchNames();
    std::vector<std::string>::const_iterator p = patch_names.begin();
    for (; p != patch_names.end(); ++p) {
      Patches::value_type patch(LookupPatch(*p));
      if (patch.get())
        patch->SetBank(*b);
    }
  }
}

LgParser::Patches::value_type LgParser::LookupPatch(
    const std::string& name) {
  // If this turns out to be a perf problem we could build hash maps of
  // name->patch.
  Patches::const_iterator it = patches_.begin();
  for (; it != patches_.end(); ++it) {
    if ((*it)->name() == name)
      return *it;
  }

  return LgParser::Patches::value_type();
}

LgParser::Patches::value_type LgParser::LookupPatch(int preset_id) {
  // If this turns out to be a perf problem we could build hash maps of
  // id->patch.
  Patches::const_iterator it = patches_.begin();
  for (; it != patches_.end(); ++it) {
    if ((*it)->preset() == preset_id)
      return *it;
  }

  return LgParser::Patches::value_type();
}

LgParser::Entries::iterator LgParser::GetPatchInsertPoint() {
  if (patches_.empty())
    return entries_.end();
  Entries::iterator ret = std::find(entries_.begin(), entries_.end(),
                                    patches_.back());
  if (ret != entries_.end())
    ++ret;
  return ret;
}

shared_ptr<LgEntry> LgParser::CreateEntry(const char* line) {
  shared_ptr<LgEntry> ret;
  if (line && IsEntryStart(line)) {
    if (IsPatchStart(line)) {
      patches_.push_back(Patches::value_type(new Patch()));
      ret = patches_.back();
    } else if (IsBankStart(line)) {
      banks_.push_back(Banks::value_type(new Bank()));
      ret = banks_.back();
    } else if (IsBankListStart(line)) {
      bank_lists_.push_back(BankLists::value_type(new BankList()));
      ret = bank_lists_.back();
    }
  }

  if (!ret.get())
    ret.reset(new LgEntry());

  entries_.push_back(ret);

  return ret;
}

void LgParser::ProcessLine(const char* line, const char* eol) {
  if (!current_entry_.get() || IsEntryStart(line) || IsSectionSeparator(line))
    current_entry_ = CreateEntry(line);

  current_entry_->AppendLine(line, eol);
}

}  // namespace lg
