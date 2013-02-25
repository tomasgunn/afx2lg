// Copyright (c) 2013, Tomas Gunnarsson
// All rights reserved.

// This is a generated file created by the Jucer!

//[Headers] You can add your own extra header files here...
//[/Headers]
#include "MainWnd.h"


//[MiscUserDefs] You can add your own user definitions and misc code here...

//[/MiscUserDefs]

MainView::MainView()
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

MainView::~MainView() {
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

void MainView::paint (Graphics& g)
{
    //[UserPrePaint] Add your own custom painting code here..
    //[/UserPrePaint]

    g.fillAll (Colours::white);

    //[UserPaint] Add your own custom painting code here..
    //[/UserPaint]
}

void MainView::resized()
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

void MainView::buttonClicked (Button* buttonThatWasClicked)
{
    //[UserbuttonClicked_Pre]
    DBG(__FUNCTION__);
    //[/UserbuttonClicked_Pre]

    if (buttonThatWasClicked == close_btn_)
    {
        //[UserButtonCode_close_btn_] -- add your button handler code here..
        JUCEApplication::quit();
        //[/UserButtonCode_close_btn_]
    }
    else if (buttonThatWasClicked == open_btn_)
    {
        //[UserButtonCode_open_btn_] -- add your button handler code here..
        OpenSysEx();
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

void MainView::visibilityChanged()
{
    //[UserCode_visibilityChanged] -- Add your code here...
    DBG(__FUNCTION__);
    //[/UserCode_visibilityChanged]
}

void MainView::moved()
{
    //[UserCode_moved] -- Add your code here...
    DBG(__FUNCTION__);
    //[/UserCode_moved]
}

void MainView::parentHierarchyChanged()
{
    //[UserCode_parentHierarchyChanged] -- Add your code here...
    DBG(__FUNCTION__);
    //[/UserCode_parentHierarchyChanged]
}

void MainView::parentSizeChanged()
{
    //[UserCode_parentSizeChanged] -- Add your code here...
    DBG(__FUNCTION__);
    //[/UserCode_parentSizeChanged]
}

void MainView::lookAndFeelChanged()
{
    //[UserCode_lookAndFeelChanged] -- Add your code here...
    DBG(__FUNCTION__);
    //[/UserCode_lookAndFeelChanged]
}

bool MainView::hitTest (int x, int y)
{
    //[UserCode_hitTest] -- Add your code here...
    DBG(__FUNCTION__);
    return true;
    //[/UserCode_hitTest]
}

void MainView::broughtToFront()
{
    //[UserCode_broughtToFront] -- Add your code here...
    DBG(__FUNCTION__);
    //[/UserCode_broughtToFront]
}

void MainView::filesDropped (const StringArray& filenames, int mouseX, int mouseY)
{
    //[UserCode_filesDropped] -- Add your code here...
    DBG(__FUNCTION__);
    //[/UserCode_filesDropped]
}

void MainView::handleCommandMessage (int commandId)
{
    //[UserCode_handleCommandMessage] -- Add your code here...
    DBG(__FUNCTION__);
    //[/UserCode_handleCommandMessage]
}

void MainView::childrenChanged()
{
    //[UserCode_childrenChanged] -- Add your code here...
    DBG(__FUNCTION__);
    //[/UserCode_childrenChanged]
}

void MainView::enablementChanged()
{
    //[UserCode_enablementChanged] -- Add your code here...
    DBG(__FUNCTION__);
    //[/UserCode_enablementChanged]
}

void MainView::mouseMove (const MouseEvent& e)
{
    //[UserCode_mouseMove] -- Add your code here...
    DBG(__FUNCTION__);
    //[/UserCode_mouseMove]
}

void MainView::mouseEnter (const MouseEvent& e)
{
    //[UserCode_mouseEnter] -- Add your code here...
    DBG(__FUNCTION__);
    //[/UserCode_mouseEnter]
}

void MainView::mouseExit (const MouseEvent& e)
{
    //[UserCode_mouseExit] -- Add your code here...
    DBG(__FUNCTION__);
    //[/UserCode_mouseExit]
}

void MainView::mouseDown (const MouseEvent& e)
{
    //[UserCode_mouseDown] -- Add your code here...
    DBG(__FUNCTION__);
    //[/UserCode_mouseDown]
}

void MainView::mouseDrag (const MouseEvent& e)
{
    //[UserCode_mouseDrag] -- Add your code here...
    DBG(__FUNCTION__);
    //[/UserCode_mouseDrag]
}

void MainView::mouseUp (const MouseEvent& e)
{
    //[UserCode_mouseUp] -- Add your code here...
    DBG(__FUNCTION__);
    //[/UserCode_mouseUp]
}

void MainView::mouseDoubleClick (const MouseEvent& e)
{
    //[UserCode_mouseDoubleClick] -- Add your code here...
    DBG(__FUNCTION__);
    //[/UserCode_mouseDoubleClick]
}

void MainView::mouseWheelMove (const MouseEvent& e, const MouseWheelDetails& wheel)
{
    //[UserCode_mouseWheelMove] -- Add your code here...
    DBG(__FUNCTION__);
    //[/UserCode_mouseWheelMove]
}

bool MainView::keyPressed (const KeyPress& key)
{
    //[UserCode_keyPressed] -- Add your code here...
    DBG(__FUNCTION__);
    return false;  // Return true if your handler uses this key event, or false to allow it to be passed-on.
    //[/UserCode_keyPressed]
}

bool MainView::keyStateChanged (const bool isKeyDown)
{
    //[UserCode_keyStateChanged] -- Add your code here...
    DBG(__FUNCTION__);
    return false;  // Return true if your handler uses this key event, or false to allow it to be passed-on.
    //[/UserCode_keyStateChanged]
}

void MainView::modifierKeysChanged (const ModifierKeys& modifiers)
{
    //[UserCode_modifierKeysChanged] -- Add your code here...
    DBG(__FUNCTION__);
    //[/UserCode_modifierKeysChanged]
}

void MainView::focusGained (FocusChangeType cause)
{
    //[UserCode_focusGained] -- Add your code here...
    DBG(__FUNCTION__);
    //[/UserCode_focusGained]
}

void MainView::focusLost (FocusChangeType cause)
{
    //[UserCode_focusLost] -- Add your code here...
    DBG(__FUNCTION__);
    //[/UserCode_focusLost]
}

void MainView::focusOfChildComponentChanged (FocusChangeType cause)
{
    //[UserCode_focusOfChildComponentChanged] -- Add your code here...
    DBG(__FUNCTION__);
    //[/UserCode_focusOfChildComponentChanged]
}

void MainView::inputAttemptWhenModal()
{
    //[UserCode_inputAttemptWhenModal] -- Add your code here...
    DBG(__FUNCTION__);
    //[/UserCode_inputAttemptWhenModal]
}



//[MiscUserCode] You can add your own definitions of your custom methods or any other code here...

void MainView::OpenSysEx() {
  FileChooser dlg("Open SysEx file", File::nonexistent, "*.syx", true);
  if (dlg.browseForFileToOpen()) {
    File f(dlg.getResult());
    if (f.exists()) {
      // tbd.
    }
  }
}

//[/MiscUserCode]

#if 0
/*  -- Jucer information section --

    This is where the Jucer puts all of its metadata, so don't change anything in here!

BEGIN_JUCER_METADATA

<JUCER_COMPONENT documentType="Component" className="MainView" componentName=""
                 parentClasses="public Component" constructorParams="" variableInitialisers=""
                 snapPixels="8" snapActive="1" snapShown="1" overlayOpacity="0.330000013"
                 fixedSize="1" initialWidth="600" initialHeight="400">
  <METHODS>
    <METHOD name="visibilityChanged()"/>
    <METHOD name="moved()"/>
    <METHOD name="parentHierarchyChanged()"/>
    <METHOD name="parentSizeChanged()"/>
    <METHOD name="lookAndFeelChanged()"/>
    <METHOD name="hitTest (int x, int y)"/>
    <METHOD name="broughtToFront()"/>
    <METHOD name="filesDropped (const StringArray&amp; filenames, int mouseX, int mouseY)"/>
    <METHOD name="handleCommandMessage (int commandId)"/>
    <METHOD name="childrenChanged()"/>
    <METHOD name="enablementChanged()"/>
    <METHOD name="mouseMove (const MouseEvent&amp; e)"/>
    <METHOD name="mouseEnter (const MouseEvent&amp; e)"/>
    <METHOD name="mouseExit (const MouseEvent&amp; e)"/>
    <METHOD name="mouseDown (const MouseEvent&amp; e)"/>
    <METHOD name="mouseDrag (const MouseEvent&amp; e)"/>
    <METHOD name="mouseUp (const MouseEvent&amp; e)"/>
    <METHOD name="mouseDoubleClick (const MouseEvent&amp; e)"/>
    <METHOD name="mouseWheelMove (const MouseEvent&amp; e, const MouseWheelDetails&amp; wheel)"/>
    <METHOD name="keyPressed (const KeyPress&amp; key)"/>
    <METHOD name="keyStateChanged (const bool isKeyDown)"/>
    <METHOD name="modifierKeysChanged (const ModifierKeys&amp; modifiers)"/>
    <METHOD name="inputAttemptWhenModal()"/>
    <METHOD name="focusOfChildComponentChanged (FocusChangeType cause)"/>
    <METHOD name="focusLost (FocusChangeType cause)"/>
    <METHOD name="focusGained (FocusChangeType cause)"/>
  </METHODS>
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
