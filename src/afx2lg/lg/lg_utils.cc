// Copyright (c) 2012, Tomas Gunnarsson
// All rights reserved.

#include "common/common_types.h"
#include "lg/lg_utils.h"

#include <locale>
#include <regex>

using std::tr1::match_results;
using std::tr1::regex;
using std::tr1::regex_match;

namespace lg {

static const int kMaxNameLength = 16;
static const char kSectionStart[] = ";=";
static const char kEntryStart[] = "* ";
static const char kComentStart[] = ";";
const char kPatchStart[] = "* PATCH :";
static const char kBankStart[] = "* BANK :";
static const char kBankListStart[] = "* BANKLIST :";

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

bool ParseInts(const std::string& str, const char* reg_exp, int* ints[],
               int size) {
  if (str[0] != '+')
    return false;

  regex rx(reg_exp);
  match_results<std::string::const_iterator> match;
  bool ret = regex_match(str, match, rx);
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
  regex rx("\\*\\s+(\\w+)\\s+:\\s+([\\w ]+)\n");
  match_results<std::string::const_iterator> match;
  bool ret = regex_match(str, match, rx);
  if (ret)
    *name = match.str(2);
  return ret;
}

bool ReplaceEntryName(std::string* str, const std::string& name) {
  regex rx("\\*\\s+\\w+\\s+:\\s+([\\w ]+)\n");
  match_results<std::string::const_iterator> match;
  bool ret = regex_match(*str, match, rx);
  if (ret)
    str->replace(match.position(1), match.length(1), name);
  return ret;
}

bool ReplaceIfMatch(std::string* str, const char* reg_exp, const char* find,
                    const char* replace) {
  regex rx(reg_exp);
  match_results<std::string::const_iterator> match;
  bool ret = regex_match(*str, match, rx);
  if (ret) {
    ret = (match.str(1).compare(find) == 0);
    if (ret)
      str->replace(match.position(1), match.length(1), replace);
  }
  return ret;
}

bool ExtractSubstring(const std::string& search, const char* reg_exp,
                      std::string* found) {
  regex rx(reg_exp);
  match_results<std::string::const_iterator> match;
  bool ret = regex_match(search, match, rx);
  if (ret)
    *found = match.str(1);
  return ret;
}

bool IsDefaultPreset(const std::string& str, std::string* name) {
  const char regex[] = "DEFAULTPRESET\\s+([\\w ]+)\n";
  return ExtractSubstring(str, regex, name);
}
}  // namespace lg
