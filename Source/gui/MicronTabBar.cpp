/*
  ==============================================================================

    MicronTabBar.cpp
    Created: 25 Sep 2013 3:17:59pm
    Author:  Jules

  ==============================================================================
*/

#include "../JuceLibraryCode/JuceHeader.h"
#include "MicronTabBar.h"

//==============================================================================
MicronTabBar::MicronTabBar (TabbedButtonBar::Orientation orientation) :
								curTabIndex(INVALID_TAB_INX),
								labelOrientation(orientation),
								tabBarDepth(65), tabBarMarginVertical(30), tabBarMarginLeft(0)
{
    buttonOffImg = ImageCache::getFromMemory (BinaryData::led_button_off_png,
                                           BinaryData::led_button_off_pngSize);
    buttonHoverImg = ImageCache::getFromMemory (BinaryData::led_button_dim_png,
                                           BinaryData::led_button_dim_pngSize);
    buttonOnImg = ImageCache::getFromMemory (BinaryData::led_button_on_png,
                                           BinaryData::led_button_on_pngSize);
}

void MicronTabBar::addTab (const String& tabName,
						  Colour /*tabBackgroundColour*/,
						  Component* const contentComponent,
						  const bool deleteComponentWhenNotNeeded,
						  const int /*insertIndex*/)
{
	Label* label = createLabelForTab(tabName);
	addAndMakeVisible(label);

	Button* button = createButtonForTab();
	button->addListener(this);
	addAndMakeVisible(button);

	if (contentComponent)
		addChildComponent(contentComponent);

	tabs.add( new Tab(label, button, contentComponent, deleteComponentWhenNotNeeded) );

	if (tabs.size() == 1)
		button->triggerClick();
}

void MicronTabBar::setTabBarDepth (const int newDepth)
{
	tabBarDepth = newDepth;
	resized();
}

void MicronTabBar::setTabBarMargins (const int newMarginVertical, const int newMarginLeft)
{
	tabBarMarginVertical = newMarginVertical;
	tabBarMarginLeft = newMarginLeft;
	resized();
}

void MicronTabBar::resized()
{
	const int numTabs = tabs.size();

	int labelsX = 0;
	int buttonsX = 0;
	int labelsOffsetY = 0;
	int buttonsOffsetY = 0;
	int tabHeight = 0;
    Justification labelsJustification = Justification::centred;

	// set up hard-coded (and probably not-quite-right) layout parameters based on the orientation setting
	switch (labelOrientation)
	{
	case TabbedButtonBar::TabsAtTop: // place labels above buttons
		tabHeight = 15;
		buttonsX = 7;
//		labelsX = -7;
		labelsOffsetY = -14;
		labelsJustification = Justification::centredTop;
		break;
		
	case TabbedButtonBar::TabsAtBottom: // place labels below buttons
		tabHeight = 15;
		buttonsX = 7;
//		labelsX = -7;
		labelsOffsetY = 14;
		labelsJustification = Justification::centredTop;
		break;

	case TabbedButtonBar::TabsAtLeft: // place labels left of buttons
		tabHeight = 10;
		labelsX = -7;
		buttonsX = 33;
		labelsJustification = Justification::topRight;
		break;

	case TabbedButtonBar::TabsAtRight: // place labels right of buttons
		tabHeight = 10;
		labelsX = 27;
		labelsJustification = Justification::topLeft;
		break;
	}
	
	const int tabSpacing = (numTabs <= 1) ? 0 : (getHeight() - 2.0f*tabBarMarginVertical - tabHeight) / (numTabs-1);

	int curTabPos = 0.5f*getHeight() - 0.5f*tabSpacing*(numTabs-1) - 0.5f*tabHeight;

	// layout all the labels, buttons, and content components
	for (int tabInx = 0; tabInx < numTabs; ++tabInx)
	{
		tabs[tabInx]->label->setTopLeftPosition(labelsX + tabBarMarginLeft, curTabPos-2+labelsOffsetY);
		tabs[tabInx]->label->setJustificationType(labelsJustification);
		
		tabs[tabInx]->button->setTopLeftPosition(buttonsX + tabBarMarginLeft, curTabPos+buttonsOffsetY);

		if (tabs[tabInx]->content)
			tabs[tabInx]->content->setBounds(tabBarDepth,0, getWidth()-tabBarDepth, getHeight());

		curTabPos += tabSpacing;
	}
}

void MicronTabBar::setCurrentTabIndex (int newTabIndex, bool /*sendChangeMessage*/)
{
	if (newTabIndex != curTabIndex)
	{
		// NOTE: have to manually manage button toggle states here as JUCE's setClickingTogglesState() was behaving unexpectedly.
		if (curTabIndex != INVALID_TAB_INX)
			tabs[curTabIndex]->button->setToggleState(false, dontSendNotification);
		if (newTabIndex != INVALID_TAB_INX)
			tabs[newTabIndex]->button->setToggleState(true, dontSendNotification);

		Component* curComponent = (curTabIndex != INVALID_TAB_INX) ? tabs[curTabIndex]->content : 0;
		Component* newComponent = (newTabIndex != INVALID_TAB_INX) ? tabs[newTabIndex]->content : 0;

		if (curComponent)
		{
			curComponent->setVisible(false);
			curComponent->setEnabled(false);
		}

		if (newComponent)
		{
			newComponent->setVisible(true);
			newComponent->setEnabled(true);
		}

		curTabIndex = newTabIndex;
	}
}

Button* MicronTabBar::createButtonForTab()
{
	ImageButton* button = new ImageButton();
	button->setRadioGroupId(1);
//	button->setClickingTogglesState(true); // NOTE: disabled because it always gives us clicks from the button *before* the current button!
	button->setImages (true, true, false,
						buttonOffImg, 1.0f, Colours::transparentWhite,
						buttonHoverImg, 1.0f, Colours::transparentWhite,
						buttonOnImg, 1.0f, Colours::transparentWhite, 0.0f);
	return button;
}

Label* MicronTabBar::createLabelForTab(const String& text)
{
	Label* label = new Label(String::empty, text);
    label->setFont (Font (13.00f, Font::bold));
    label->setEditable (false, false, false);
    label->setColour (TextEditor::textColourId, Colours::black);
    label->setColour (TextEditor::backgroundColourId, Colour (0x00000000));
	label->setSize(40,20);
	return label;
}

void MicronTabBar::buttonClicked (Button* button)
{
	int tabInx = findTabInxFromButton(button);
	setCurrentTabIndex(tabInx);
}

int MicronTabBar::findTabInxFromButton(const Button* button)
{
	int tabInx = tabs.size();
	while (--tabInx >= 0)
		if (button == tabs[tabInx]->button)
			break;
	jassert(tabInx >= 0 && tabInx < tabs.size());
	return tabInx;
}

