// Copyright (c) 2013, Tomas Gunnarsson
// All rights reserved.

// This is a generated file created by the Jucer!

#pragma once
#ifndef __JUCER_HEADER_MAINVIEW_MAINWND_FC92E9F8__
#define __JUCER_HEADER_MAINVIEW_MAINWND_FC92E9F8__

//[Headers]     -- You can add your own extra header files here --
#include "juce/JuceHeader.h"
using namespace juce;
//[/Headers]


//[Comments]
// The main window.
//[/Comments]
class MainView  : public Component,
                  public ButtonListener {
 public:
  MainView();
  ~MainView();

  //[UserMethods]     -- You can add your own custom methods in this section.
  void OpenSysEx();
  //[/UserMethods]

  void paint (Graphics& g);
  void resized();
  void buttonClicked (Button* buttonThatWasClicked);
  void visibilityChanged();
  void moved();
  void parentHierarchyChanged();
  void parentSizeChanged();
  void lookAndFeelChanged();
  bool hitTest (int x, int y);
  void broughtToFront();
  void filesDropped (const StringArray& filenames, int mouseX, int mouseY);
  void handleCommandMessage (int commandId);
  void childrenChanged();
  void enablementChanged();
  void mouseMove (const MouseEvent& e);
  void mouseEnter (const MouseEvent& e);
  void mouseExit (const MouseEvent& e);
  void mouseDown (const MouseEvent& e);
  void mouseDrag (const MouseEvent& e);
  void mouseUp (const MouseEvent& e);
  void mouseDoubleClick (const MouseEvent& e);
  void mouseWheelMove (const MouseEvent& e, const MouseWheelDetails& wheel);
  bool keyPressed (const KeyPress& key);
  bool keyStateChanged (const bool isKeyDown);
  void modifierKeysChanged (const ModifierKeys& modifiers);
  void focusGained (FocusChangeType cause);
  void focusLost (FocusChangeType cause);
  void focusOfChildComponentChanged (FocusChangeType cause);
  void inputAttemptWhenModal();


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


  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainView);
};

//[EndFile] You can add extra defines here...
//[/EndFile]

#endif   // __JUCER_HEADER_MAINVIEW_MAINWND_FC92E9F8__
