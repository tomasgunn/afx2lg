// Copyright (c) 2012, Tomas Gunnarsson
// All rights reserved.

#include "common/common_types.h"

#include "lg/lg_entry.h"
#include "lg/lg_parser.h"
#include "lg/lg_utils.h"

#include <iomanip>
#include <regex>
#include <sstream>

namespace lg {

using std::match_results;
using std::regex;
using std::regex_match;

void LgEntry::AppendLine(const char* line, const char* eol) {
  lines_.push_back(std::string(line, eol + 1));  // \n inclusive.
  // LG export files can have superfluous whitespace at the end of names.
  // Let's trim that now.
  std::string& s= lines_.back();
  size_t chars = 0, i = s.length() > 2 ? s.length() - 2 : 0;
  while (i > 0 && isspace(s[i])) {
    --i;
    ++chars;
  }
  if (chars)
    s.erase(i + 1, chars);
}

void LgEntry::WriteLines(LgParserCallback* callback) {
  std::vector<std::string>::const_iterator it = lines_.begin();
  for (; it != lines_.end(); ++it)
    callback->WriteLine(it->c_str(), it->length());
}

void NamedEntry::SetName(const std::string& name) {
  if (!lines_.empty())
    ReplaceEntryName(&lines_[0], name);
  name_ = name;
}

void NamedEntry::AppendLine(const char* line, const char* eol) {
  LgEntry::AppendLine(line, eol);
  if (lines_.size() == 1)
    ParseEntryName(lines_.back(), &name_);
}

void BankList::AppendLine(const char* line, const char* eol) {
  NamedEntry::AppendLine(line, eol);
  if (lines_.size() > 1) {
    std::string& str = lines_.back();
    // Remove the trailing '\n' temporarily.  We'll put it back in WriteLines.
    str.erase(str.end() - 1);
    if (str.empty() || IsComment(str.c_str())) {
      lines_.pop_back();
    }
  }
}

void BankList::WriteLines(LgParserCallback* callback) {
  if (!lines_.empty()) {
    std::vector<std::string>::iterator it = lines_.begin();
    ++it;
    for (; it != lines_.end(); ++it)
      (*it) += '\n';
    lines_.push_back(";-----------------------------------------\n");
  }
  NamedEntry::WriteLines(callback);
}

void BankList::AppendBank(const std::string& bank_name) {
  lines_.push_back(bank_name);
}

////////////////////////////////////////////////////////////////////////////////

void Bank::AppendLine(const char* line, const char* eol) {
  NamedEntry::AppendLine(line, eol);
  if (lines_.size() > 1 && default_preset_.empty()) {
    if (IsDefaultPreset(lines_.back(), &default_preset_)) {
      lines_.pop_back();
    }
  }
}

void Bank::WriteLines(LgParserCallback* callback) {
  if (lines_.empty())
    return;

  std::vector<std::string>::const_iterator it = lines_.begin();
  callback->WriteLine(it->c_str(), it->length());
  ++it;

  if (!inherited_from_name_.empty()) {
    std::string line("DERIVED FROM ");
    line += inherited_from_name_;
    line += '\n';
    // TODO: Just change the callback to use std::string.
    callback->WriteLine(line.c_str(), line.length());
  }

  if (!default_preset_.empty()) {
    std::string line("DEFAULTPRESET ");
    line += default_preset_;
    line += '\n';
    callback->WriteLine(line.c_str(), line.length());
  }

  for (; it != lines_.end(); ++it)
    callback->WriteLine(it->c_str(), it->length());

  if (!IsComment(lines_.back().c_str())) {
    const char separator[] =
        ";---------------------------------------------------------------\n";
    callback->WriteLine(separator, arraysize(separator) - 1);
  }
}

void Bank::SetBankList(const shared_ptr<BankList>& bank_list) {
  ASSERT(!bank_list_.get());
  bank_list_ = bank_list;
}

void Bank::OnPatchNameChange(const std::string& old_name,
                                       const std::string& new_name) {
  if (lines_.empty())
    return;

  if (old_name == default_preset_)
    default_preset_ = new_name;

  Lines::iterator it = lines_.begin() + 1;
  const char* reg_exp = "switch \\d+\\s+\\:\\s+PA\\s+([\\w ]+)\n";
  for (; it != lines_.end(); ++it)
    ReplaceIfMatch(&(*it), reg_exp, old_name.c_str(), new_name.c_str());
}

std::vector<std::string> Bank::GetPatchNames() const {
  std::vector<std::string> ret;
  if (lines_.empty())
    return ret;

  Lines::const_iterator it = lines_.begin() + 1;
  const char* reg_exp = "switch \\d+\\s+\\:\\s+PA\\s+([\\w ]+)\n";
  std::string str;
  for (; it != lines_.end(); ++it) {
    if (ExtractSubstring(*it, reg_exp, &str))
      ret.push_back(str);
  }

  return ret;
}

void Bank::SetInheritedFrom(const std::string& bank_name) {
  inherited_from_name_ = bank_name;
}

void Bank::RemoveNonPatchEntries() {
  if (lines_.empty())
    return;

  Lines::const_iterator it = lines_.begin() + 1;
  regex rx("switch \\d+\\s+\\:\\s+PA\\s+([\\w ]+)\n");
  for (; it != lines_.end(); ++it) {
    match_results<std::string::const_iterator> match;
    if (!regex_match(*it, match, rx))
      it = lines_.erase(it) - 1;
  }
}

////////////////////////////////////////////////////////////////////////////////

int Patch::preset() const {
  ASSERT(preset_ < 0x80);
  return static_cast<int>(preset_) | static_cast<int>(bank_id_) << 7;
}

void Patch::AppendLine(const char* line, const char* eol) {
  NamedEntry::AppendLine(line, eol);
  const std::string& str = lines_.back();

  if (lines_.size() > 1) {
    // TODO(tommi): Right now we are not _really_ aware of different midi
    // channels.  We just hope that the CC and PC values will be for the same
    // midi channel and that'll be the right one.
    int cc, value;
    if (ParseCC(str, &channel_, &cc, &value) && cc == 0) {
      bank_id_ = value;
      cc_index_ = static_cast<int>(lines_.size() - 1);
    } else if (ParseProgramChange(str, &channel_, &value)) {
      preset_ = value;
      pc_index_ = static_cast<int>(lines_.size() - 1);
    }
  }
}

void Patch::SetName(const std::string& name) {
  if (bank_)
    bank_->OnPatchNameChange(name_, name);

  if (lines_.empty()) {
    std::string str(kPatchStart);
    str += " " + name + "\n";
    lines_.push_back(str);
    name_ = name;
  } else {
    NamedEntry::SetName(name);
  }
}

void Patch::Update(const axefx::Preset& p) {
  std::string name(p.name());
  // TODO: We need to also make sure each name is unique with in the list
  // of names for the LG since it relies on names and not IDs to map between
  // patches,banks and banklists.
  CheckNameSizeLimit(&name);

  if (bank_)
    bank_->OnPatchNameChange(name_, name);

  if (lines_.empty()) {
    std::string str(kPatchStart);
    str += " " + name + "\n";
    lines_.push_back(str);
    name_ = name;
    SetPreset(p.id());
  } else {
    NamedEntry::SetName(name);
    if (p.id() != preset())
      SetPreset(p.id());
  }
}

void Patch::SetPreset(int preset_number) {
  ASSERT(!lines_.empty());
  preset_ = (preset_number & 0x7F);
  bank_id_ = preset_number >> 7;

  std::stringstream stream;
  // Same as "+ %02i CC    000 %03i\n".
  stream << "+ " << std::setfill('0') << std::setw(2) << channel_
      << " CC    000 " << std::setw(3) << bank_id_ << std::endl;

  if (cc_index_ == -1) {
    cc_index_ = static_cast<int>(lines_.size());
    if (pc_index_ == -1) {
      lines_.push_back(stream.str());
    } else {
      lines_.insert(lines_.begin() + pc_index_, stream.str());
    }
  } else {
    lines_[cc_index_] = stream.str();
  }

  stream.clear();
  // Same as "+ %02i PC    %03i\n".
  stream << "+ " << std::setfill('0') << std::setw(2) << channel_
         << " PC    " << std::setw(3) << preset_ << std::endl;

  if (pc_index_ == -1) {
    pc_index_ = static_cast<int>(lines_.size());
    lines_.push_back(stream.str());
  } else {
    lines_[pc_index_] = stream.str();
  }
}

void Patch::SetBank(const shared_ptr<Bank>& bank) {
  bank_ = bank;
}

}  // namespace lg
