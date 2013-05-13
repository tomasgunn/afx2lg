// Copyright (c) 2013, Tomas Gunnarsson
// All rights reserved.

// This is a generated file created by the Jucer!

//[Headers] You can add your own extra header files here...
//[/Headers]
%%includeFilesCPP%%

//[MiscUserDefs] You can add your own user definitions and misc code here...
//[/MiscUserDefs]

%%className%%::%%className%%(%%constructorParams%%)
%%initialisers%%{
  %%constructor%%

  //[Constructor] You can add your own custom stuff here..
  //[/Constructor]
}

%%className%%::~%%className%%() {
  //[Destructor_pre]. You can add your own custom destruction code here..
  //[/Destructor_pre]

  %%destructor%%

  //[Destructor]. You can add your own custom destruction code here..
  //[/Destructor]
}
%%methodDefinitions%%
//[MiscUserCode] You can add your own definitions of your custom methods or any other code here...
//[/MiscUserCode]
#if 0
/*  -- Jucer information section --

    This is where the Jucer puts all of its metadata, so don't change anything in here!

BEGIN_JUCER_METADATA

%%metadata%%
END_JUCER_METADATA
*/
#endif
%%staticMemberDefinitions%%
//[EndFile] You can add extra defines here...
//[/EndFile]
