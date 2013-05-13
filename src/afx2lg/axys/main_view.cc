// Copyright (c) 2013, Tomas Gunnarsson
// All rights reserved.

#include "axys/main_view.h"

#include "axefx/preset.h"

using axefx::SysExParser;
using namespace juce;

namespace {
class ComparePresets {
 public:
  int compareElements(TreeViewItem* first, TreeViewItem* second) {
    auto p1 = static_cast<const PresetItem*>(first);
    auto p2 = static_cast<const PresetItem*>(second);
    if (p1->id() < p2->id())
      return -1;

    if (p1->id() > p2->id())
      return 1;

    return 0;
  }
};

// Used to identify when presets are dropped from the tree view.
const int kMagicPresetNumber = 0xF321;
}  // namespace

bool TreeRootItem::isInterestedInDragSource(
    const juce::DragAndDropTarget::SourceDetails& source_details) {
  DBG(__FUNCTION__);

  if (!source_details.description.isInt() ||
      static_cast<int>(source_details.description) != kMagicPresetNumber) {
    return false;
  }

  return true;
}

void TreeRootItem::itemDropped(
    const juce::DragAndDropTarget::SourceDetails& source_details,
    int insert_index) {
  DBG(__FUNCTION__ << " " << insert_index);
  jassert(source_details.description.isInt());
  jassert(static_cast<int>(source_details.description) == kMagicPresetNumber);

  Array<PresetItem*> selected;
  PresetItem* preset = NULL;

  // Process all selected items _before_ the insert point.
  for (int i = 0; i < insert_index; ++i) {
    preset = static_cast<PresetItem*>(getSubItem(i));
    if (preset->isSelected())
      selected.add(preset);

    if (selected.size())
      preset->set_id(preset->id() - selected.size());
  }

  int new_id = preset ? preset->id() + 1 : 0;
  for (auto* n : selected)
    n->set_id(new_id++);
  selected.clear();

  int count = getNumSubItems();
  jassert(count > 0);  // otherwise we shouldn't have reached here.

  for (int i = count - 1; i >= insert_index; --i) {
    preset = static_cast<PresetItem*>(getSubItem(i));
    if (preset->isSelected())
      selected.add(preset);

    if (selected.size())
      preset->set_id(preset->id() + selected.size());
  }

  // Add the currently selected items in reverse order.
  new_id += selected.size();
  for (auto* n : selected)
    n->set_id(--new_id);

  ComparePresets comparator;
  sortSubItems(comparator);

  // We could do this only if things have actually changed.
  treeHasChanged();
}

PresetItem::PresetItem(const shared_ptr<axefx::Preset>& preset)
    : preset_(preset) {
}

PresetItem::~PresetItem() {
  DBG(__FUNCTION__);
}

int PresetItem::id() const {
  if (preset_->from_edit_buffer())
    return -1;
  return preset_->id();
}

void PresetItem::set_id(int id) {
  if (id < 0)
    preset_->SetAsEditBuffer();
  else
    preset_->set_id(id);
}

bool PresetItem::mightContainSubItems() {
  return false;  // For now at least.
}

bool PresetItem::isInterestedInDragSource(
    const DragAndDropTarget::SourceDetails& source_details) {
  DBG(__FUNCTION__);
  return false;
}

void PresetItem::itemDropped(
    const juce::DragAndDropTarget::SourceDetails& source_details,
    int insert_index) {
  DBG(__FUNCTION__ << " " << insert_index);
}

var PresetItem::getDragSourceDescription() {
  return var(kMagicPresetNumber);
}

void PresetItem::paintItem(Graphics& g, int width, int height) {
  // if this item is selected, fill it with a background colour..
  if (isSelected())
    g.fillAll(Colours::blue.withAlpha(0.3f));

  // g.setColour(Colour(0, 0, 0));
  // g.setFont(height * 0.7f);

  // TODO: Somehow support using either 0 based or 1 based IDs.
  String label;
  if (preset_->from_edit_buffer()) {
    label = "? ";
  } else {
    label = String(preset_->id());
    label += " ";
  }
  label += preset_->name().c_str();
  g.drawText(label, 0, 0, width, height, Justification::centredLeft, true);
}

////////////////////////////////////////////////////////////////////////////////

MainView::MainView() : root_(this) {
  tree_view()->setRootItem(&root_);
  tree_view()->setRootItemVisible(false);
  tree_view()->setMultiSelectEnabled(true);
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
  if (!file.existsAsFile())
    return false;

  MemoryBlock mem;
  if (!file.loadFileAsData(mem)) {
    ShowError("Failed to open file: " + file.getFullPathName());
    return false;
  }

  SysExParser parser;
  auto data = reinterpret_cast<const uint8_t*>(mem.getData());
  if (!parser.ParseSysExBuffer(data, data + mem.getSize(), true)) {
    ShowError("Failed to parse file: " + file.getFullPathName());
    return false;
  }

  // TODO: Support IR and firmware files.
  if (parser.type() != SysExParser::PRESET &&
      parser.type() != SysExParser::PRESET_ARCHIVE) {
    ShowError("Sorry, only preset files are supported right now.");
    return false;
  }

  DBG("Parsed file: " + file.getFullPathName());

  // TODO: Detect duplicates.
  //  - only one preset per ID should be allowed.
  //  - Ignore adding a preset that already exists (exact match)
  //  - When a preset is dropped at a specific location in the tree,
  //    ask if the ID of that preset should be changed.
  //

  bool presets_added = false;
  bool attempt_to_insert_system_data = false;

  const auto& presets = parser.presets();
  for (auto& p : presets) {
    //  We don't support system data banks.
    if (!p.second->is_global_setting()) {
      PresetItem* item = new PresetItem(p.second);
      // This hands ownership over to the root_.
      root_.addSubItem(item);
      presets_added = true;
    } else {
      attempt_to_insert_system_data = true;
    }
  }

  if (attempt_to_insert_system_data) {
    ShowError("Sorry, backups of system data can't be added as presets.");
  }

  ComparePresets comparator;
  root_.sortSubItems(comparator);

  return presets_added;
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
