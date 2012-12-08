// Copyright (c) 2012, Tomas Gunnarsson
// All rights reserved.

#include "axefx/blocks.h"

namespace axefx {

const int kFirstBlockId = 100;

inline bool IsBitSet(uint8_t byte, int index) {
  ASSERT(index >= 0 && index < 8);
  return (byte >> index) & 1;
}

#ifdef _DEBUG

// If we really need this, it would be better to keep it in a separate file
// (e.g. json) that can be updated separately.
// AxeEdit uses .profile files (a copy of one is in this directory)
// to track valid values for parameters per AxeFx firmware version.
// This data comes from and is only for basic testing.
// http://wiki.fractalaudio.com/axefx2/index.php?title=AMP_(block):_list
// TODO: Come up with a better list :) Some entries missing etc (see below).
const char* const kAmpNames[] = {
  "59 Bassguy ('59 Fender Bassman)",
  "65 Bassguy ('65 Fender Bassman)",
  "Vibrato Verb (Fender Vibroverb)",
  "Deluxe Verb (Fender Deluxe Reverb)",
  "Double Verb (Fender Twin Reverb)",
  "JR Blues (Fender Blues Junior)",
  "Class-A 15W TB (Vox AC-15 Top Boost)",
  "Class-A 30W (Vox AC-30)",
  "Class-A 30W TB (Vox AC-30 Top Boost)",
  "Brit JM45 (Marshall JTM45)",
  "Plexi Normal / Treble (Marshall Super Lead 1959)",
  "1987X Normal / Treble (Marshall 1987x Vintage Series)",
  "Brit 800 (Marshall JCM800 2204)",
  "Brit Super (Marshall AFD100)",
  "HiPower Normal / Brilliant (Hiwatt DR103)",
  "USA Clean 1 (Mesa Boogie Mark IV)",
  "USA Clean 2 (Mesa Boogie Triaxis)",
  "USA Rhy 1 (Mesa Boogie Mark IV)",
  "USA IIC+ Norm / Bright (Mesa Boogie Mark IIC+)",
  "USA Lead 1 / Lead 2 (Mesa Boogie Mark IV)",
  "Recto Orange / Red, Vintage / Modern (Mesa Boogie Dual Rectifier)",
  "Euro Blue / Red (Bogner Ecstasy 20th Anniversary)",
  "Shiver Clean / Lead (Bogner Shiva 20th Anniversary)",
  "Euro Uber (Bogner Uberschall)",
  "Solo 99 Clean (Soldano X99)",
  "Solo 100 Rhy / Lead (Soldano SLO-100)",
  "Friedman BE / HBE (Friedman Brown Eye and Hairy Brown Eye)",
  "PVH 6160 (Peavey 5150 II)",
  "MR Z 38 Sr (Dr. Z Maz 38 SR)",
  "CA3+ Rhy / Lead (CAE 3+ SE)",
  "Wrecker 1 (Trainwreck Express)",
  "Corncob M50 (Cornford MK50II)",
  "Carol-Ann Od-2 (Carol-Ann OD-2r)",
  "Fryette D60 L / M (Fryette Deliverance 60)",
  "Brit Brown (Van Halen's Marshall)",
  "Citrus RV50 (Orange Rockerverb)",
  "Jazz 120 (Roland JC-120)",
  "Energyball (Engl Powerball)",
  "ODS-100 Clean / Lead (Dumble OD Special)",  // two amps?
  "FAS Rhythm",
  "FAS Lead 1",
  "FAS Lead 2",
  "FAS Modern",
  "Das Metall (Diezel VH4)",
  "Brit Pre (Marshall JMP-1)",
  "Buttery (Budda Twinmaster)",
  "Boutique 1 / 2 (Matchless Chieftain)",  // two amps?
  "Cameron Ch.1 / Ch.2 (Cameron CCV)",
  "SV Bass (Ampeg SVT)",
  "Tube Pre (generic tube preamp)",
  "FAS Brown (Van Halen's Marshall)",
  "FAS Big Hair",
  "Solo X99 Lead (Soldano X99)",
  "Supertweed",
  "FAS Wreck (Trainwreck Express)",
  "TX Star Lead (Mesa Boogie Lone Star)",
  "Brit JVM OD1 / OD2 (Marshall JVM410)",
  "FAS 6160 (Peavey 5150)",
  "Cali Leggy (Carvin Legacy)",
  "USA Lead 1+ / Lead 2+ (Mesa Boogie Mark IV)",
  "Prince Tone (Fender Princeton)",
  "Blanknshp Leeds (Blankenship Leeds)",
  "5153 Green / Blue / Red (EVH 5150 III)",
  "Solo 88 Rhythm (Soldano X88)",
  "Division13 CJ (Divided by 13 CJ11)",
  "Herbie CH2- / CH2+ / CH3 (Diezel Herbert)",  // multiple amps?
  "Dizzy V4 2 / 3 / 4 (Diezel VH4)",  // Is this several different amps?
  "Dirty Shirley (Friedman Dirty Shirley)",
  "Suhr Badger 18 / 30",
  "Prince Tone 2 (Fender Princeton)",
  "Super Trem (Supro 1964T)",
  "Atomica Low / High (Cameron Atomica)",
  "Deluxe Tweed (Fender Tweed Deluxe)",
  "Spawn Q-Rod 1st / 2nd / 3rd (Splawn Quickrod)",
  "Brit Silver (Marshall Silver Jubilee)",
  "Spawn Nitrous (Splawn Nitro)",
  "FAS Crunch",
  "Two Stone J-35 (Two-Rock Jet 35)",
  "Fox ODS (Fuchs Overdrive Supreme)",
  "Hot Kitty (Bad Cat Hot Cat)",
  "Band-Commander (1968 Fender Bandmaster)",
  "Super Verb (1964 Fender Super Reverb)",
};

const char* GetAmpName(uint16_t amp_id) {
  if (amp_id >= arraysize(kAmpNames))
    return "AmpUnknown";
  return kAmpNames[amp_id];
}
#endif

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
  }
  return false;
}

BlockSceneState::BlockSceneState(uint16_t bypass_state)
    : bypass_(bypass_state & 0xFF),
      xy_(bypass_state >> 8) {
}

BlockSceneState::~BlockSceneState() {}

bool BlockSceneState::IsBypassedInScene(int scene) const {
  return IsBitSet(bypass_, scene);
}

bool BlockSceneState::IsConfigYEnabledInScene(int scene) const {
  return IsBitSet(xy_, scene);
}

BlockInMatrix::BlockInMatrix() : block_(0), input_mask_(0) {}

bool BlockInMatrix::is_shunt() const {
  // NOTE: The maximum value is just a guess.
  return block_ >= 200 && block_ < (200 + (12 * 4));
}

BlockParameters::BlockParameters()
    : block_(BLOCK_INVALID),
      config_(CONFIG_X),
      global_block_index_(0) {
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

BlockSceneState BlockParameters::GetBypassState() const {
  int bypass_id = GetBlockBypassParamID(type());
  return bypass_id == -1 ? BlockSceneState(0) :
                           BlockSceneState(GetParamValue(bypass_id, true));
}

}  // namespace axefx
