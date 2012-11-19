// Copyright (c) 2012, Tomas Gunnarsson
// All rights reserved.

#include "axefx/blocks.h"

namespace axefx {

inline bool IsBitSet(uint8_t byte, int index) {
  ASSERT(index >= 0 && index < 8);
  return (byte >> index) & 1;
}

// If we really need this, it would be better to keep it in a separate file
// (e.g. json) that can be updated separately.
// Data comes from:
// http://wiki.fractalaudio.com/axefx2/index.php?title=AMP_(block):_list
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
  "ODS-100 Clean / Lead (Dumble OD Special)",
  "FAS Rhythm",
  "FAS Lead 1",
  "FAS Lead 2",
  "FAS Modern",
  "Das Metall (Diezel VH4)",
  "Brit Pre (Marshall JMP-1)",
  "Buttery (Budda Twinmaster)",
  "Boutique 1 / 2 (Matchless Chieftain)",
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
  "Herbie CH2- / CH2+ / CH3 (Diezel Herbert)",
  "Dizzy V4 2 / 3 / 4 (Diezel VH4)",
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
  "Band-Commander (1968 Fender Bandmaster)",
  "Super Verb (1964 Fender Super Reverb)",
};

struct BlockMap {
  int block_id;
  const char* name;
};

// Keep sorted.
const BlockMap kBlocks[] = {
  {100, "Compressor 1"},
  {101, "Compressor 2"},
  {102, "GraphicEQ 1"},
  {103, "GraphicEQ 2"},
  {104, "Parametric EQ 1"},
  {105, "Parametric EQ 2"},
  {106, "Amp 1"},
  {107, "Amp 2"},
  {108, "Cabinet 1"},
  {109, "Cabinet 2"},
  {110, "Reverb 1"},
  {111, "Reverb 2"},
  {112, "Delay 1"},
  {113, "Delay 2"},
  {114, "Multidelay 1"},
  {115, "Multidelay 2"},
  {116, "Chorus 1"},
  {117, "Chorus 2"},
  {118, "Flanger 1"},
  {119, "Flanger 2"},
  {120, "Rotary 1"},
  {121, "Rotary 2"},
  {122, "Phaser 1"},
  {123, "Phaser 2"},
  {124, "Wahwah 1"},
  {125, "Wahwah 2"},
  {126, "Formant"},
  {127, "Vol/Pan 1"},
  {128, "Panner/Tremolo 1"},
  {129, "Panner/Tremolo 2"},
  {130, "Pitch 1"},
  {131, "Filter 1"},
  {132, "Filter 2"},
  {133, "Drive 1"},
  {134, "Drive 2"},
  {135, "Enhancer"},
  {136, "Effects Loop"},
  {137, "Mixer 1"},
  {138, "Mixer 2"},
  {139, "NoiseGate"},
  {140, "Output"},
  {141, "Controllers"},
  {142, "Feedback Send"},
  {143, "Feedback Return"},
  {144, "Synth 1"},
  {145, "Synth 2"},
  {146, "Vocoder"},
  {147, "Megatap Delay"},
  {148, "Crossover 1"},
  {149, "Crossover 2"},
  {150, "Gate/Expander 1"},
  {151, "Gate/Expander 2"},
  {152, "RingMod"},
  {153, "Pitch 2"},
  {154, "Multiband Comp 1"},
  {155, "Multiband Comp 2"},
  {156, "Quad Chorus 1"},
  {157, "Quad Chorus 2"},
  {158, "Resonator 1"},
  {159, "Resonator 2"},
  {160, "GraphicEQ 3"},
  {161, "GraphicEQ 4"},
  {162, "Parametric EQ 3"},
  {163, "Parametric EQ 4"},
  {164, "Filter 3"},
  {165, "Filter 4"},
  {166, "Vol/Pan 2"},
  {167, "Vol/Pan 3"},
  {168, "Vol/Pan 4"},
  {169, "Looper"},
  {170, "Tone Match"},
};

const int kFirstBlockId = kBlocks[0].block_id;
const int kLastBlockId = kBlocks[arraysize(kBlocks) - 1].block_id;

const char* GetAmpName(uint16_t amp_id) {
  if (amp_id >= arraysize(kAmpNames))
    return "AmpUnknown";
  return kAmpNames[amp_id];
}

const char* GetBlockName(uint16_t block_id) {
  for (int i = 0; i < arraysize(kBlocks); ++i) {
    if (block_id == kBlocks[i].block_id)
      return kBlocks[i].name;
  }
  return "BlockUnknown";
}

bool IsShunt(uint16_t block_id) {
  return block_id >= 200 && block_id < (200 + (12*4));
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

}  // namespace axefx
