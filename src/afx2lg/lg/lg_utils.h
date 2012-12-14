// Copyright (c) 2012, Tomas Gunnarsson
// All rights reserved.

#pragma once

#ifndef LG_UTILS_H_
#define LG_UTILS_H_

#include <string>
#include <unordered_set>

namespace lg {

typedef std::unordered_set<std::string> ReservedNames;

extern const char kPatchStart[];
extern const size_t kMaxNameLength;

std::string GenerateUniqueName(const ReservedNames& taken_names,
                               const std::string& original_name);
void CheckNameSizeLimit(std::string* name);
bool IsSectionSeparator(const char* line);
bool IsComment(const char* line);
bool IsEntryStart(const char* line);
bool IsPatchStart(const char* line);
bool IsBankStart(const char* line);
bool IsBankListStart(const char* line);
bool FindEol(const char** ptr, const char* end);
bool ParseCC(const std::string& str, int* channel, int* cc, int* value);
bool ParseProgramChange(const std::string& str, int* channel, int* preset);
bool ParseEntryName(const std::string& str, std::string* name);
bool ReplaceEntryName(std::string* str, const std::string& name);
bool ReplaceIfMatch(std::string* str, const char* reg_exp, const char* find,
                    const char* replace);
bool ExtractSubstring(const std::string& search, const char* reg_exp,
                      std::string* found);
bool IsDefaultPreset(const std::string& str, std::string* name);

}  // namespace lg

#endif  // LG_UTILS_H_
