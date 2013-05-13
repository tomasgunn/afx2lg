// Copyright (c) 2013, Tomas Gunnarsson
// All rights reserved.

#include "axys/tree_preset_item.h"

#include "axefx/preset.h"

using namespace juce;

PresetItem::PresetItem(const shared_ptr<axefx::Preset>& preset)
    : preset_(preset) {
}

PresetItem::~PresetItem() {
  DBG(__FUNCTION__);
}

int PresetItem::id() const {
  if (preset_->from_edit_buffer())
    return -1;
  return preset_->id();
}

void PresetItem::set_id(int id) {
  if (id < 0)
    preset_->SetAsEditBuffer();
  else
    preset_->set_id(id);
}

bool PresetItem::mightContainSubItems() {
  return false;  // For now at least.
}

bool PresetItem::isInterestedInDragSource(
    const DragAndDropTarget::SourceDetails& source_details) {
  DBG(__FUNCTION__);
  return false;
}

void PresetItem::itemDropped(
    const juce::DragAndDropTarget::SourceDetails& source_details,
    int insert_index) {
  DBG(__FUNCTION__ << " " << insert_index);
}

var PresetItem::getDragSourceDescription() {
  return var(kMagicPresetNumber);
}

void PresetItem::paintItem(Graphics& g, int width, int height) {
  // if this item is selected, fill it with a background colour..
  if (isSelected())
    g.fillAll(Colours::blue.withAlpha(0.3f));

  // g.setColour(Colour(0, 0, 0));
  // g.setFont(height * 0.7f);

  // TODO: Somehow support using either 0 based or 1 based IDs.
  String label;
  if (preset_->from_edit_buffer()) {
    label = "? ";
  } else {
    label = String(preset_->id());
    label += " ";
  }
  label += preset_->name().c_str();
  g.drawText(label, 0, 0, width, height, Justification::centredLeft, true);
}
