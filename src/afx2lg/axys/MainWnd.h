// Copyright (c) 2013, Tomas Gunnarsson
// All rights reserved.

// This is a generated file created by the Jucer!

#pragma once
#ifndef __JUCER_HEADER_MAINVIEWBASE_MAINWND_1EDCBC8A__
#define __JUCER_HEADER_MAINVIEWBASE_MAINWND_1EDCBC8A__

//[Headers]     -- You can add your own extra header files here --
#include "juce/JuceHeader.h"
using juce::Component;
using juce::Button;
using juce::ButtonListener;
using juce::TextButton;
using juce::ToggleButton;
using juce::TreeView;
using juce::Graphics;
using juce::GroupComponent;
//[/Headers]


//[Comments]
// The main window.
//[/Comments]
class MainViewBase  : public Component,
                      public ButtonListener {
 public:
  MainViewBase();
  ~MainViewBase();

  //[UserMethods]     -- You can add your own custom methods in this section.
  TextButton* close_btn() { return close_btn_; }
  TreeView* tree_view() { return tree_view_; }
  TextButton* open_btn() { return open_btn_; }
  TextButton* send_btn() { return send_btn_; }
  ToggleButton* edit_buffer_chk() { return edit_buffer_chk_; }
  //[/UserMethods]

  void paint (Graphics& g);
  void resized();
  void buttonClicked (Button* buttonThatWasClicked);


 private:
  //[UserVariables]   -- You can add your own custom variables in this section.
  //[/UserVariables]

  // Member variables added by the jucer.
  GroupComponent* preset_group_;
  TextButton* close_btn_;
  TreeView* tree_view_;
  TextButton* open_btn_;
  TextButton* send_btn_;
  ToggleButton* edit_buffer_chk_;


  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainViewBase);
};

//[EndFile] You can add extra defines here...
//[/EndFile]

#endif   // __JUCER_HEADER_MAINVIEWBASE_MAINWND_1EDCBC8A__
