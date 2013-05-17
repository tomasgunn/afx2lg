// Copyright (c) 2013, Tomas Gunnarsson
// All rights reserved.

#include "axys/main_view.h"

#include <functional>

#include "axefx/axe_fx_sysex_parser.h"
#include "axefx/preset.h"
#include "axys/tree_preset_item.h"

using std::placeholders::_1;

using axefx::SysExParser;
using namespace juce;

MainView::MainView() : root_(this, &undo_manager_, true, true) {
  tree_view_->setRootItem(&root_);
  tree_view_->setRootItemVisible(false);
  tree_view_->setMultiSelectEnabled(true);
  tree_view_->setWantsKeyboardFocus(true);
  tree_view_->addKeyListener(&root_);

  Rectangle<int> rc = grid_group_->getBoundsInParent();

  static const int kBlockWidth = 52;
  static const int kBlockHeight = 38;

  rc.removeFromTop(20);
  rc.removeFromBottom(5);
  rc.reduce(10, 0);

  const int kHorizSpacing =
      (rc.getWidth() - (axefx::kMatrixColumns * kBlockWidth)) /
      (axefx::kMatrixColumns - 1);
  const int kVerticalSpacing =
      (rc.getHeight() - (axefx::kMatrixRows * kBlockHeight)) /
      (axefx::kMatrixRows - 1);

  int block_x = rc.getTopLeft().getX();
  int block_y = rc.getTopLeft().getY();
  for (auto& r : grid) {
    for (auto& b : r.blocks) {
      addAndMakeVisible(&b, -1);
      b.setLookAndFeel(&old_school_);
      b.setButtonText("block");
      b.setConnectedEdges(
          Button::ConnectedOnLeft | Button::ConnectedOnRight |
          Button::ConnectedOnTop  | Button::ConnectedOnBottom);
      b.setBounds(block_x, block_y, kBlockWidth, kBlockHeight);
      block_x += kBlockWidth + kHorizSpacing;
    }
    block_y += kBlockHeight + kVerticalSpacing;
    block_x = rc.getTopLeft().getX();
  }

  grid_group_->toBack();
}

MainView::~MainView() {}

bool MainView::isInterestedInFileDrag(const StringArray& files) {
  return true;  // Accept files dropped regardless of extension.
}

void MainView::filesDropped(const StringArray& files, int x, int y) {
  String err;
  int err_count = 0;

  for (const auto& f : files) {
    String e;
    OpenFile(f, &e);
    if (e.length() && err_count < 10) {
      ++err_count;
      if (err_count == 10) {
        err += "(etc)";
      } else {
        err += e;
        err += "\n";
      }
    }
  }

  if (err.length())
    ShowError(err);
}

void MainView::buttonClicked(Button* btn) {
  struct {
    Button* btn;
    void (MainView::*handler)();
  } handlers[] = {
    { open_btn_, &MainView::OnOpenSysEx },
    { export_all_btn_, &MainView::OnExportAll },
    { export_sel_btn_, &MainView::OnExportSel },
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
  if (dlg.browseForFileToOpen()) {
    String err;
    if (!OpenFile(dlg.getResult(), &err))
      ShowError(err);
  }
}

void MainView::OnExportAll() {
  FileChooser dlg("Save SysEx file", File::nonexistent, "*.syx", true);
  if (dlg.browseForFileToSave(true))
    ExportToFile(dlg.getResult(), false);
}

void MainView::OnExportSel() {
  bool has_selected_items = false;
  int count = root_.getNumSubItems();
  for (int i = 0; i < count && !has_selected_items; ++i)
    has_selected_items = root_.getPreset(i)->isSelected();
  if (!has_selected_items) {
    ShowError("No presets are selected.");
    return;
  }

  FileChooser dlg("Save SysEx file", File::nonexistent, "*.syx", true);
  if (dlg.browseForFileToSave(true))
    ExportToFile(dlg.getResult(), true);
}

bool MainView::OpenFile(const String& path, String* err) {
  DBG(path);
  return OpenFile(File(path), err);
}

bool MainView::OpenFile(const File& file, juce::String* err) {
  int ret = root_.addPresetsFromFile(file, err);
  return ret != -1;
}

bool MainView::ExportToFile(const juce::File& file, bool only_selection) {
  struct Callback {
    Callback() {}
    ~Callback() {}

    void OnData(const std::vector<uint8_t>& data) {
      output->write(&data[0], data.size());
    }

    unique_ptr<FileOutputStream> output;
  };

  // Delete any previously existing file.
  file.deleteFile();

  Callback callback;
  callback.output.reset(file.createOutputStream());
  if (!callback.output) {
    ShowError("Failed to open file for writing: " + file.getFullPathName());
    return false;
  }

  axefx::SysExCallback cb = std::bind(&Callback::OnData, &callback, _1);
  int count = root_.getNumSubItems();
  for (int i = 0; i < count; ++i) {
    auto* p = root_.getPreset(i);
    if (!only_selection || p->isSelected())
      p->preset()->Serialize(cb);
  }

  return true;
}

void MainView::ShowError(const String& text) {
  NativeMessageBox::showMessageBoxAsync(AlertWindow::WarningIcon,
      "Sorry", text, this, nullptr);
}
