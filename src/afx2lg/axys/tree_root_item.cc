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

void TreeRootItem::sortPresets() {
  ComparePresets comparator;
  sortSubItems(comparator);
}

int TreeRootItem::AddPresetsFromFile(const File& file, String* err) {
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
  bool attempt_to_insert_system_data = false;

  const auto& presets = parser.presets();
  for (auto& p : presets) {
    //  We don't support system data banks.
    if (!p.second->is_global_setting()) {
      PresetItem* item = new PresetItem(p.second);
      // This hands ownership over to the root_.
      addSubItem(item);
      presets_added = true;
    } else {
      attempt_to_insert_system_data = true;
    }
  }

  if (attempt_to_insert_system_data && !presets_added) {
    *err = "Cannot add system backup: " + file.getFullPathName();
    return -1;
  }

  sortPresets();

  return presets_added;
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

  sortPresets();

  // We could do this only if things have actually changed.
  treeHasChanged();
}
