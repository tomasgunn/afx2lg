// Copyright (c) 2013, Tomas Gunnarsson
// All rights reserved.

#include "axys/main_view.h"

MainView::MainView() {}
MainView::~MainView() {}

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
      "Open SysEx file", juce::File::nonexistent, "*.syx", true);
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
