// Copyright (c) 2013, Tomas Gunnarsson
// All rights reserved.

#include "main/main_view.h"

#include "axefx/axe_fx_sysex_parser.h"
#include "axys/tree_preset_item.h"
#include "lg/lg_parser.h"

namespace {
class SetupFileWriter : public lg::LgParserCallback {
 public:
  explicit SetupFileWriter() {}
  ~SetupFileWriter() {}

  bool Initialize(const File& f) {
    file_.reset(f.createOutputStream());
    return file_.get() != NULL;
  }

  bool AddPreset(const shared_ptr<axefx::Preset>& p) {
    if (p->from_edit_buffer())
      return false;

    if (presets_.find(p->id()) != presets_.end())
      return false;

    presets_.insert(std::make_pair(p->id(), p));

    return true;
  }

 private:
  virtual void WriteLine(const char* line, size_t length) {
    file_->write(line, length);
  }

  virtual const axefx::PresetMap& GetPresetMap() {
    return presets_;
  }

  axefx::PresetMap presets_;
  unique_ptr<juce::FileOutputStream> file_;
  DISALLOW_COPY_AND_ASSIGN(SetupFileWriter);
};
}  // namespace

MainView::MainView() : root_(this, &undo_manager_, false, false) {
  tree_view_->setRootItem(&root_);
  tree_view_->setRootItemVisible(false);
  tree_view_->setMultiSelectEnabled(true);
  tree_view_->setWantsKeyboardFocus(true);
  tree_view_->addKeyListener(&root_);
}

MainView::~MainView() {}

bool MainView::isInterestedInFileDrag(const StringArray& files) {
  return true;  // Accept files dropped regardless of extension.
}

void MainView::filesDropped(const StringArray& files, int x, int y) {
  String err;
  int err_count = 0;
  for (const auto& f : files) {
    File file(f);
    String e;
    if (file.getFileExtension().equalsIgnoreCase(".txt")) {
      OpenTemplate(file);
    } else {
      // Assume a sysex file.
      root_.addPresetsFromFile(file, &e);
    }

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

  FileChooser dlg("Save output as", File::nonexistent, "*.txt", true);
  if (!dlg.browseForFileToSave(true))
    return;

  SetupFileWriter writer;
  if (!writer.Initialize(dlg.getResult())) {
    ShowError("Failed to write to the output file.");
    return;
  }

  int count = root_.getNumSubItems();
  for (int i = 0; i < count; ++i)
    writer.AddPreset(root_.getPreset(i)->preset());

  MemoryBlock mem;
  if (!template_file_.loadFileAsData(mem)) {
    ShowError("Failed to read from the template file: " +
        template_file_.getFullPathName());
    return;
  }

  lg::LgParser lg_parser;
  if (!lg_parser.ParseBuffer(&writer, reinterpret_cast<char*>(mem.getData()),
          reinterpret_cast<char*>(mem.getData()) + mem.getSize())) {
    ShowError("No patches found in the template file.\n" +
        template_file_.getFullPathName());
    return;
  }

  NativeMessageBox::showMessageBoxAsync(AlertWindow::InfoIcon,
      "Done",
      "A new setup file has been successfully generated.\n"
      "The next step is to import this file into the LG Control Center.",
      this,
      nullptr);
}

bool MainView::OpenTemplate(const File& f) {
  DBG(__FUNCTION__);
  if (!f.existsAsFile())
    return false;

  template_file_ = f;
  template_path_->setText(f.getFullPathName(), sendNotification);

  return true;
}

bool MainView::OpenSysExFile(const File& f) {
  String err;
  int ret = root_.addPresetsFromFile(f, &err);
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
