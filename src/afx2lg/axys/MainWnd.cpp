// Copyright (c) 2013, Tomas Gunnarsson
// All rights reserved.

// This is a generated file created by the Jucer!

//[Headers] You can add your own extra header files here...
//[/Headers]
#include "MainWnd.h"


//[MiscUserDefs] You can add your own user definitions and misc code here...
using namespace juce;
namespace axys {
//[/MiscUserDefs]

MainViewBase::MainViewBase()
    : grid_group_ (0),
      preset_group_ (0),
      tree_view_ (0),
      open_btn_ (0),
      send_btn_ (0),
      edit_buffer_chk_ (0),
      export_all_btn_ (0),
      export_sel_btn_ (0),
      copy_scenes_ (0)
{
  addAndMakeVisible (grid_group_ = new GroupComponent ("gridgroup",
                                                       "Grid"));
  grid_group_->setColour (GroupComponent::outlineColourId, Colour (0x66ffffff));
  grid_group_->setColour (GroupComponent::textColourId, Colours::white);

  addAndMakeVisible (preset_group_ = new GroupComponent ("Presets",
                                                         "Presets (drop .syx files here)"));
  preset_group_->setColour (GroupComponent::outlineColourId, Colour (0x66fffefe));
  preset_group_->setColour (GroupComponent::textColourId, Colours::white);

  addAndMakeVisible (tree_view_ = new TreeView ("tree"));
  tree_view_->setExplicitFocusOrder (5);
  tree_view_->setColour (TreeView::backgroundColourId, Colour (0xfff8f8f8));
  tree_view_->setColour (TreeView::linesColourId, Colours::white);

  addAndMakeVisible (open_btn_ = new TextButton ("Open"));
  open_btn_->setExplicitFocusOrder (1);
  open_btn_->setButtonText ("Open...");
  open_btn_->setConnectedEdges (Button::ConnectedOnRight);
  open_btn_->addListener (this);
  open_btn_->setColour (TextButton::buttonColourId, Colour (0xff58585c));
  open_btn_->setColour (TextButton::textColourOnId, Colours::white);
  open_btn_->setColour (TextButton::textColourOffId, Colours::white);

  addAndMakeVisible (send_btn_ = new TextButton ("Send"));
  send_btn_->setExplicitFocusOrder (2);
  send_btn_->setConnectedEdges (Button::ConnectedOnRight);
  send_btn_->addListener (this);
  send_btn_->setColour (TextButton::buttonColourId, Colour (0xff58585c));
  send_btn_->setColour (TextButton::textColourOnId, Colours::white);
  send_btn_->setColour (TextButton::textColourOffId, Colours::white);

  addAndMakeVisible (edit_buffer_chk_ = new ToggleButton ("Edit Buffer"));
  edit_buffer_chk_->setTooltip ("For single presets only - override the target ID by sending it to the edit buffer (no overwrite).");
  edit_buffer_chk_->setExplicitFocusOrder (3);
  edit_buffer_chk_->setButtonText ("...to edit buffer");
  edit_buffer_chk_->setConnectedEdges (Button::ConnectedOnRight);
  edit_buffer_chk_->addListener (this);
  edit_buffer_chk_->setToggleState (true, false);
  edit_buffer_chk_->setColour (ToggleButton::textColourId, Colours::white);

  addAndMakeVisible (export_all_btn_ = new TextButton ("ExportAll"));
  export_all_btn_->setExplicitFocusOrder (1);
  export_all_btn_->setButtonText ("Save all...");
  export_all_btn_->setConnectedEdges (Button::ConnectedOnRight);
  export_all_btn_->addListener (this);
  export_all_btn_->setColour (TextButton::buttonColourId, Colour (0xff58585c));
  export_all_btn_->setColour (TextButton::textColourOnId, Colours::white);
  export_all_btn_->setColour (TextButton::textColourOffId, Colours::white);

  addAndMakeVisible (export_sel_btn_ = new TextButton ("ExportSel"));
  export_sel_btn_->setExplicitFocusOrder (1);
  export_sel_btn_->setButtonText ("Save selection...");
  export_sel_btn_->setConnectedEdges (Button::ConnectedOnRight);
  export_sel_btn_->addListener (this);
  export_sel_btn_->setColour (TextButton::buttonColourId, Colour (0xff58585c));
  export_sel_btn_->setColour (TextButton::textColourOnId, Colours::white);
  export_sel_btn_->setColour (TextButton::textColourOffId, Colours::white);

  addAndMakeVisible (copy_scenes_ = new TextButton ("CopyScenes"));
  copy_scenes_->setExplicitFocusOrder (2);
  copy_scenes_->setButtonText ("Copy scenes...");
  copy_scenes_->setConnectedEdges (Button::ConnectedOnRight);
  copy_scenes_->addListener (this);
  copy_scenes_->setColour (TextButton::buttonColourId, Colour (0xff58585c));
  copy_scenes_->setColour (TextButton::textColourOnId, Colours::white);
  copy_scenes_->setColour (TextButton::textColourOffId, Colours::white);


  //[UserPreSize]
  //[/UserPreSize]

  setSize (800, 600);


  //[Constructor] You can add your own custom stuff here..
  //[/Constructor]
}

MainViewBase::~MainViewBase() {
  //[Destructor_pre]. You can add your own custom destruction code here..
  //[/Destructor_pre]

  deleteAndZero (grid_group_);
  deleteAndZero (preset_group_);
  deleteAndZero (tree_view_);
  deleteAndZero (open_btn_);
  deleteAndZero (send_btn_);
  deleteAndZero (edit_buffer_chk_);
  deleteAndZero (export_all_btn_);
  deleteAndZero (export_sel_btn_);
  deleteAndZero (copy_scenes_);


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
    grid_group_->setBounds (8, 400, 784, 200);
    preset_group_->setBounds (0, 0, 672, 400);
    tree_view_->setBounds (8, 16, 656, 376);
    open_btn_->setBounds (680, 8, 110, 24);
    send_btn_->setBounds (680, 104, 110, 24);
    edit_buffer_chk_->setBounds (680, 128, 112, 24);
    export_all_btn_->setBounds (680, 40, 110, 24);
    export_sel_btn_->setBounds (680, 72, 110, 24);
    copy_scenes_->setBounds (680, 152, 110, 24);
    //[UserResized] Add your own custom resize handling here..
    //[/UserResized]
}

void MainViewBase::buttonClicked (Button* buttonThatWasClicked)
{
    //[UserbuttonClicked_Pre]
    DBG(__FUNCTION__);
    //[/UserbuttonClicked_Pre]

    if (buttonThatWasClicked == open_btn_)
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
    else if (buttonThatWasClicked == export_all_btn_)
    {
        //[UserButtonCode_export_all_btn_] -- add your button handler code here..
        //[/UserButtonCode_export_all_btn_]
    }
    else if (buttonThatWasClicked == export_sel_btn_)
    {
        //[UserButtonCode_export_sel_btn_] -- add your button handler code here..
        //[/UserButtonCode_export_sel_btn_]
    }
    else if (buttonThatWasClicked == copy_scenes_)
    {
        //[UserButtonCode_copy_scenes_] -- add your button handler code here..
        //[/UserButtonCode_copy_scenes_]
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
                 fixedSize="1" initialWidth="800" initialHeight="600">
  <BACKGROUND backgroundColour="ff200606"/>
  <GROUPCOMPONENT name="gridgroup" id="721debe9e465cc7" memberName="grid_group_"
                  virtualName="" explicitFocusOrder="0" pos="8 400 784 200" outlinecol="66ffffff"
                  textcol="ffffffff" title="Grid"/>
  <GROUPCOMPONENT name="Presets" id="474e528d5750f64f" memberName="preset_group_"
                  virtualName="" explicitFocusOrder="0" pos="0 0 672 400" outlinecol="66fffefe"
                  textcol="ffffffff" title="Presets (drop .syx files here)"/>
  <TREEVIEW name="tree" id="47015bbd5cceb2c6" memberName="tree_view_" virtualName=""
            explicitFocusOrder="5" pos="8 16 656 376" backgroundColour="fff8f8f8"
            linecol="ffffffff" rootVisible="1" openByDefault="0"/>
  <TEXTBUTTON name="Open" id="f125429dc95f3592" memberName="open_btn_" virtualName=""
              explicitFocusOrder="1" pos="680 8 110 24" bgColOff="ff58585c"
              textCol="ffffffff" textColOn="ffffffff" buttonText="Open..."
              connectedEdges="2" needsCallback="1" radioGroupId="0"/>
  <TEXTBUTTON name="Send" id="d2081c26999967e6" memberName="send_btn_" virtualName=""
              explicitFocusOrder="2" pos="680 104 110 24" bgColOff="ff58585c"
              textCol="ffffffff" textColOn="ffffffff" buttonText="Send" connectedEdges="2"
              needsCallback="1" radioGroupId="0"/>
  <TOGGLEBUTTON name="Edit Buffer" id="a1836a23878e44ca" memberName="edit_buffer_chk_"
                virtualName="" explicitFocusOrder="3" pos="680 128 112 24" tooltip="For single presets only - override the target ID by sending it to the edit buffer (no overwrite)."
                txtcol="ffffffff" buttonText="...to edit buffer" connectedEdges="2"
                needsCallback="1" radioGroupId="0" state="1"/>
  <TEXTBUTTON name="ExportAll" id="4ff071a05498b425" memberName="export_all_btn_"
              virtualName="" explicitFocusOrder="1" pos="680 40 110 24" bgColOff="ff58585c"
              textCol="ffffffff" textColOn="ffffffff" buttonText="Save all..."
              connectedEdges="2" needsCallback="1" radioGroupId="0"/>
  <TEXTBUTTON name="ExportSel" id="f9a9b3a0348e775d" memberName="export_sel_btn_"
              virtualName="" explicitFocusOrder="1" pos="680 72 110 24" bgColOff="ff58585c"
              textCol="ffffffff" textColOn="ffffffff" buttonText="Save selection..."
              connectedEdges="2" needsCallback="1" radioGroupId="0"/>
  <TEXTBUTTON name="CopyScenes" id="8b0cef5fb2c14a1" memberName="copy_scenes_"
              virtualName="" explicitFocusOrder="2" pos="680 152 110 24" bgColOff="ff58585c"
              textCol="ffffffff" textColOn="ffffffff" buttonText="Copy scenes..."
              connectedEdges="2" needsCallback="1" radioGroupId="0"/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif

//[EndFile] You can add extra defines here...
}  // namespace axys
//[/EndFile]
