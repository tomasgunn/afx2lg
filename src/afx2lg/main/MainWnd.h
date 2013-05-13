// Copyright (c) 2013, Tomas Gunnarsson
// All rights reserved.

// This is a generated file created by the Jucer!

#pragma once
#ifndef __JUCER_HEADER_MAINVIEWBASE_MAINWND_7D05C021__
#define __JUCER_HEADER_MAINVIEWBASE_MAINWND_7D05C021__

//[Headers]     -- You can add your own extra header files here --
#include "juce/JuceHeader.h"
using namespace juce;
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
  TextButton* generate_btn_;
  Label* template_path_;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainViewBase);
};

//[EndFile] You can add extra defines here...
//[/EndFile]

#endif   // __JUCER_HEADER_MAINVIEWBASE_MAINWND_7D05C021__
