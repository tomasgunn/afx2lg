// Copyright (c) 2013, Tomas Gunnarsson
// All rights reserved.

#pragma once
#ifndef AFX2LG_MAIN_VIEW_H_
#define AFX2LG_MAIN_VIEW_H_

#include "main/MainWnd.h"
#include "axys/tree_root_item.h"

using namespace juce;

class MainView
    : public MainViewBase,
      public DragAndDropContainer,
      public FileDragAndDropTarget,
      public KeyListener {
 public:
  MainView();
  virtual ~MainView();

 private:
  virtual bool isInterestedInFileDrag(const StringArray& files);
  virtual void filesDropped(const StringArray& files, int x, int y);

  virtual void buttonClicked(Button* btn);

  virtual bool keyPressed(const KeyPress& key,
                          Component* originatingComponent);

  void OnOpenTemplate();
  void OnGenerateSetup();

  bool OpenTemplate(const File& f);
  bool OpenSysExFile(const File& f);

  void ShowError(const String& text);

  TreeRootItem root_;
  File template_file_;
};

#endif  // AFX2LG_MAIN_VIEW_H_
