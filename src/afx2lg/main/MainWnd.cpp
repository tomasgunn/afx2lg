// Copyright (c) 2013, Tomas Gunnarsson
// All rights reserved.

// This is a generated file created by the Jucer!

//[Headers] You can add your own extra header files here...
//[/Headers]
#include "MainWnd.h"


//[MiscUserDefs] You can add your own user definitions and misc code here...
using namespace juce;
//[/MiscUserDefs]

MainViewBase::MainViewBase()
    : preset_group_ (0),
      tree_view_ (0),
      open_btn_ (0),
      generate_btn_ (0),
      template_path_ (0)
{
  addAndMakeVisible (preset_group_ = new GroupComponent ("Presets",
                                                         "#2 Select presets (drop .syx files here)"));
  preset_group_->setColour (GroupComponent::outlineColourId, Colour (0x66fffefe));
  preset_group_->setColour (GroupComponent::textColourId, Colours::white);

  addAndMakeVisible (tree_view_ = new TreeView ("tree"));
  tree_view_->setExplicitFocusOrder (3);
  tree_view_->setColour (TreeView::backgroundColourId, Colour (0xfff8f8f8));
  tree_view_->setColour (TreeView::linesColourId, Colours::white);

  addAndMakeVisible (open_btn_ = new TextButton ("OpenTemplate"));
  open_btn_->setExplicitFocusOrder (1);
  open_btn_->setButtonText ("#1 Open LG setup template (exported from Control Center)");
  open_btn_->addListener (this);
  open_btn_->setColour (TextButton::buttonColourId, Colour (0xff58585c));
  open_btn_->setColour (TextButton::textColourOnId, Colours::white);
  open_btn_->setColour (TextButton::textColourOffId, Colours::white);

  addAndMakeVisible (generate_btn_ = new TextButton ("Generate"));
  generate_btn_->setExplicitFocusOrder (4);
  generate_btn_->setButtonText ("#3 Generate new setup file (import this file into Control Center)");
  generate_btn_->addListener (this);
  generate_btn_->setColour (TextButton::buttonColourId, Colour (0xff58585c));
  generate_btn_->setColour (TextButton::textColourOnId, Colours::white);
  generate_btn_->setColour (TextButton::textColourOffId, Colours::white);

  addAndMakeVisible (template_path_ = new Label ("path",
                                                 "<template path>"));
  template_path_->setExplicitFocusOrder (2);
  template_path_->setFont (Font (15.0000f, Font::plain));
  template_path_->setJustificationType (Justification::centred);
  template_path_->setEditable (false, false, false);
  template_path_->setColour (Label::textColourId, Colours::white);
  template_path_->setColour (Label::outlineColourId, Colour (0x93c3bcbc));
  template_path_->setColour (TextEditor::textColourId, Colours::black);
  template_path_->setColour (TextEditor::backgroundColourId, Colour (0x0));


  //[UserPreSize]
  //[/UserPreSize]

  setSize (480, 370);


  //[Constructor] You can add your own custom stuff here..
  //[/Constructor]
}

MainViewBase::~MainViewBase() {
  //[Destructor_pre]. You can add your own custom destruction code here..
  //[/Destructor_pre]

  deleteAndZero (preset_group_);
  deleteAndZero (tree_view_);
  deleteAndZero (open_btn_);
  deleteAndZero (generate_btn_);
  deleteAndZero (template_path_);


  //[Destructor]. You can add your own custom destruction code here..
  //[/Destructor]
}
void MainViewBase::paint (Graphics& g)
{
    //[UserPrePaint] Add your own custom painting code here..
    //[/UserPrePaint]

    g.fillAll (Colour (0xff200606));

    //[UserPaint] Add your own custom painting code here..
    //[/UserPaint]
}

void MainViewBase::resized()
{
    preset_group_->setBounds (0, 64, 480, 264);
    tree_view_->setBounds (8, 80, 464, 240);
    open_btn_->setBounds (8, 8, 464, 24);
    generate_btn_->setBounds (8, 336, 464, 24);
    template_path_->setBounds (8, 40, 464, 24);
    //[UserResized] Add your own custom resize handling here..
    //[/UserResized]
}

void MainViewBase::buttonClicked (Button* buttonThatWasClicked)
{
    //[UserbuttonClicked_Pre]
    //[/UserbuttonClicked_Pre]

    if (buttonThatWasClicked == open_btn_)
    {
        //[UserButtonCode_open_btn_] -- add your button handler code here..
        //[/UserButtonCode_open_btn_]
    }
    else if (buttonThatWasClicked == generate_btn_)
    {
        //[UserButtonCode_generate_btn_] -- add your button handler code here..
        //[/UserButtonCode_generate_btn_]
    }

    //[UserbuttonClicked_Post]
    //[/UserbuttonClicked_Post]
}


//[MiscUserCode] You can add your own definitions of your custom methods or any other code here...
//[/MiscUserCode]
#if 0
/*  -- Jucer information section --

    This is where the Jucer puts all of its metadata, so don't change anything in here!

BEGIN_JUCER_METADATA

<JUCER_COMPONENT documentType="Component" className="MainViewBase" componentName=""
                 parentClasses="public Component" constructorParams="" variableInitialisers=""
                 snapPixels="8" snapActive="1" snapShown="1" overlayOpacity="0.330000013"
                 fixedSize="1" initialWidth="480" initialHeight="370">
  <BACKGROUND backgroundColour="ff200606"/>
  <GROUPCOMPONENT name="Presets" id="474e528d5750f64f" memberName="preset_group_"
                  virtualName="" explicitFocusOrder="0" pos="0 64 480 264" outlinecol="66fffefe"
                  textcol="ffffffff" title="#2 Select presets (drop .syx files here)"/>
  <TREEVIEW name="tree" id="47015bbd5cceb2c6" memberName="tree_view_" virtualName=""
            explicitFocusOrder="3" pos="8 80 464 240" backgroundColour="fff8f8f8"
            linecol="ffffffff" rootVisible="1" openByDefault="0"/>
  <TEXTBUTTON name="OpenTemplate" id="f125429dc95f3592" memberName="open_btn_"
              virtualName="" explicitFocusOrder="1" pos="8 8 464 24" bgColOff="ff58585c"
              textCol="ffffffff" textColOn="ffffffff" buttonText="#1 Open LG setup template (exported from Control Center)"
              connectedEdges="0" needsCallback="1" radioGroupId="0"/>
  <TEXTBUTTON name="Generate" id="f9a9b3a0348e775d" memberName="generate_btn_"
              virtualName="" explicitFocusOrder="4" pos="8 336 464 24" bgColOff="ff58585c"
              textCol="ffffffff" textColOn="ffffffff" buttonText="#3 Generate new setup file (import this file into Control Center)"
              connectedEdges="0" needsCallback="1" radioGroupId="0"/>
  <LABEL name="path" id="5fccaea59fd5ec0" memberName="template_path_"
         virtualName="" explicitFocusOrder="2" pos="8 40 464 24" textCol="ffffffff"
         outlineCol="93c3bcbc" edTextCol="ff000000" edBkgCol="0" labelText="&lt;template path&gt;"
         editableSingleClick="0" editableDoubleClick="0" focusDiscardsChanges="0"
         fontname="Default font" fontsize="15" bold="0" italic="0" justification="36"/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif

//[EndFile] You can add extra defines here...
//[/EndFile]
