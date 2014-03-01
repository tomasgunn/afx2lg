// Copyright (c) 2013, Tomas Gunnarsson
// All rights reserved.

#pragma once
#ifndef AXYS_TREE_ROOT_ITEM_H_
#define AXYS_TREE_ROOT_ITEM_H_

#include "common/common_types.h"
#include "juce/JuceHeader.h"

class PresetItem;

class TreeRootItem
    : public juce::TreeViewItem,
      public juce::KeyListener {
 public:
  TreeRootItem(juce::FileDragAndDropTarget* delegate,
               juce::UndoManager* undo_manager,
               bool allow_drag_drop_of_presets,
               bool allow_edit_buffer_presets,
               bool update_id_on_move);
  virtual ~TreeRootItem();

  // Sorts presets using the default compare function (IDs).
  void sortPresets();

  // Returns the number of presets added, or -1 in the case of an error.
  int addPresetsFromFile(const juce::File& file, juce::String* err);

  PresetItem* getPreset(int index) const;

  void deleteSelection();
  void moveSelectionUp();
  void moveSelectionDown();

 private:
  virtual bool mightContainSubItems() { return true; }

  virtual bool isInterestedInDragSource(
      const juce::DragAndDropTarget::SourceDetails& source_details);
  virtual void itemDropped(
      const juce::DragAndDropTarget::SourceDetails& source_details,
      int insert_index);

  virtual bool isInterestedInFileDrag(const juce::StringArray& files) {
    return delegate_->isInterestedInFileDrag(files);
  }

  virtual void filesDropped(const juce::StringArray& files, int insert_index) {
    delegate_->filesDropped(files, 0, 0);
  }

  virtual bool keyPressed(const juce::KeyPress& key,
      juce::Component* originatingComponent);

  juce::UndoManager* undo_manager_;
  juce::FileDragAndDropTarget* delegate_;
  bool allow_drag_drop_of_presets_;
  bool allow_edit_buffer_presets_;
  bool update_id_on_move_;
};

#endif  // AXYS_TREE_ROOT_ITEM_H_
