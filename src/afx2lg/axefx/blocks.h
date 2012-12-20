// Copyright (c) 2012, Tomas Gunnarsson
// All rights reserved.

#pragma once
#ifndef AXE_FX_BLOCKS_H_
#define AXE_FX_BLOCKS_H_

#include "common/common_types.h"
#include "axefx/axefx_ii_ids.h"

#include <vector>

namespace Json {
class Value;
}

namespace axefx {

extern const int kFirstBlockId;

bool BlockSupportsXY(AxeFxBlockType type);

#ifdef _DEBUG
// TODO: This list is really version specific, so it should take
// more arguments (or be a member method of Preset).
const char* GetAmpName(uint16_t amp_id);
#endif

#pragma pack(push)
#pragma pack(1)

// Represents an effect block's bypass and x/y state per scene.
class BlockSceneState {
 public:
  explicit BlockSceneState(uint16_t bypass_state);
  ~BlockSceneState();

  bool IsBypassedInScene(int scene) const;
  bool IsConfigYEnabledInScene(int scene) const;

  void ToJson(bool supports_xy, Json::Value* out) const;

 private:
  uint8_t bypass_;
  uint8_t xy_;
};

class BlockInMatrix {
 public:
  BlockInMatrix();

  bool is_shunt() const;
  AxeFxIIBlockID block() const { return static_cast<AxeFxIIBlockID>(block_); }
  const uint16_t& input_mask() const { return input_mask_; }

  void ToJson(Json::Value* out) const;

 private:
  uint16_t block_;
  // |input_mask| is a bit mask of 4 bits that shows how the current block
  // receives its input from the previous column.
  // 0010 means the current block connects with the block at x-1,1.
  // 1001 means connections with blocks x-1,0 and x-1,3.
  uint16_t input_mask_;
};

const size_t kMatrixRows = 4u;
const size_t kMatrixColumns = 12u;
typedef BlockInMatrix Matrix[kMatrixColumns][kMatrixRows];

#pragma pack(pop)

enum BlockConfig {
  CONFIG_X,
  CONFIG_Y,
};

class BlockParameters {
 public:
  BlockParameters();
  ~BlockParameters();

  // Populates the block parameters from a 16bit value array.
  // Returns the number of 16bit items eaten or 0 if the buffer
  // wasn't big enough.
  size_t Initialize(const uint16_t* data, size_t count);

  AxeFxBlockType type() const;
  AxeFxIIBlockID block() const;

  bool supports_xy() const;
  size_t param_count() const;
  bool is_modifier() const;
  BlockConfig active_config() const { return config_; }
  int global_block_index() const { return global_block_index_; }

  uint16_t GetParamValue(int index, bool get_x_value) const;

  BlockSceneState GetBypassState() const;

  void ToJson(Json::Value* out) const;

 private:
  AxeFxIIBlockID block_;
  BlockConfig config_;
  int global_block_index_;
  std::vector<uint16_t> params_;
};

}  // namespace axefx

#endif
