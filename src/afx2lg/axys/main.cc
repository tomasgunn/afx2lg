// Copyright (c) 2013, Tomas Gunnarsson
// All rights reserved.

#include "common/common_types.h"

#include "axys/main_view.h"
#include "juce/JuceHeader.h"

using juce::String;

class MainWnd  : public juce::DocumentWindow {
 public:
  MainWnd()
      : juce::DocumentWindow(
          "Axys", juce::Colours::lightgrey,
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

class AxysApp : public juce::JUCEApplication {
 public:
  AxysApp() {}
  ~AxysApp() {}

  virtual void initialise(const String& cmd) {
    main_wnd_.reset(new MainWnd());
  }

  virtual void shutdown() {
    main_wnd_.reset();
  }

  virtual const String getApplicationName() {
    return "Axys";
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


START_JUCE_APPLICATION(AxysApp)
