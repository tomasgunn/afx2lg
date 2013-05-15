// Copyright (c) 2013, Tomas Gunnarsson
// All rights reserved.

// This is a generated file created by the Jucer!

#pragma once
#ifndef __JUCER_HEADER_MAINVIEWBASE_MAINWND_3692B25D__
#define __JUCER_HEADER_MAINVIEWBASE_MAINWND_3692B25D__

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
  //[/UserMethods]
  void paint (Graphics& g);
  void resized();
  void buttonClicked (Button* buttonThatWasClicked);


 protected:
  //[UserVariables]   -- You can add your own custom variables in this section.
  //[/UserVariables]

  // Member variables added by the jucer.
  GroupComponent* preset_group_;
  TreeView* tree_view_;
  TextButton* open_btn_;
  TextButton* send_btn_;
  ToggleButton* edit_buffer_chk_;
  TextButton* export_all_btn_;
  TextButton* export_sel_btn_;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainViewBase);
};

//[EndFile] You can add extra defines here...
//[/EndFile]

#endif   // __JUCER_HEADER_MAINVIEWBASE_MAINWND_3692B25D__
