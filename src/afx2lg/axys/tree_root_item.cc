// Copyright (c) 2013, Tomas Gunnarsson
// All rights reserved.

#include "axys/tree_root_item.h"

#include "axefx/axe_fx_sysex_parser.h"
#include "axefx/preset.h"
#include "axys/tree_preset_item.h"

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
}  // namespace

TreeRootItem::TreeRootItem(juce::FileDragAndDropTarget* delegate,
                           bool allow_drag_drop_of_presets,
                           bool allow_edit_buffer_presets)
    : delegate_(delegate),
      allow_drag_drop_of_presets_(allow_drag_drop_of_presets),
      allow_edit_buffer_presets_(allow_edit_buffer_presets) {
}

TreeRootItem::~TreeRootItem() {}

void TreeRootItem::sortPresets() {
  ComparePresets comparator;
  sortSubItems(comparator);
}

int TreeRootItem::addPresetsFromFile(const File& file, String* err) {
  jassert(err);
  if (!file.existsAsFile()) {
    *err = "File not found: " + file.getFullPathName();
    return -1;
  }

  MemoryBlock mem;
  if (!file.loadFileAsData(mem)) {
    *err = "Failed to open file: " + file.getFullPathName();
    return -1;
  }

  SysExParser parser;
  auto data = reinterpret_cast<const uint8_t*>(mem.getData());
  if (!parser.ParseSysExBuffer(data, data + mem.getSize(), true)) {
    *err = "Failed to parse file: " + file.getFullPathName();
    return -1;
  }

  // TODO: Support IR and firmware files.
  if (parser.type() != SysExParser::PRESET &&
      parser.type() != SysExParser::PRESET_ARCHIVE) {
    *err = "Not a preset file: " + file.getFullPathName();
    return -1;
  }

  DBG("Parsed file: " + file.getFullPathName());

  // TODO: Detect duplicates.
  //  - only one preset per ID should be allowed.
  //  - Ignore adding a preset that already exists (exact match)
  //  - When a preset is dropped at a specific location in the tree,
  //    ask if the ID of that preset should be changed.
  //

  bool presets_added = false;
  int failed_add_count = 0;

  const auto& presets = parser.presets();
  for (auto& p : presets) {
    //  We don't support system data banks.
    const auto& preset = p.second;
    bool add = !preset->is_global_setting();
    if (add && !allow_edit_buffer_presets_)
      add = !preset->from_edit_buffer();
    if (add) {
      // This hands ownership over to the root_.
      addSubItem(new PresetItem(preset));
      presets_added = true;
    } else {
      ++failed_add_count;
    }
  }

  if (!presets_added) {
    if (failed_add_count) {
      *err = "Could not add presets from this file: " + file.getFullPathName() +
             "\nEither the file contains system data or preset(s) without an ID.";
    } else {
      *err = "No presets were found in this file :-|";
    }
    return -1;
  }

  sortPresets();

  return true;
}

PresetItem* TreeRootItem::getPreset(int index) const {
  return static_cast<PresetItem*>(getSubItem(index));
}

void TreeRootItem::deleteSelection() {
  int count = getNumSubItems();
  for (int i = count - 1; i >= 0; --i) {
    if (getPreset(i)->isSelected()) {
      removeSubItem(i, true);
    }
  }
}

bool TreeRootItem::isInterestedInDragSource(
    const juce::DragAndDropTarget::SourceDetails& source_details) {
  DBG(__FUNCTION__);

  if (!source_details.description.isInt() ||
      static_cast<int>(source_details.description) !=
          PresetItem::kMagicPresetNumber) {
    return false;
  }

  return allow_drag_drop_of_presets_;
}

void TreeRootItem::itemDropped(
    const juce::DragAndDropTarget::SourceDetails& source_details,
    int insert_index) {
  DBG(__FUNCTION__ << " " << insert_index);
  jassert(source_details.description.isInt());
  jassert(static_cast<int>(source_details.description) ==
          PresetItem::kMagicPresetNumber);

  Array<PresetItem*> selected;
  PresetItem* preset = NULL;

  // Process all selected items _before_ the insert point.
  for (int i = 0; i < insert_index; ++i) {
    preset = getPreset(i);
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
    preset = getPreset(i);
    if (preset->isSelected())
      selected.add(preset);

    if (selected.size())
      preset->set_id(preset->id() + selected.size());
  }

  // Add the currently selected items in reverse order.
  new_id += selected.size();
  for (auto* n : selected)
    n->set_id(--new_id);

  sortPresets();

  // We could do this only if things have actually changed.
  treeHasChanged();
}
