/*
  ==============================================================================

    LookAndFeel.cpp
    Created: 20 Sep 2013 6:40:04pm
    Author:  Jules

  ==============================================================================
*/

#include "LookAndFeel.h"
#include "LcdLabel.h"
#include "LcdComboBox.h"
#include "MicronSlider.h"

juce_ImplementSingleton (PluginLookAndFeel);

PluginLookAndFeel::PluginLookAndFeel() {
    blackKnob = ImageCache::getFromMemory (BinaryData::knobBack_png,
                                           BinaryData::knobBack_pngSize);
    blackKnobMarker = ImageCache::getFromMemory (BinaryData::knob_png,
                                     BinaryData::knob_pngSize);
	toggleButtonOff = ImageCache::getFromMemory (BinaryData::pbutton0_png,
                                           BinaryData::pbutton0_pngSize);
	toggleButtonOn = ImageCache::getFromMemory (BinaryData::pbutton1_png,
                                           BinaryData::pbutton1_pngSize);
	sliderThumb = ImageCache::getFromMemory (BinaryData::sliderThumb_png,
                                           BinaryData::sliderThumb_pngSize);
	lcdPanel = ImageCache::getFromMemory (BinaryData::lcd_panel_png,
                                           BinaryData::lcd_panel_pngSize);

	setColour(PopupMenu::highlightedBackgroundColourId, Colour(0xff2050f0));
	setColour(PopupMenu::highlightedTextColourId, Colours::white);
	setColour(PopupMenu::backgroundColourId, Colours::transparentWhite); // transparent allows us to draw the rounded rectangle instead
}

void PluginLookAndFeel::drawRotarySlider (Graphics &g,
                                          int	x,
                                          int 	y,
                                          int 	width,
                                          int 	height,
                                          float sliderPosProportional,
                                          float rotaryStartAngle,
                                          float rotaryEndAngle,
                                          Slider &slider) {
    const Image& k = blackKnob;
    int centerX = x + width /2;
    int centerY = y + height/2;
    AffineTransform t;
    
    g.drawImageAt(k, centerX - k.getBounds().getCentreX(), centerY - k.getBounds().getCentreY());
    
    t = t.rotated(rotaryStartAngle + (sliderPosProportional * (rotaryEndAngle - rotaryStartAngle)), blackKnobMarker.getBounds().getCentreX(), blackKnobMarker.getBounds().getCentreY());
    t = t.translated(centerX - k.getBounds().getCentreX(), centerY - k.getBounds().getCentreY());
    g.drawImageTransformed(blackKnobMarker, t);
}

void PluginLookAndFeel::drawPopupMenuBackground (Graphics &g, int width, int height)
{
	g.setColour(Colours::white);
    g.fillRoundedRectangle (1, 0, width-2, height, 4.0f);
}

void PluginLookAndFeel::drawPopupMenuItem (Graphics& g,
                                     int width, int height,
                                     const bool isSeparator,
                                     const bool isActive,
                                     const bool isHighlighted,
                                     const bool isTicked,
                                     const bool hasSubMenu,
                                     const String& text,
                                     const String& shortcutKeyText,
                                     Image* image,
                                     const Colour* const textColourToUse)
{
    const float halfH = height * 0.5f;

    if (isSeparator)
    {
        const float separatorIndent = 5.5f;

        g.setColour (Colour (0x33000000));
        g.drawLine (separatorIndent, halfH, width - separatorIndent, halfH);

        g.setColour (Colour (0x66ffffff));
        g.drawLine (separatorIndent, halfH + 1.0f, width - separatorIndent, halfH + 1.0f);
    }
    else
    {
        Colour textColour (findColour (PopupMenu::textColourId));

        if (textColourToUse != nullptr)
            textColour = *textColourToUse;

        if (isHighlighted)
        {
			// NOTE: This is the only difference from the standard L&F method - using a gradient highlight.
			Colour baseColour = findColour(PopupMenu::highlightedBackgroundColourId);
			ColourGradient gradient = ColourGradient(baseColour.overlaidWith( Colours::white.withAlpha(0.4f) ), 0, 0,
													baseColour, 0, height,
													false);
			g.setGradientFill(gradient);

//            g.setColour (findColour (PopupMenu::highlightedBackgroundColourId));
            g.fillRect (1, 1, width - 2, height - 2);

            g.setColour (findColour (PopupMenu::highlightedTextColourId));
        }
        else
        {
            g.setColour (textColour);
        }

        if (! isActive)
            g.setOpacity (0.3f);

        Font font (getPopupMenuFont());

        if (font.getHeight() > height / 1.3f)
            font.setHeight (height / 1.3f);

        g.setFont (font);

        const int leftBorder = (height * 5) / 4;
        const int rightBorder = 4;

        if (image != nullptr)
        {
            g.drawImageWithin (*image,
                               2, 1, leftBorder - 4, height - 2,
                               RectanglePlacement::centred | RectanglePlacement::onlyReduceInSize, false);
        }
        else if (isTicked)
        {
            const Path tick (getTickShape (1.0f));
            const float th = font.getAscent();
            const float ty = halfH - th * 0.5f;

            g.fillPath (tick, tick.getTransformToScaleToFit (2.0f, ty, (float) (leftBorder - 4),
                                                             th, true));
        }

        g.drawFittedText (text,
                          leftBorder, 0, width - (leftBorder + rightBorder), height,
                          Justification::centredLeft, 1);

        if (shortcutKeyText.isNotEmpty())
        {
            Font f2 (font);
            f2.setHeight (f2.getHeight() * 0.75f);
            f2.setHorizontalScale (0.95f);
            g.setFont (f2);

            g.drawText (shortcutKeyText,
                        leftBorder, 0, width - (leftBorder + rightBorder + 4), height,
                        Justification::centredRight,
                        true);
        }

        if (hasSubMenu)
        {
            const float arrowH = 0.6f * getPopupMenuFont().getAscent();
            const float x = width - height * 0.6f;

            Path p;
            p.addTriangle (x, halfH - arrowH * 0.5f,
                           x, halfH + arrowH * 0.5f,
                           x + arrowH * 0.6f, halfH);

            g.fillPath (p);
        }
    }
}

void PluginLookAndFeel::positionComboBoxText (ComboBox& box, Label& label)
{
	LcdComboBox* lcdComboBox = dynamic_cast<LcdComboBox*>(&box);
	if ( ! lcdComboBox )
	{	// fall back on standard look and feel.
		stdLookAndFeel.positionComboBoxText(box, label);
		return;
	}

	label.setBounds (1, 0,
					 box.getWidth() - 4,
					 box.getHeight() - 2);

	label.setFont (getComboBoxFont (box));
}

void PluginLookAndFeel::drawComboBox (Graphics& g, int width, int height,
                                const bool isButtonDown,
                                int buttonX, int buttonY,
                                int buttonW, int buttonH,
                                ComboBox& box)
{
	LcdComboBox* lcdComboBox = dynamic_cast<LcdComboBox*>(&box);

	if (lcdComboBox)
	{
		g.drawImageWithin(lcdPanel, 0, 0, width, height, RectanglePlacement(RectanglePlacement::stretchToFit));
	}
	else
	{
		const float cornerSize = 4.0f;
		ColourGradient gradient = ColourGradient(Colour(0xfff8f8f8), 0, 0.35*height,
												Colour(0xffe0e0e0), 0, 0.65*height,
												false);
		g.setGradientFill(gradient);

		g.fillRoundedRectangle(1, 1, width-2, height-2, cornerSize);
		
		if (box.isEnabled() && box.hasKeyboardFocus (false))
		{
			g.setColour (box.findColour (ComboBox::buttonColourId));
			g.drawRoundedRectangle (1, 1, width, height, cornerSize, 2.0f);
		}
		else
		{
			g.setColour (Colours::black);
			g.drawRoundedRectangle (1.5, 1.5, width-2, height-2, cornerSize, 0.5f);
		}

		if (box.isEnabled())
		{
			const float arrowX = 0.3f;
			const float arrowH = 0.2f;

			Path p;
			p.addTriangle (buttonX + buttonW * 0.5f,            buttonY + buttonH * (0.45f - arrowH),
						   buttonX + buttonW * (1.0f - arrowX), buttonY + buttonH * 0.45f,
						   buttonX + buttonW * arrowX,          buttonY + buttonH * 0.45f);

			p.addTriangle (buttonX + buttonW * 0.5f,            buttonY + buttonH * (0.55f + arrowH),
						   buttonX + buttonW * (1.0f - arrowX), buttonY + buttonH * 0.55f,
						   buttonX + buttonW * arrowX,          buttonY + buttonH * 0.55f);

			g.setColour (box.findColour (ComboBox::arrowColourId));
			g.fillPath (p);
		}
	}
}

void PluginLookAndFeel::drawToggleButton (Graphics& g,
                                    ToggleButton& button,
                                    bool isMouseOverButton,
                                    bool isButtonDown)
{
	// TODO: we really don't even want any keyboard focus to land on the button, keyfocus should fall to main window then to host.
    if (button.hasKeyboardFocus (true))
    {
        g.setColour (button.findColour (TextEditor::focusedOutlineColourId));
        g.drawRect (0, 0, button.getWidth(), button.getHeight());
    }

	const Image& buttonImg = button.getToggleState() ? toggleButtonOn : toggleButtonOff;
    g.drawImageAt(buttonImg, 2.0f, 1.0f + (button.getHeight() - buttonImg.getHeight()) * 0.5f);


    float fontSize = jmin (15.0f, button.getHeight() * 0.75f);

    const float tickWidth = fontSize * 1.1f;
/*
    drawTickBox (g, button, 4.0f, (button.getHeight() - tickWidth) * 0.5f,
                 tickWidth, tickWidth,
                 button.getToggleState(),
                 button.isEnabled(),
                 isMouseOverButton,
                 isButtonDown);
*/
    g.setColour (button.findColour (ToggleButton::textColourId));
    g.setFont (fontSize);

    if (! button.isEnabled())
        g.setOpacity (0.5f);

    const int textX = (int) tickWidth + 5;

    g.drawFittedText (button.getButtonText(),
                      textX, 0,
                      button.getWidth() - textX - 2, button.getHeight(),
                      Justification::centredLeft, 10);
}

void PluginLookAndFeel::drawGroupComponentOutline (Graphics& g, int width, int height,
                                             const String& text,
                                             const Justification& position,
                                             GroupComponent& group)
{
    const float textH = 15.0f;
    const float indent = 3.0f;
    const float textEdgeGap = 4.0f;
    float cs = 4.0f;

    Font f (textH);

    int x = indent;
    int y = f.getAscent() - 3.0f;
    int w = jmax (0.0f, width - x * 2.0f);
    int h = jmax (0.0f, height - y  - indent);
    cs = jmin (cs, w * 0.5f, h * 0.5f);
    const float cs2 = 2.0f * cs;

    float textW = text.isEmpty() ? 0 : jlimit (0.0f, jmax (0.0f, w - cs2 - textEdgeGap * 2), f.getStringWidth (text) + textEdgeGap * 2.0f);
    float textX = cs + textEdgeGap;

    if (position.testFlags (Justification::horizontallyCentred))
        textX = cs + (w - cs2 - textW) * 0.5f;
    else if (position.testFlags (Justification::right))
        textX = w - cs - textW - textEdgeGap;

    const float alpha = group.isEnabled() ? 1.0f : 0.5f;

    g.setColour (group.findColour (GroupComponent::outlineColourId)
                    .withMultipliedAlpha (alpha));

	{
		Path p;
		p.startNewSubPath (x + textX + textW, y);
		p.lineTo (x + w - cs, y);

		p.addArc (x + w - cs2, y, cs2, cs2, 0, float_Pi * 0.5f);
		p.lineTo (x + w, y + 2*cs);

		g.strokePath (p, PathStrokeType (1.5f));
	}

	{
		Path p;
		p.startNewSubPath (x + 2*cs, y + h);
		p.lineTo (x + cs, y + h);

		p.addArc (x, y + h - cs2, cs2, cs2, float_Pi, float_Pi * 1.5f);
		p.lineTo (x, y + cs);

		p.addArc (x, y, cs2, cs2, float_Pi * 1.5f, float_Pi * 2.0f);
		p.lineTo (x + textX, y);

		g.strokePath (p, PathStrokeType (1.5f));
	}
	
    g.setColour (group.findColour (GroupComponent::textColourId)
                    .withMultipliedAlpha (alpha));
    g.setFont (f);
    g.drawText (text,
                roundToInt (x + textX), 0,
                roundToInt (textW),
                roundToInt (textH),
                Justification::centred, true);
}

void PluginLookAndFeel::drawLinearSliderBackground (Graphics& g,
                                              int x, int y,
                                              int width, int height,
                                              float sliderPos,
                                              float minSliderPos,
                                              float maxSliderPos,
                                              const Slider::SliderStyle style,
                                              Slider& slider)
{
	// HACK: We need to get a pointer specifically to a MicronSlider if it is one, because isHorizontal is not a virtual function...
	MicronSlider* micronKnob = dynamic_cast<MicronSlider*>(&slider);
	if ( ! micronKnob )
	{	// not a MicronSlider, fall back to standard Slider drawing...
		stdLookAndFeel.drawLinearSliderBackground (g, x, y, width, height, sliderPos, minSliderPos, maxSliderPos, style, slider);
		return;
	}

    const float sliderRadius = (float) (getSliderThumbRadius (slider) - 2);

    const Colour trackColour (slider.findColour (Slider::trackColourId));
    const Colour gradCol1 (trackColour.overlaidWith (Colours::black.withAlpha (slider.isEnabled() ? 0.25f : 0.13f)));
    const Colour gradCol2 (trackColour.overlaidWith (Colour (0x14000000)));
    Path indent;

	// HACK: This is why we need the dynamic_cast above, because isHorizontal() is not virtual.
    if (micronKnob->isHorizontal())
    {
        const float iy = y + height * 0.5f - sliderRadius * 0.5f;
        const float ih = sliderRadius;

        g.setGradientFill (ColourGradient (gradCol1, 0.0f, iy,
                                           gradCol2, 0.0f, iy + ih, false));

        indent.addRoundedRectangle (x - sliderRadius * 0.5f, iy,
                                    width + sliderRadius, ih,
                                    5.0f);
        g.fillPath (indent);
    }
    else
    {
        const float ix = x + width * 0.5f - sliderRadius * 0.5f;
        const float iw = sliderRadius;

        g.setGradientFill (ColourGradient (gradCol1, ix, 0.0f,
                                           gradCol2, ix + iw, 0.0f, false));

        indent.addRoundedRectangle (ix, y - sliderRadius * 0.5f,
                                    iw, height + sliderRadius,
                                    5.0f);
        g.fillPath (indent);
    }

    g.setColour (Colour (0x4c000000));
    g.strokePath (indent, PathStrokeType (0.5f));
}

void PluginLookAndFeel::drawLinearSliderThumb (Graphics& g,
                                         int x, int y,
                                         int width, int height,
                                         float sliderPos,
                                         float minSliderPos,
                                         float maxSliderPos,
                                         const Slider::SliderStyle style,
                                         Slider& slider)
{
    if (style == Slider::LinearHorizontal || style == Slider::LinearVertical)
    {
        float kx, ky;

        if (style == Slider::LinearVertical)
        {
            kx = x + width * 0.5f;
            ky = sliderPos;
        }
        else
        {
            kx = sliderPos;
            ky = y + height * 0.5f;
        }

		g.setColour(Colours::white);
		g.drawImageAt(sliderThumb, kx - 0.5f*sliderThumb.getWidth(), ky - 0.5f*sliderThumb.getHeight());
    }
    else
    {	// fall back to standard look and feel for other cases.
		stdLookAndFeel.drawLinearSliderThumb(g, x, y, width, height, sliderPos, minSliderPos, maxSliderPos, style, slider);
    }
}

void PluginLookAndFeel::drawLabel (Graphics& g, Label& label)
{
	LcdLabel* lcdLabel = dynamic_cast<LcdLabel*>(&label);
	if ( ! lcdLabel )
	{	// we are not drawing an LcdLabel, fall back to standard drawing.
		stdLookAndFeel.drawLabel(g, label);
		return;
	}

//    g.fillAll (label.findColour (Label::backgroundColourId));
	g.drawImageWithin(lcdPanel, 0, 0, label.getWidth(), label.getHeight(), RectanglePlacement(RectanglePlacement::stretchToFit));

    if (! label.isBeingEdited())
    {
        const float alpha = label.isEnabled() ? 1.0f : 0.5f;
        const Font font (getLabelFont (label));

        g.setColour (label.findColour (Label::textColourId).withMultipliedAlpha (alpha));
        g.setFont (font);
        g.drawFittedText (label.getText(),
                          label.getHorizontalBorderSize(),
                          label.getVerticalBorderSize(),
                          label.getWidth() - 2 * label.getHorizontalBorderSize(),
                          label.getHeight() - 2 * label.getVerticalBorderSize(),
                          label.getJustificationType(),
                          jmax (1, (int) (label.getHeight() / font.getHeight())),
                          label.getMinimumHorizontalScale());

        g.setColour (label.findColour (Label::outlineColourId).withMultipliedAlpha (alpha));
    }
    else if (label.isEnabled())
    {
        g.setColour (label.findColour (Label::outlineColourId));
    }
}

void PluginLookAndFeel::fillTextEditorBackground (Graphics& g, int width, int height, TextEditor& textEditor)
{
//    g.fillAll (textEditor.findColour (TextEditor::backgroundColourId));
	g.drawImageWithin(lcdPanel, 0, 0, width, height, RectanglePlacement(RectanglePlacement::stretchToFit));
}

void PluginLookAndFeel::drawTextEditorOutline (Graphics& g, int width, int height, TextEditor& textEditor)
{
return;
    if (textEditor.isEnabled())
    {
        if (textEditor.hasKeyboardFocus (true) && ! textEditor.isReadOnly())
        {
            const int border = 2;

            g.setColour (textEditor.findColour (TextEditor::focusedOutlineColourId));
            g.drawRect (0, 0, width, height, border);

            g.setOpacity (1.0f);
            const Colour shadowColour (textEditor.findColour (TextEditor::shadowColourId).withMultipliedAlpha (0.75f));
            drawBevel (g, 0, 0, width, height + 2, border + 2, shadowColour, shadowColour);
        }
        else
        {
            g.setColour (textEditor.findColour (TextEditor::outlineColourId));
            g.drawRect (0, 0, width, height);

            g.setOpacity (1.0f);
            const Colour shadowColour (textEditor.findColour (TextEditor::shadowColourId));
            drawBevel (g, 0, 0, width, height + 2, 3, shadowColour, shadowColour);
        }

    }
}

InvertedButtonLookAndFeel::InvertedButtonLookAndFeel() {
	toggleButtonOff = ImageCache::getFromMemory (BinaryData::pbutton0_png,
                                                 BinaryData::pbutton0_pngSize);
	toggleButtonOn = ImageCache::getFromMemory (BinaryData::pbutton1_png,
                                                BinaryData::pbutton1_pngSize);
}

void InvertedButtonLookAndFeel::drawToggleButton (Graphics& g, ToggleButton& button, bool isMouseOverButton, bool isButtonDown)
{
	const Image& buttonImg = button.getToggleState() ? toggleButtonOff : toggleButtonOn;
    g.drawImageAt(buttonImg, 2.0f, 1.0f + (button.getHeight() - buttonImg.getHeight()) * 0.5f);
    
}
