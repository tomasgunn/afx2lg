// Copyright (c) 2012, Tomas Gunnarsson
// All rights reserved.

#include "axefx/blocks.h"
#include "json/value.h"

#include <algorithm>

namespace axefx {

const int kFirstBlockId = 100;

inline bool IsBitSet(uint8_t byte, int index) {
  ASSERT(index >= 0 && index < 8);
  return (byte >> index) & 1;
}

inline void SetBit(uint8_t* byte, int index, bool set) {
  ASSERT(index >= 0 && index < 8);
  set ? ((*byte) |= 1 << index) : ((*byte) &= ~(1 << index));
}

bool BlockSupportsXY(AxeFxBlockType type) {
  switch(type) {
    case BLOCK_TYPE_AMP:
    case BLOCK_TYPE_CAB:
    case BLOCK_TYPE_CHORUS:
    case BLOCK_TYPE_DELAY:
    case BLOCK_TYPE_DRIVE:
    case BLOCK_TYPE_FLANGER:
    case BLOCK_TYPE_PITCH:
    case BLOCK_TYPE_PHASER:
    case BLOCK_TYPE_REVERB:
    case BLOCK_TYPE_WAH:
      return true;
    default:
      break;
  }
  return false;
}

BlockSceneState::BlockSceneState(uint16_t bypass_state)
    : bypass_(bypass_state & 0xFF),
      xy_(bypass_state >> 8) {
}

BlockSceneState::~BlockSceneState() {}

uint16_t BlockSceneState::As16bit() const {
  return bypass_ | static_cast<uint16_t>(xy_ << 8);
}

bool BlockSceneState::IsBypassedInScene(int scene) const {
  return IsBitSet(bypass_, scene);
}

void BlockSceneState::SetBypassedInScene(int scene, bool bypassed) {
  SetBit(&bypass_, scene, bypassed);
}

bool BlockSceneState::IsConfigYEnabledInScene(int scene) const {
  return IsBitSet(xy_, scene);
}

void BlockSceneState::SetConfigYEnabledInScene(int scene, bool y_enabled) {
  SetBit(&xy_, scene, y_enabled);
}

void BlockSceneState::CopyScene(int from, int to) {
  SetConfigYEnabledInScene(to, IsConfigYEnabledInScene(from));
  SetBypassedInScene(to, IsBypassedInScene(from));
}

bool BlockSceneState::ScenesAreEqual(int scene_a, int scene_b) const {
  return IsConfigYEnabledInScene(scene_a) == IsConfigYEnabledInScene(scene_b) &&
         IsBypassedInScene(scene_a) == IsBypassedInScene(scene_b);
}

bool BlockSceneState::IsEqual(const BlockSceneState& other) const {
  return other.bypass_ == bypass_ && other.xy_ == xy_;
}

void BlockSceneState::ToJson(bool supports_xy, Json::Value* out) const {
  Json::Value& j = *out;
  Json::Value enabled;
  // A value of true means the block is active in that scene.
  for (int i = 0; i < 8; ++i)
    enabled.append(!IsBypassedInScene(i));
  j["enabled"] = enabled;

  if (supports_xy) {
    Json::Value xy;
    for (int i = 0; i < 8; ++i)
      xy.append(Json::Value(IsConfigYEnabledInScene(i) ? "y" : "x"));
    j["xy"] = xy;
  }
}

BlockInMatrix::BlockInMatrix() : block_(0), input_mask_(0) {}

BlockInMatrix::BlockInMatrix(uint16_t block, uint16_t mask)
    : block_(block), input_mask_(mask) {
}

bool BlockInMatrix::is_shunt() const {
  // NOTE: The maximum value is just a guess.
  return block_ >= 200 && block_ < (200 + (12 * 4));
}

void BlockInMatrix::ToJson(Json::Value* out) const {
  Json::Value& j = *out;
  j["id"] = block_;
  j["is_shunt"] = is_shunt();
  Json::Value inputs;
  // If the other bits are used, we don't know about it (yet).
  ASSERT((input_mask_ >> 4) == 0);
  for (size_t i = 1; i <= 4; ++i) {
    if ((input_mask_ & (1 << i)) != 0)
      inputs.append(static_cast<Json::Value::UInt>(i));
  }
  j["input_rows"] = inputs;
}

BlockParameters::BlockParameters()
    : block_(BLOCK_INVALID),
      config_(CONFIG_X),
      global_block_index_(0u) {
}

BlockParameters::~BlockParameters() {}

// Populates the block parameters from a 16bit value array.
// Returns the number of 16bit items eaten.
size_t BlockParameters::Initialize(const uint16_t* data, size_t count) {
  if (count < 2U || count < (data[1] + 2U)) {
    ASSERT(false);
    return 0;
  }

  uint16_t block_id = data[0];
  uint8_t state = block_id >> 8;
  if (state) {
    if ((state & 0x80) != 0)
      config_ = CONFIG_Y;
    global_block_index_ = state & 0x0F;
    block_id &= 0x00FF;
  }

  block_ = static_cast<AxeFxIIBlockID>(block_id);
  params_.reserve(data[1]);
  for (uint16_t i = 0; i < data[1]; ++i)
    params_.push_back(data[i + 2]);
  return params_.size() + 2;
}

size_t BlockParameters::Write(uint16_t* dest, size_t buffer_size) const {
  if (buffer_size < (params_.size() + 2)) {
    ASSERT(false);
    return 0u;
  }

  uint16_t id_and_state = static_cast<uint16_t>(block_);
  if (config_ == CONFIG_Y)
    id_and_state |= (0x80 << 8);
  ASSERT((global_block_index_ & 0x0F) == global_block_index_);
  id_and_state |= (global_block_index_ << 8);

  size_t pos = 0u;
  dest[pos++] = id_and_state;
  dest[pos++] = static_cast<uint16_t>(params_.size());
  for (const auto& value: params_)
    dest[pos++] = value;

  return pos;
}

AxeFxBlockType BlockParameters::type() const {
  return GetBlockType(block_);
}

AxeFxIIBlockID BlockParameters::block() const {
  return block_;
}

bool BlockParameters::supports_xy() const {
  return !is_modifier() && BlockSupportsXY(type());
}

size_t BlockParameters::param_count() const {
  return params_.size();
}

bool BlockParameters::is_modifier() const {
  return block_ >= 1 && block_ < kFirstBlockId;
}

uint16_t BlockParameters::GetParamValue(int index, bool get_x_value) const {
  ASSERT(get_x_value || supports_xy());
  ASSERT(!supports_xy() || (static_cast<size_t>(index) < params_.size() / 2));
  ASSERT(supports_xy() || static_cast<size_t>(index) < params_.size());
  return get_x_value ? params_[index] : params_[(params_.size() / 2) + index];
}

void BlockParameters::SetParamValue(int index,
                                    uint16_t value,
                                    bool set_x_value) {
  ASSERT(set_x_value || supports_xy());
  ASSERT(!supports_xy() || (static_cast<size_t>(index) < params_.size() / 2));
  ASSERT(supports_xy() || static_cast<size_t>(index) < params_.size());
  set_x_value ?
      params_[index] = value :
      params_[(params_.size() / 2) + index] = value;
}

BlockSceneState BlockParameters::GetBypassState() const {
  int bypass_id = GetBlockBypassParamID(type());
  return bypass_id == -1 ? BlockSceneState(0) :
                           BlockSceneState(GetParamValue(bypass_id, true));
}

bool BlockParameters::SetBypassState(const BlockSceneState& state) {
  int bypass_id = GetBlockBypassParamID(type());
  if (bypass_id == -1)
    return false;
  SetParamValue(bypass_id, state.As16bit(), true);
  return true;
}

void BlockParameters::ToJson(Json::Value* out) const {
  Json::Value& j = *out;
  AxeFxBlockType block_type = GetBlockType(block_);
  const char* type_name = GetBlockTypeName(block_type);
  j["id"] = block_;
  j["name"] = GetBlockName(block_);
  j["type"] = type_name;
  j["supports_xy"] = supports_xy();
  if (global_block_index_)
    j["global_block_id"] = static_cast<int>(global_block_index_);

  // Used for x/y configs.
  size_t y_offset = params_.size() / 2u;

  if (block_type == BLOCK_TYPE_AMP && !params_.empty()) {
    j["amp_x"] = GetAmpName(params_[DISTORT_TYPE]);
    j["amp_y"] = GetAmpName(params_[y_offset + DISTORT_TYPE]);
  } else if (block_type == BLOCK_TYPE_CAB) {
    j["cab_x_left"] = GetCabName(params_[CABINET_TYPEL]);
    j["cab_x_right"] = GetCabName(params_[CABINET_TYPER]);
    j["cab_y_left"] = GetCabName(params_[y_offset + CABINET_TYPEL]);
    j["cab_y_right"] = GetCabName(params_[y_offset + CABINET_TYPER]);
  }

  std::string default_param_prefix(type_name);
  std::transform(default_param_prefix.begin(), default_param_prefix.end(),
                 default_param_prefix.begin(), &tolower);
  default_param_prefix += '_';

  Json::Value scenes;
  GetBypassState().ToJson(supports_xy(), &scenes);
  j["scenes"] = scenes;

  Json::Value values;
  if (supports_xy()) {
    Json::Value values_x, values_y;
    Json::Value* x_and_y[] = { &values_x, &values_y };
    int v = 0;
    for (size_t i = 0; i < params_.size(); ++i) {
      const char* param_name =
          GetParamName(block_type, static_cast<int>(i % y_offset));
      if (i == y_offset)
        ++v;

      Json::Value& dict = *x_and_y[v];
      if (!param_name[0]) {
        dict[default_param_prefix + std::to_string(i)] = params_[i];
      } else {
        dict[param_name] = params_[i];
      }
    }
    values["x"] = values_x;
    values["y"] = values_y;
  } else {
    for (size_t i = 0; i < params_.size(); ++i) {
      const char* param_name = GetParamName(block_type, static_cast<int>(i));
      if (!param_name[0]) {
        values[default_param_prefix + std::to_string(i)] = params_[i];
      } else {
        values[param_name] = params_[i];
      }
    }
  }
  j["values"] = values;
}

}  // namespace axefx
