// Copyright (c) 2013, Tomas Gunnarsson
// All rights reserved.

#include "axys/main_view.h"

#include "axefx/axe_fx_sysex_parser.h"
#include "axefx/preset.h"
#include "axys/tree_preset_item.h"

using axefx::SysExParser;
using namespace juce;

MainView::MainView() : root_(this, &undo_manager_, true, true) {
  tree_view()->setRootItem(&root_);
  tree_view()->setRootItemVisible(false);
  tree_view()->setMultiSelectEnabled(true);
  tree_view()->setWantsKeyboardFocus(true);
  tree_view()->addKeyListener(&root_);
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
  for (const auto& f : files) {
    if (!OpenFile(f)) {
      // TODO: Support a way to batch up error messages so that we don't
      // display a series of dialog boxes when things go bad.
    }
  }
}

void MainView::buttonClicked(Button* btn) {
  struct {
    Button* btn;
    void (MainView::*handler)();
  } handlers[] = {
    { close_btn(), &MainView::OnClose },
    { open_btn(), &MainView::OnOpenSysEx },
    { export_all_btn(), &MainView::OnExportAll },
    { export_sel_btn(), &MainView::OnExportSel },
  };

  for (auto& h : handlers) {
    if (h.btn == btn) {
      (this->*h.handler)();
      break;
    }
  }
}

void MainView::OnOpenSysEx() {
  FileChooser dlg("Open SysEx file", File::nonexistent, "*.syx", true);
  if (dlg.browseForFileToOpen())
    OpenFile(dlg.getResult());
}

void MainView::OnExportAll() {
  FileChooser dlg("Save SysEx file", File::nonexistent, "*.syx", true);
  // dlg.browseForDirectory();
  // dlg.browseForFileToSave();
  if (dlg.browseForFileToSave(true))
    ExportAllToFile(dlg.getResult());
}

void MainView::OnExportSel() {
  FileChooser dlg("Save SysEx file", File::nonexistent, "*.syx", true);
  // dlg.browseForDirectory();
  // dlg.browseForFileToSave();
  if (dlg.browseForFileToSave(true))
    ExportSelectionToFile(dlg.getResult());
}

void MainView::OnClose() {
  JUCEApplication::quit();
}

bool MainView::OpenFile(const String& path) {
  DBG(path);
  return OpenFile(File(path));
}

bool MainView::OpenFile(const File& file) {
  String err;
  int ret = root_.addPresetsFromFile(file, &err);
  if (ret == -1) {
    ShowError(err);
    return false;
  }

  return ret != 0;
}

bool MainView::ExportAllToFile(const juce::File& file) {
  // TODO.
  return false;
}

bool MainView::ExportSelectionToFile(const juce::File& file) {
  // TODO.
  return false;
}

void MainView::ShowError(const String& text) {
  NativeMessageBox::showMessageBoxAsync(AlertWindow::WarningIcon,
      "Sorry", text, this, nullptr);
}
