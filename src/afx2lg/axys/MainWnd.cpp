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
      close_btn_ (0),
      tree_view_ (0),
      open_btn_ (0),
      send_btn_ (0),
      edit_buffer_chk_ (0)
{
  addAndMakeVisible (preset_group_ = new GroupComponent ("Presets",
                                                         "presets"));

  addAndMakeVisible (close_btn_ = new TextButton ("Close"));
  close_btn_->setExplicitFocusOrder (4);
  close_btn_->addListener (this);

  addAndMakeVisible (tree_view_ = new TreeView ("tree"));
  tree_view_->setExplicitFocusOrder (5);
  tree_view_->setColour (TreeView::backgroundColourId, Colour (0xfff8f8f8));
  tree_view_->setColour (TreeView::linesColourId, Colours::white);

  addAndMakeVisible (open_btn_ = new TextButton ("Open"));
  open_btn_->setExplicitFocusOrder (1);
  open_btn_->setButtonText ("Open...");
  open_btn_->addListener (this);

  addAndMakeVisible (send_btn_ = new TextButton ("Send"));
  send_btn_->setExplicitFocusOrder (2);
  send_btn_->addListener (this);

  addAndMakeVisible (edit_buffer_chk_ = new ToggleButton ("Edit Buffer"));
  edit_buffer_chk_->setTooltip ("For single presets only - override the target ID by sending it to the edit buffer (no overwrite).");
  edit_buffer_chk_->setExplicitFocusOrder (3);
  edit_buffer_chk_->setButtonText ("Target edit buffer");
  edit_buffer_chk_->addListener (this);
  edit_buffer_chk_->setToggleState (true, false);


  //[UserPreSize]
  //[/UserPreSize]

  setSize (600, 400);


  //[Constructor] You can add your own custom stuff here..
  //[/Constructor]
}

MainViewBase::~MainViewBase() {
  //[Destructor_pre]. You can add your own custom destruction code here..
  //[/Destructor_pre]

  deleteAndZero (preset_group_);
  deleteAndZero (close_btn_);
  deleteAndZero (tree_view_);
  deleteAndZero (open_btn_);
  deleteAndZero (send_btn_);
  deleteAndZero (edit_buffer_chk_);


  //[Destructor]. You can add your own custom destruction code here..
  //[/Destructor]
}

void MainViewBase::paint (Graphics& g)
{
    //[UserPrePaint] Add your own custom painting code here..
    //[/UserPrePaint]

    g.fillAll (Colours::white);

    //[UserPaint] Add your own custom painting code here..
    //[/UserPaint]
}

void MainViewBase::resized()
{
    preset_group_->setBounds (0, 0, 480, 400);
    close_btn_->setBounds (480, 368, 110, 24);
    tree_view_->setBounds (8, 16, 464, 376);
    open_btn_->setBounds (480, 8, 110, 24);
    send_btn_->setBounds (480, 40, 110, 24);
    edit_buffer_chk_->setBounds (480, 64, 104, 24);
    //[UserResized] Add your own custom resize handling here..
    //[/UserResized]
}

void MainViewBase::buttonClicked (Button* buttonThatWasClicked)
{
    //[UserbuttonClicked_Pre]
    DBG(__FUNCTION__);
    //[/UserbuttonClicked_Pre]

    if (buttonThatWasClicked == close_btn_)
    {
        //[UserButtonCode_close_btn_] -- add your button handler code here..
        //[/UserButtonCode_close_btn_]
    }
    else if (buttonThatWasClicked == open_btn_)
    {
        //[UserButtonCode_open_btn_] -- add your button handler code here..
        //[/UserButtonCode_open_btn_]
    }
    else if (buttonThatWasClicked == send_btn_)
    {
        //[UserButtonCode_send_btn_] -- add your button handler code here..
        //[/UserButtonCode_send_btn_]
    }
    else if (buttonThatWasClicked == edit_buffer_chk_)
    {
        //[UserButtonCode_edit_buffer_chk_] -- add your button handler code here..
        //[/UserButtonCode_edit_buffer_chk_]
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
                 fixedSize="1" initialWidth="600" initialHeight="400">
  <BACKGROUND backgroundColour="ffffffff"/>
  <GROUPCOMPONENT name="Presets" id="474e528d5750f64f" memberName="preset_group_"
                  virtualName="" explicitFocusOrder="0" pos="0 0 480 400" title="presets"/>
  <TEXTBUTTON name="Close" id="42801a59912a14a4" memberName="close_btn_" virtualName=""
              explicitFocusOrder="4" pos="480 368 110 24" buttonText="Close"
              connectedEdges="0" needsCallback="1" radioGroupId="0"/>
  <TREEVIEW name="tree" id="47015bbd5cceb2c6" memberName="tree_view_" virtualName=""
            explicitFocusOrder="5" pos="8 16 464 376" backgroundColour="fff8f8f8"
            linecol="ffffffff" rootVisible="1" openByDefault="0"/>
  <TEXTBUTTON name="Open" id="f125429dc95f3592" memberName="open_btn_" virtualName=""
              explicitFocusOrder="1" pos="480 8 110 24" buttonText="Open..."
              connectedEdges="0" needsCallback="1" radioGroupId="0"/>
  <TEXTBUTTON name="Send" id="d2081c26999967e6" memberName="send_btn_" virtualName=""
              explicitFocusOrder="2" pos="480 40 110 24" buttonText="Send"
              connectedEdges="0" needsCallback="1" radioGroupId="0"/>
  <TOGGLEBUTTON name="Edit Buffer" id="a1836a23878e44ca" memberName="edit_buffer_chk_"
                virtualName="" explicitFocusOrder="3" pos="480 64 104 24" tooltip="For single presets only - override the target ID by sending it to the edit buffer (no overwrite)."
                buttonText="Target edit buffer" connectedEdges="0" needsCallback="1"
                radioGroupId="0" state="1"/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif



//[EndFile] You can add extra defines here...
//[/EndFile]
