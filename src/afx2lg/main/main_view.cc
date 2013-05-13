// Copyright (c) 2013, Tomas Gunnarsson
// All rights reserved.

#include "main/main_view.h"

MainView::MainView() : root_(this, false) {
  tree_view_->setRootItem(&root_);
  tree_view_->setRootItemVisible(false);
  tree_view_->setMultiSelectEnabled(true);
}

MainView::~MainView() {}

void MainView::mouseEnter(const MouseEvent& event) {
  DBG(__FUNCTION__);
}

bool MainView::isInterestedInFileDrag(const StringArray& files) {
  return true;  // Accept files dropped regardless of extension.
}

void MainView::fileDragEnter(const StringArray& files, int x, int y) {
  DBG(__FUNCTION__);
}

void MainView::fileDragMove(const StringArray& files, int x, int y) {
  DBG(__FUNCTION__);
}

void MainView::filesDropped(const StringArray& files, int x, int y) {
  // TODO: Support a way to batch up error messages so that we don't
  // display a series of dialog boxes when things go bad.
  for (const auto& f : files) {
    File file(f);
    if (file.getFileExtension().equalsIgnoreCase(".txt")) {
      OpenTemplate(file);
    } else {
      // Assume a sysex file.
      OpenSysExFile(file);
    }
  }
}

void MainView::buttonClicked(Button* btn) {
  struct {
    Button* btn;
    void (MainView::*handler)();
  } handlers[] = {
    { open_btn_, &MainView::OnOpenTemplate },
    { generate_btn_, &MainView::OnGenerateSetup },
  };

  for (auto& h : handlers) {
    if (h.btn == btn) {
      (this->*h.handler)();
      break;
    }
  }
}

void MainView::OnOpenTemplate() {
  FileChooser dlg("Open LG setup template", File::nonexistent, "*.txt", true);
  if (dlg.browseForFileToOpen())
    OpenTemplate(dlg.getResult());
}

void MainView::OnGenerateSetup() {
  DBG(__FUNCTION__);
  if (!template_file_.existsAsFile()) {
    ShowError("Please select a template file first.");
    return;
  }
}

bool MainView::OpenTemplate(const File& f) {
  DBG(__FUNCTION__);
  return false;
}

bool MainView::OpenSysExFile(const File& f) {
  String err;
  int ret = root_.AddPresetsFromFile(f, &err);
  if (ret == -1) {
    ShowError(err);
    return false;
  }

  return ret != 0;
}

void MainView::ShowError(const String& text) {
  NativeMessageBox::showMessageBoxAsync(AlertWindow::WarningIcon,
      "Sorry", text, this, nullptr);
}
