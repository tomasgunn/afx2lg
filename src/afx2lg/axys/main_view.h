// Copyright (c) 2013, Tomas Gunnarsson
// All rights reserved.

#pragma once
#ifndef AXYS_MAIN_VIEW_H_
#define AXYS_MAIN_VIEW_H_

#include "axys/MainWnd.h"
#include "common/common_types.h"

class MainView
    : public MainViewBase,
      public juce::FileDragAndDropTarget {
 public:
  MainView();
  virtual ~MainView();

 private:
  class TreeRootItem : public juce::TreeViewItem {
   public:
    TreeRootItem(juce::FileDragAndDropTarget* delegate) : delegate_(delegate) {}
    virtual ~TreeRootItem() {}

   private:
    virtual bool mightContainSubItems() {
      return true;
    }

    virtual bool isInterestedInFileDrag(const juce::StringArray& files) {
      return delegate_->isInterestedInFileDrag(files);
    }

    virtual void filesDropped (const juce::StringArray& files, int insert_index) {
      delegate_->filesDropped(files, 0, 0);
    }

   private:
    juce::FileDragAndDropTarget* delegate_;
  };

  virtual void mouseEnter(const juce::MouseEvent& event);

  virtual bool isInterestedInFileDrag(const juce::StringArray& files);
  virtual void fileDragEnter(const juce::StringArray& files, int x, int y);
  virtual void fileDragMove(const juce::StringArray& files, int x, int y);
  virtual void filesDropped (const juce::StringArray& files, int x, int y);

  virtual void buttonClicked(Button* btn);

  void OnOpenSysEx();
  void OnClose();
  void OpenFile(const juce::String& path);

  TreeRootItem root_;
};

#endif  // AXYS_MAIN_VIEW_H_
