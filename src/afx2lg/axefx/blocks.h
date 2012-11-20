// Copyright (c) 2012, Tomas Gunnarsson
// All rights reserved.

#pragma once
#ifndef AXE_FX_BLOCKS_H_
#define AXE_FX_BLOCKS_H_

#include "common_types.h"

namespace axefx {

// TODO: Write a tool (Python?) that generates C++ structures and enums for
// all of the types from the axeml file that ship with AxeEdit.

extern const int kFirstBlockId;
extern const int kLastBlockId;

// TODO: This list is really version specific, so it should take
// more arguments (or be a member method of Preset).
const char* GetAmpName(uint16_t amp_id);

// TODO: This is also version specific.
const char* GetBlockName(uint16_t block_id);

bool IsShunt(uint16_t block_id);

// Represents an effect block's bypass and x/y state per scene.
class BlockSceneState {
 public:
  BlockSceneState(uint16_t bypass_state);
  ~BlockSceneState();

  bool IsBypassedInScene(int scene) const;
  bool IsConfigYEnabledInScene(int scene) const;

 private:
  uint8_t bypass_;
  uint8_t xy_;
};

}  // namespace axefx

#endif
