// Copyright (c) 2013, Tomas Gunnarsson
// All rights reserved.

#include "axefx/axe_fx_sysex_parser.h"
#include "common/file_utils.h"
#include "juce/JuceHeader.h"
#include "lg/lg_parser.h"
#include "main/main_view.h"

// TODO: Rename this 'main' folder!

using juce::String;

// TODO: Consolidate all the boilerplate juce code between this app and Axys.

class MainWnd  : public juce::DocumentWindow {
 public:
  MainWnd()
      : juce::DocumentWindow(
          "Afx2LG", juce::Colours::darkgrey,
          DocumentWindow::minimiseButton | DocumentWindow::closeButton,
          true) {
    // Create an instance of our main content component, and add it to our window..
    setContentOwned(new MainView(), true);
    centreWithSize(getWidth(), getHeight());

    setVisible(true);
  }

  ~MainWnd() {
    // (the content component will be deleted automatically)
  }

  virtual void closeButtonPressed() {
    juce::JUCEApplication::quit();
  }
};

class Afx2LGApp : public juce::JUCEApplication {
 public:
  Afx2LGApp() {}
  ~Afx2LGApp() {}

  virtual void initialise(const String& cmd) {
    main_wnd_.reset(new MainWnd());
  }

  virtual void shutdown() {
    main_wnd_.reset();
  }

  virtual const String getApplicationName() {
    return "Afx2LG";
  }

  virtual const String getApplicationVersion() {
    return "0.0.0.1";
  }

  virtual bool moreThanOneInstanceAllowed() {
    return true;
  }

  virtual void anotherInstanceStarted(const String& cmd) {
  }

 private:
  std::unique_ptr<MainWnd> main_wnd_;
};

START_JUCE_APPLICATION(Afx2LGApp)
