#include <stdafx.h>
#include "lg_parser.h"

#include <algorithm>
#include <locale>
#include <regex>

// TODO: Use MIDICHANNEL variables to get a hint for what channel to assume?
// TODO: Use DEFAULT_BANKLIST to choose a banklist to add banks to.
//    alternatively figure out from the default patch what bank to use as
//    a bank template and find the banklist that has the default bank and add
//    generated banks to that list.

namespace {
const int kMaxNameLength = 16;
const char kSectionStart[] = ";=";
const char kEntryStart[] = "* ";
const char kComentStart[] = ";";
const char kPatchStart[] = "* PATCH :";
const char kBankStart[] = "* BANK :";
const char kBankListStart[] = "* BANKLIST :";

void CheckNameSizeLimit(std::string* name) {
  if (name->size() <= kMaxNameLength)
    return;

  // Start by converting to CamelCapsNaming.
  std::string::iterator it = name->begin();
  for (; it != name->end(); ++it) {
    if (isspace(*it)) {
      std::string::iterator next = it + 1;
      if (next != name->end()) {
        *next = toupper(*next, std::locale::classic());
      }
      it = name->erase(it) - 1;
    }
  }
  
  if (name->size() <= kMaxNameLength)
    return;
}

bool IsSectionSeparator(const char* line) {
  return strncmp(line, kSectionStart, sizeof(kSectionStart) - 1) == 0;
}

bool IsComment(const char* line) {
  return strncmp(line, kComentStart, sizeof(kComentStart) - 1) == 0;
}

bool IsEntryStart(const char* line) {
  return strncmp(line, kEntryStart, sizeof(kEntryStart) - 1) == 0;
}

bool IsPatchStart(const char* line) {
  return strncmp(line, kPatchStart, sizeof(kPatchStart) - 1) == 0;
}

bool IsBankStart(const char* line) {
  return strncmp(line, kBankStart, sizeof(kBankStart) - 1) == 0;
}

bool IsBankListStart(const char* line) {
  return strncmp(line, kBankListStart, sizeof(kBankListStart) - 1) == 0;
}

bool FindEol(const char** ptr, const char* end) {
  while (*ptr < end && *ptr[0] != '\n')
    ++(*ptr);
  return *ptr[0] == '\n';
}

bool ParseInts(const std::string& str, const char* regex, int* ints[], int size) {
  if (str[0] != '+')
    return false;

  std::regex rx(regex);
  std::match_results<std::string::const_iterator> match;
  bool ret = std::regex_match(str, match, rx);
  if (ret) {
    for (int i = 0; i < size; ++i) {
      if (ints[i])
        *ints[i] = atoi(match.str(i + 1).c_str());
    }
  }

  return ret;
}

bool ParseCC(const std::string& str, int* channel, int* cc, int* value) {
  int* ints[] = { channel, cc, value };
  return ParseInts(str, "\\+\\s+(\\d+)\\s+CC\\s+(\\d+)\\s+(\\d+).*\n", ints,
                   arraysize(ints));
}

bool ParseProgramChange(const std::string& str, int* channel, int* preset) {
  int* ints[] = { channel, preset };
  return ParseInts(str, "\\+\\s+(\\d+)\\s+PC\\s+(\\d+).*\n", ints,
                   arraysize(ints));
}

bool ParseEntryName(const std::string& str, std::string* name) {
  std::regex rx("\\*\\s+(\\w+)\\s+:\\s+([\\w ]+)\n");
  std::match_results<std::string::const_iterator> match;
  bool ret = std::regex_match(str, match, rx);
  if (ret)
    *name = match.str(2);
  return ret;
}

bool ReplaceEntryName(std::string* str, const std::string& name) {
  std::regex rx("\\*\\s+\\w+\\s+:\\s+([\\w ]+)\n");
  std::match_results<std::string::const_iterator> match;
  bool ret = std::regex_match(*str, match, rx);
  if (ret)
    str->replace(match.position(1), match.length(1), name);
  return ret;
}

bool ReplaceIfMatch(std::string* str, const char* regex, const char* find,
                    const char* replace) {
  std::regex rx(regex);
  std::match_results<std::string::const_iterator> match;
  bool ret = std::regex_match(*str, match, rx);
  if (ret) {
    ret = (match.str(1).compare(find) == 0);
    if (ret)
      str->replace(match.position(1), match.length(1), replace);
  }
  return ret;
}

bool ExtractSubstring(const std::string& search, const char* regex,
                      std::string* found) {
  std::regex rx(regex);
  std::match_results<std::string::const_iterator> match;
  bool ret = std::regex_match(search, match, rx);
  if (ret)
    *found = match.str(1);
  return ret;
}

bool IsDefaultPreset(const std::string& str, std::string* name) {
  const char regex[] = "DEFAULTPRESET\\s+([\\w ]+)\n";
  return ExtractSubstring(str, regex, name);
}

}  // namespace

void LgParser::LgEntry::AppendLine(const char* line, const char* eol) {
  lines_.push_back(std::string(line, eol + 1));  // \n inclusive.
  // LG export files can have superfluous whitespace at the end of names.
  // Let's trim that now.
  std::string& s= lines_.back();
  int chars = 0, i = s.length() - 2;
  while (i > 0 && isspace(s[i])) {
    --i;
    ++chars;
  }
  if (chars)
    s.erase(i + 1, chars);
}

void LgParser::LgEntry::WriteLines(LgParserCallback* callback) {
  std::vector<std::string>::const_iterator it = lines_.begin();
  for (; it != lines_.end(); ++it)
    callback->WriteLine(it->c_str(), it->length());
}

void LgParser::NamedEntry::SetName(const std::string& name) {
  if (!lines_.empty())
    ReplaceEntryName(&lines_[0], name);
  name_ = name;
}

void LgParser::NamedEntry::AppendLine(const char* line, const char* eol) {
  LgEntry::AppendLine(line, eol);
  if (lines_.size() == 1)
    ParseEntryName(lines_.back(), &name_);
}

void LgParser::BankList::AppendLine(const char* line, const char* eol) {
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

void LgParser::BankList::WriteLines(LgParserCallback* callback) {
  if (!lines_.empty()) {
    std::vector<std::string>::iterator it = lines_.begin();
    ++it;
    for (; it != lines_.end(); ++it)
      (*it) += '\n';
    lines_.push_back(";-----------------------------------------\n");
  }
  NamedEntry::WriteLines(callback);
}

void LgParser::BankList::AppendBank(const std::string& bank_name) {
  lines_.push_back(bank_name);
}

////////////////////////////////////////////////////////////////////////////////

void LgParser::Bank::AppendLine(const char* line, const char* eol) {
  NamedEntry::AppendLine(line, eol);
  if (lines_.size() > 1 && default_preset_.empty()) {
    if (IsDefaultPreset(lines_.back(), &default_preset_)) {
      lines_.pop_back();
    }
  }
}

void LgParser::Bank::WriteLines(LgParserCallback* callback) {
  if (lines_.empty())
    return;

  std::vector<std::string>::const_iterator it = lines_.begin();
  callback->WriteLine(it->c_str(), it->length());
  ++it;

  char buffer[0xff] = {0};
  if (!inherited_from_name_.empty()) {
    sprintf_s(buffer, arraysize(buffer), "DERIVED FROM %hs\n",
              inherited_from_name_.c_str());
    callback->WriteLine(buffer, strlen(buffer));
  }

  if (!default_preset_.empty()) {
    sprintf_s(buffer, arraysize(buffer), "DEFAULTPRESET %hs\n",
              default_preset_.c_str());
    callback->WriteLine(buffer, strlen(buffer));
  }

  for (; it != lines_.end(); ++it)
    callback->WriteLine(it->c_str(), it->length());

  if (!IsComment(lines_.back().c_str())) {
    const char separator[] =
        ";---------------------------------------------------------------\n";
    callback->WriteLine(separator, arraysize(separator) - 1);
  }
}

void LgParser::Bank::SetBankList(const std::shared_ptr<BankList>& bank_list) {
  ASSERT(!bank_list_.get());
  bank_list_ = bank_list;
}

void LgParser::Bank::OnPatchNameChange(const std::string& old_name,
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

std::vector<std::string> LgParser::Bank::GetPatchNames() const {
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

void LgParser::Bank::SetInheritedFrom(const std::string& bank_name) {
  inherited_from_name_ = bank_name;
}

void LgParser::Bank::RemoveNonPatchEntries() {
  if (lines_.empty())
    return;

  Lines::const_iterator it = lines_.begin() + 1;
  std::regex rx("switch \\d+\\s+\\:\\s+PA\\s+([\\w ]+)\n");
  for (; it != lines_.end(); ++it) {
    std::match_results<std::string::const_iterator> match;
    if (!std::regex_match(*it, match, rx))
      it = lines_.erase(it) - 1;
  }
}

////////////////////////////////////////////////////////////////////////////////

int LgParser::Patch::preset() const {
  ASSERT(preset_ < 0x80);
  return static_cast<int>(preset_) | static_cast<int>(bank_id_) << 7;
}

void LgParser::Patch::AppendLine(const char* line, const char* eol) {
  NamedEntry::AppendLine(line, eol);
  const std::string& str = lines_.back();

  if (lines_.size() > 1) {
    // TODO(tommi): Right now we are not _really_ aware of different midi
    // channels.  We just hope that the CC and PC values will be for the same
    // midi channel and that'll be the right one.
    int cc, value;
    if (ParseCC(str, &channel_, &cc, &value) && cc == 0) {
      bank_id_ = value;
      cc_index_ = lines_.size() - 1;
    } else if (ParseProgramChange(str, &channel_, &value)) {
      preset_ = value;
      pc_index_ = lines_.size() - 1;
    }
  }
}

void LgParser::Patch::SetName(const std::string& name) {
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

void LgParser::Patch::Update(const Preset& p) {
  // A pretty naive translation from wide to ascii, but works for our purposes.
  std::string name(p.name.begin(), p.name.end());
  CheckNameSizeLimit(&name);

  if (bank_)
    bank_->OnPatchNameChange(name_, name);

  if (lines_.empty()) {
    std::string str(kPatchStart);
    str += " " + name + "\n";
    lines_.push_back(str);
    name_ = name;
    SetPreset(p.id);
  } else {
    NamedEntry::SetName(name);
    if (p.id != preset())
      SetPreset(p.id);
  }
}

void LgParser::Patch::SetPreset(int preset_number) {
  ASSERT(!lines_.empty());
  preset_ = (preset_number & 0x7F);
  bank_id_ = preset_number >> 7;

  char buffer[0xff] = {0};
  sprintf_s(buffer, arraysize(buffer), "+ %02i CC    000 %03i\n",
            channel_, bank_id_);

  if (cc_index_ == -1) {
    cc_index_ = lines_.size();
    if (pc_index_ == -1) {
      lines_.push_back(std::string(buffer));
    } else {
      lines_.insert(lines_.begin() + pc_index_, buffer);
    }
  } else {
    lines_[cc_index_] = buffer;
  }

  sprintf_s(buffer, arraysize(buffer), "+ %02i PC    %03i\n",
            channel_, preset_);

  if (pc_index_ == -1) {
    pc_index_ = lines_.size();
    lines_.push_back(std::string(buffer));
  } else {
    lines_[pc_index_] = buffer;
  }
}

void LgParser::Patch::SetBank(const std::shared_ptr<Bank>& bank) {
  bank_ = bank;
}

////////////////////////////////////////////////////////////////////////////////

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
  }

  // Begin quick-n-dirty O(n^2) algos. :-|

  ConnectBanksToBankLists();
  ConnectPatchesToBanks();

  // Prepare a template bank that'll be used to create new banks.
  std::shared_ptr<Bank> template_bank(
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

  std::shared_ptr<Bank> new_bank;
  Entries::iterator patch_insert_point = GetPatchInsertPoint();
  // Keep track of how many banks we have now, before we start creating banks,
  // so we'll know how many to add to the entry list.
  size_t bank_size = banks_.size();

  const PresetMap& presets = callback->GetPresetMap();
  PresetMap::const_iterator it = presets.begin();
  for (; it != presets.end(); ++it) {
    Patches::value_type p = LookupPatch(it->second.id);
    if (p.get()) {
      p->Update(it->second);
    } else {
      // Use a template to add a patch.
      p.reset(new Patch(*patches_[0].get()));
      p->SetBank(std::shared_ptr<Bank>());
      p->Update(it->second);

      if (!new_bank.get()) {
        new_bank.reset(new Bank(*template_bank.get()));
        // Generate a new bank name.
        char buffer[0xff] = {0};
        // TODO: It might be nicer to name the bank after the default preset.
        sprintf_s(buffer, arraysize(buffer), "Bank:%s",
            p->name().c_str());
        new_bank->SetName(buffer);
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

std::shared_ptr<LgParser::LgEntry> LgParser::CreateEntry(const char* line) {
  std::shared_ptr<LgEntry> ret;
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
