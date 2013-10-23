/*
  ==============================================================================

    MicronTabBar.h
    Created: 25 Sep 2013 3:17:59pm
    Author:  Jules

  ==============================================================================
*/

#ifndef MICRONTABBAR_H_INCLUDED
#define MICRONTABBAR_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"

//==============================================================================
/*
	MicronTabBar:
		Custom component which partially supports TabbedComponent interface.
*/
class MicronTabBar    : public Component, public ButtonListener
{
public:
	// NOTE: tabs are always on the left: we're using the orientation to position the labels the next to the buttons.
    explicit MicronTabBar (TabbedButtonBar::Orientation orientation);

	void setTabBarDepth (const int newDepth);
	void setTabBarMargins (const int newMarginVertical, const int newMarginLeft);

	// NOTE: not using sendChangeMessage, only there for compatibility with TabbedComponent interface.
    void setCurrentTabIndex (int newTabIndex, bool sendChangeMessage = true);

	// NOTE: only using tabName, contentComponent, and deleteComponentWhenNotNeeded parameters of the original TabbedComponent interface.
	void addTab (const String& tabName, Colour tabBackgroundColour, Component* const contentComponent, const bool deleteComponentWhenNotNeeded, const int insertIndex = -1);

	void buttonClicked (Button*);

	void resized();

private:
	Button* createButtonForTab();
	Label* createLabelForTab(const String& text);
	int findTabInxFromButton(const Button* button);

	struct Tab
	{
		Tab(Label* label, Button* button, Component* content, bool isContentOwned) : label(label), button(button), content(content), isContentOwned(isContentOwned) { }
		~Tab() { if (isContentOwned) delete content; }

		ScopedPointer<Label>		label;
		ScopedPointer<Button>		button;
		Component*					content;
		bool						isContentOwned;
	};

	OwnedArray<Tab> tabs;

	enum { INVALID_TAB_INX = -1 };
	int curTabIndex;

	TabbedButtonBar::Orientation labelOrientation;
	int tabBarDepth;
	int tabBarMarginVertical;
	int tabBarMarginLeft;

	Image buttonOffImg;
	Image buttonHoverImg;
	Image buttonOnImg;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MicronTabBar)
};


#endif  // MICRONTABBAR_H_INCLUDED
