// Copyright (c) 2013, Tomas Gunnarsson
// All rights reserved.

// This is a generated file created by the Jucer!

#pragma once
#ifndef %%headerGuard%%
#define %%headerGuard%%

//[Headers]     -- You can add your own extra header files here --
#include "JuceHeader.h"
//[/Headers]
%%includeFilesH%%

//[Comments]
// An auto-generated component, created by the Jucer.
// Describe your class and how it works here!
//[/Comments]
%%classDeclaration%% {
 public:
  %%className%%(%%constructorParams%%);
  ~%%className%%();

  //[UserMethods]     -- You can add your own custom methods in this section.
  //[/UserMethods]
  %%publicMemberDeclarations%%
 protected:
  //[UserVariables]   -- You can add your own custom variables in this section.
  //[/UserVariables]

  // Member variables added by the jucer.
  %%privateMemberDeclarations%%
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(%%className%%);
};

//[EndFile] You can add extra defines here...
//[/EndFile]

#endif   // %%headerGuard%%
