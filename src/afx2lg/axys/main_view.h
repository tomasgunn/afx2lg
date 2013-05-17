// Copyright (c) 2013, Tomas Gunnarsson
// All rights reserved.

#pragma once
#ifndef AXYS_MAIN_VIEW_H_
#define AXYS_MAIN_VIEW_H_

#include "axefx/blocks.h"
#include "axys/MainWnd.h"
#include "axys/tree_root_item.h"
#include "common/common_types.h"

class MainView
    : public axys::MainViewBase,
      public juce::DragAndDropContainer,
      public juce::FileDragAndDropTarget {
 public:
  MainView();
  virtual ~MainView();

 private:
  virtual bool isInterestedInFileDrag(const juce::StringArray& files);
  virtual void filesDropped(const juce::StringArray& files, int x, int y);

  virtual void buttonClicked(Button* btn);

  void OnOpenSysEx();
  void OnExportAll();
  void OnExportSel();
  bool OpenFile(const juce::String& path, juce::String* err);
  bool OpenFile(const juce::File& file, juce::String* err);
  bool ExportToFile(const juce::File& file, bool only_selection);

  void ShowError(const juce::String& text);

  juce::UndoManager undo_manager_;
  TreeRootItem root_;

  juce::OldSchoolLookAndFeel old_school_;

  struct GridRow {
    TextButton blocks[axefx::kMatrixColumns];
  };
  GridRow grid[axefx::kMatrixRows];
};

#endif  // AXYS_MAIN_VIEW_H_
