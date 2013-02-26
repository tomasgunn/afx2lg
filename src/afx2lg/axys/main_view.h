// Copyright (c) 2013, Tomas Gunnarsson
// All rights reserved.

#pragma once
#ifndef AXYS_MAIN_VIEW_H_
#define AXYS_MAIN_VIEW_H_

#include "axys/MainWnd.h"
#include "common/common_types.h"

class MainView : public MainViewBase {
 public:
  MainView();
  ~MainView();

 private:
  virtual void buttonClicked(Button* btn);

  void OnOpenSysEx();
  void OnClose();
};

#endif  // AXYS_MAIN_VIEW_H_
