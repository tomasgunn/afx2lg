// Copyright (c) 2013, Tomas Gunnarsson
// All rights reserved.

#pragma once
#ifndef AXYS_TREE_PRESET_ITEM_H_
#define AXYS_TREE_PRESET_ITEM_H_

#include "common/common_types.h"
#include "juce/JuceHeader.h"

namespace axefx {
class Preset;
}  // namespace axefx

class PresetItem : public juce::TreeViewItem {
 public:
  static const int kMagicPresetNumber = 0xF321;

  PresetItem(const shared_ptr<axefx::Preset>& preset);

  virtual ~PresetItem();

  int id() const;
  void set_id(int id);

 private:
  virtual bool mightContainSubItems();
  virtual juce::var getDragSourceDescription();
  virtual bool isInterestedInDragSource(
      const juce::DragAndDropTarget::SourceDetails& source_details);
  virtual void itemDropped(
      const juce::DragAndDropTarget::SourceDetails& source_details,
      int insert_index);
  virtual void paintItem(juce::Graphics& g, int width, int height);

  shared_ptr<axefx::Preset> preset_;
};

#endif  // AXYS_TREE_PRESET_ITEM_H_
