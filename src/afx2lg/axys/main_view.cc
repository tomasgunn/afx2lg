// Copyright (c) 2013, Tomas Gunnarsson
// All rights reserved.

#include "axys/main_view.h"

using juce::StringArray;

MainView::MainView() : root_(this) {
  tree_view()->setRootItem(&root_);
  tree_view()->setRootItemVisible(false);
}

MainView::~MainView() {}

void MainView::mouseEnter(const juce::MouseEvent& event) {
  DBG(__FUNCTION__);
}

bool MainView::isInterestedInFileDrag(const StringArray& files) {
  DBG(__FUNCTION__);
  return true;  // Accept files dropped regardless of extension.
}

void MainView::fileDragEnter(const juce::StringArray& files, int x, int y) {
  DBG(__FUNCTION__);
}

void MainView::fileDragMove(const juce::StringArray& files, int x, int y) {
  DBG(__FUNCTION__);
}

void MainView::filesDropped(const StringArray& files, int x, int y) {
  for (const auto& f : files) {
    OpenFile(f);
  }
}

void MainView::buttonClicked(Button* btn) {
  struct {
    Button* btn;
    void (MainView::*handler)();
  } handlers[] = {
    { close_btn(), &MainView::OnClose },
    { open_btn(), &MainView::OnOpenSysEx },
  };

  for (auto& h : handlers) {
    if (h.btn == btn) {
      (this->*h.handler)();
      break;
    }
  }
}

void MainView::OnOpenSysEx() {
  juce::FileChooser dlg(
      "Open SysEx file",
      juce::File::nonexistent,
      "*.syx", true);

  if (dlg.browseForFileToOpen()) {
    juce::File f(dlg.getResult());
    if (f.exists()) {
      // tbd.
    }
  }
}

void MainView::OnClose() {
  juce::JUCEApplication::quit();
}

void MainView::OpenFile(const juce::String& path) {
  DBG(path);
}
