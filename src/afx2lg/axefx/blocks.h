// Copyright (c) 2012, Tomas Gunnarsson
// All rights reserved.

#pragma once
#ifndef AXE_FX_BLOCKS_H_
#define AXE_FX_BLOCKS_H_

#include "common_types.h"

namespace axefx {

extern const int kFirstBlockId;
extern const int kLastBlockId;

const char* GetAmpName(uint16_t amp_id);
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
