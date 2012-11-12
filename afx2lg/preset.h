#pragma once
#ifndef PRESET_H_

#include <map>

struct Preset {
  int id;
  std::wstring name;
  // this might grow... or not.
};

// Preset ID/number -> name.
typedef std::map<int, Preset> PresetMap;

#endif  // PRESET_H_
