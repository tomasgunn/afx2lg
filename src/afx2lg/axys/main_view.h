// Copyright (c) 2013, Tomas Gunnarsson
// All rights reserved.

#pragma once
#ifndef AXYS_MAIN_VIEW_H_
#define AXYS_MAIN_VIEW_H_

#include "axefx/axe_fx_sysex_parser.h"
#include "axys/MainWnd.h"
#include "common/common_types.h"

class TreeRootItem : public juce::TreeViewItem {
 public:
  TreeRootItem(juce::FileDragAndDropTarget* delegate) : delegate_(delegate) {}
  virtual ~TreeRootItem() {}

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

  juce::FileDragAndDropTarget* delegate_;
};

class PresetItem : public juce::TreeViewItem {
 public:
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

class MainView
    : public MainViewBase,
      public juce::DragAndDropContainer,
      public juce::FileDragAndDropTarget {
 public:
  MainView();
  virtual ~MainView();

 private:
  virtual void mouseEnter(const juce::MouseEvent& event);

  virtual bool isInterestedInFileDrag(const juce::StringArray& files);
  virtual void fileDragEnter(const juce::StringArray& files, int x, int y);
  virtual void fileDragMove(const juce::StringArray& files, int x, int y);
  virtual void filesDropped(const juce::StringArray& files, int x, int y);

  virtual void buttonClicked(Button* btn);

  void OnOpenSysEx();
  void OnClose();
  bool OpenFile(const juce::String& path);
  bool OpenFile(const juce::File& file);

  void ShowError(const juce::String& text);

  TreeRootItem root_;
};

#endif  // AXYS_MAIN_VIEW_H_
