/*
  ==============================================================================

    LookAndFeel.h
    Created: 20 Sep 2013 6:40:04pm
    Author:  Jules

  ==============================================================================
*/

#ifndef LOOKANDFEEL_H_INCLUDED
#define LOOKANDFEEL_H_INCLUDED

#include "JuceHeader.h"

/*
	PluginLookAndFeel:
		LookAndFeel subclass to give components their customized appearance for micronAU.
*/

class PluginLookAndFeel : public LookAndFeel, public DeletedAtShutdown
{
public:
	juce_DeclareSingleton (PluginLookAndFeel, false);
	
    PluginLookAndFeel();

    void drawRotarySlider (Graphics &g, int x, int y, int width, int height, float sliderPosProportional, float rotaryStartAngle, float rotaryEndAngle, Slider &slider);

    void drawPopupMenuBackground (Graphics &g, int width, int height);
	void drawPopupMenuItem (Graphics& g, int width, int height, const bool isSeparator, const bool isActive,
                                     const bool isHighlighted, const bool isTicked, const bool hasSubMenu,
                                     const String& text, const String& shortcutKeyText, Image* image, const Colour* const textColourToUse);

	void positionComboBoxText (ComboBox& box, Label& label);
	void drawComboBox (Graphics& g, int width, int height, const bool isButtonDown, int buttonX, int buttonY, int buttonW, int buttonH, ComboBox& box);

	void drawToggleButton (Graphics& g, ToggleButton& button, bool isMouseOverButton, bool isButtonDown);

	void drawGroupComponentOutline (Graphics& g, int width, int height, const String& text, const Justification& position, GroupComponent& group);

	void drawLinearSliderBackground (Graphics& g, int x, int y, int width, int height, float /*sliderPos*/, float /*minSliderPos*/, float /*maxSliderPos*/, const Slider::SliderStyle /*style*/, Slider& slider);
	void drawLinearSliderThumb (Graphics& g, int x, int y, int width, int height, float sliderPos, float minSliderPos, float maxSliderPos, const Slider::SliderStyle style, Slider& slider);

	void drawLabel (Graphics& g, Label& label);

	void fillTextEditorBackground (Graphics& g, int width, int height, TextEditor& textEditor);
	void drawTextEditorOutline (Graphics& g, int width, int height, TextEditor& textEditor);

private:
	LookAndFeel stdLookAndFeel; // for falling back on the standard look and feel when needed.

    Image blackKnob;
    Image blackKnobMarker;
	Image toggleButtonOff;
	Image toggleButtonOn;
	Image sliderThumb;
	Image lcdPanel;
};

#endif  // LOOKANDFEEL_H_INCLUDED
