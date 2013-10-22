/*
  ==============================================================================

    MicronSlider.cpp
    Created: 20 Sep 2013 6:48:12pm
    Author:  Jules

  ==============================================================================
*/

#include "../JuceLibraryCode/JuceHeader.h"
#include "MicronSlider.h"

class MicronSlider::Pimpl   : public AsyncUpdater,
                        public ButtonListener,  // (can't use Button::Listener due to idiotic VC2005 bug)
                        public LabelListener,
                        public ValueListener
{
public:
    Pimpl (MicronSlider& s, Slider::SliderStyle sliderStyle, Slider::TextEntryBoxPosition textBoxPosition)
      : owner (s),
        style (sliderStyle),
        lastCurrentValue (0), lastValueMin (0), lastValueMax (0),
        minimum (0), maximum (10), interval (0), doubleClickReturnValue (0),
        skewFactor (1.0), velocityModeSensitivity (1.0),
        velocityModeOffset (0.0), velocityModeThreshold (1),
        rotaryStart (float_Pi * 1.2f),
        rotaryEnd (float_Pi * 2.8f),
		mouseFineAdjust(false),
        sliderRegionStart (0), sliderRegionSize (1), sliderBeingDragged (-1),
        pixelsForFullDragExtent (250),
        textBoxPos (textBoxPosition),
        numDecimalPlaces (7),
        textBoxWidth (80), textBoxHeight (20),
        incDecButtonMode (Slider::incDecButtonsNotDraggable),
        editableText (true),
        doubleClickToValue (false),
        isVelocityBased (false),
        userKeyOverridesVelocity (true),
        rotaryStop (true),
        incDecButtonsSideBySide (false),
        sendChangeOnlyOnRelease (false),
        popupDisplayEnabled (false),
        menuEnabled (false),
        useDragEvents (false),
        scrollWheelEnabled (true),
        snapsToMousePos (true),
        parentForPopupDisplay (nullptr)
    {
		resetDoubleClickTimer();
    }

    ~Pimpl()
    {
        currentValue.removeListener (this);
        valueMin.removeListener (this);
        valueMax.removeListener (this);
        popupDisplay = nullptr;
    }

    //==============================================================================
    void registerListeners()
    {
        currentValue.addListener (this);
        valueMin.addListener (this);
        valueMax.addListener (this);
    }

    bool isHorizontal() const noexcept
    {
        return style == Slider::LinearHorizontal
            || style == Slider::LinearBar
            || style == Slider::TwoValueHorizontal
            || style == Slider::ThreeValueHorizontal;
    }

    bool isVertical() const noexcept
    {
        return style == Slider::LinearVertical
            || style == Slider::LinearBarVertical
            || style == Slider::TwoValueVertical
            || style == Slider::ThreeValueVertical;
    }

    bool isRotary() const noexcept
    {
        return style == Slider::Rotary
            || style == Slider::RotaryHorizontalDrag
            || style == Slider::RotaryVerticalDrag
            || style == Slider::RotaryHorizontalVerticalDrag;
    }

    bool incDecDragDirectionIsHorizontal() const noexcept
    {
        return incDecButtonMode == Slider::incDecButtonsDraggable_Horizontal
                || (incDecButtonMode == Slider::incDecButtonsDraggable_AutoDirection && incDecButtonsSideBySide);
    }

    float getPositionOfValue (const double value) const
    {
        if (isHorizontal() || isVertical())
            return getLinearSliderPos (value);

        jassertfalse; // not a valid call on a slider that doesn't work linearly!
        return 0.0f;
    }

    void setRange (const double newMin, const double newMax, const double newInt)
    {
        if (minimum != newMin || maximum != newMax || interval != newInt)
        {
            minimum = newMin;
            maximum = newMax;
            interval = newInt;

            // figure out the number of DPs needed to display all values at this
            // interval setting.
            numDecimalPlaces = 7;

            if (newInt != 0)
            {
                int v = abs ((int) (newInt * 10000000));

                while ((v % 10) == 0)
                {
                    --numDecimalPlaces;
                    v /= 10;
                }
            }

            // keep the current values inside the new range..
            if (style != Slider::TwoValueHorizontal && style != Slider::TwoValueVertical)
            {
                setValue (getValue(), dontSendNotification);
            }
            else
            {
                setMinValue (getMinValue(), dontSendNotification, false);
                setMaxValue (getMaxValue(), dontSendNotification, false);
            }

            updateText();
        }
    }

    double getValue() const
    {
        // for a two-value style slider, you should use the getMinValue() and getMaxValue()
        // methods to get the two values.
        jassert (style != Slider::TwoValueHorizontal && style != Slider::TwoValueVertical);

        return currentValue.getValue();
    }

    void setValue (double newValue, const NotificationType notification)
    {
        // for a two-value style slider, you should use the setMinValue() and setMaxValue()
        // methods to set the two values.
        jassert (style != Slider::TwoValueHorizontal && style != Slider::TwoValueVertical);

        newValue = constrainedValue (newValue);

        if (style == Slider::ThreeValueHorizontal || style == Slider::ThreeValueVertical)
        {
            jassert ((double) valueMin.getValue() <= (double) valueMax.getValue());

            newValue = jlimit ((double) valueMin.getValue(),
                               (double) valueMax.getValue(),
                               newValue);
        }

        if (newValue != lastCurrentValue)
        {
            if (valueBox != nullptr)
                valueBox->hideEditor (true);

            lastCurrentValue = newValue;

            // (need to do this comparison because the Value will use equalsWithSameType to compare
            // the new and old values, so will generate unwanted change events if the type changes)
            if (currentValue != newValue)
                currentValue = newValue;

            updateText();
            owner.repaint();

            if (popupDisplay != nullptr)
                popupDisplay->updatePosition (owner.getTextFromValue (newValue));

            triggerChangeMessage (notification);
        }
    }

    void setMinValue (double newValue, const NotificationType notification,
                      const bool allowNudgingOfOtherValues)
    {
        // The minimum value only applies to sliders that are in two- or three-value mode.
        jassert (style == Slider::TwoValueHorizontal || style == Slider::TwoValueVertical
                  || style == Slider::ThreeValueHorizontal || style == Slider::ThreeValueVertical);

        newValue = constrainedValue (newValue);

        if (style == Slider::TwoValueHorizontal || style == Slider::TwoValueVertical)
        {
            if (allowNudgingOfOtherValues && newValue > (double) valueMax.getValue())
                setMaxValue (newValue, notification, false);

            newValue = jmin ((double) valueMax.getValue(), newValue);
        }
        else
        {
            if (allowNudgingOfOtherValues && newValue > lastCurrentValue)
                setValue (newValue, notification);

            newValue = jmin (lastCurrentValue, newValue);
        }

        if (lastValueMin != newValue)
        {
            lastValueMin = newValue;
            valueMin = newValue;
            owner.repaint();

            if (popupDisplay != nullptr)
                popupDisplay->updatePosition (owner.getTextFromValue (newValue));

            triggerChangeMessage (notification);
        }
    }

    void setMaxValue (double newValue, const NotificationType notification,
                      const bool allowNudgingOfOtherValues)
    {
        // The maximum value only applies to sliders that are in two- or three-value mode.
        jassert (style == Slider::TwoValueHorizontal || style == Slider::TwoValueVertical
                  || style == Slider::ThreeValueHorizontal || style == Slider::ThreeValueVertical);

        newValue = constrainedValue (newValue);

        if (style == Slider::TwoValueHorizontal || style == Slider::TwoValueVertical)
        {
            if (allowNudgingOfOtherValues && newValue < (double) valueMin.getValue())
                setMinValue (newValue, notification, false);

            newValue = jmax ((double) valueMin.getValue(), newValue);
        }
        else
        {
            if (allowNudgingOfOtherValues && newValue < lastCurrentValue)
                setValue (newValue, notification);

            newValue = jmax (lastCurrentValue, newValue);
        }

        if (lastValueMax != newValue)
        {
            lastValueMax = newValue;
            valueMax = newValue;
            owner.repaint();

            if (popupDisplay != nullptr)
                popupDisplay->updatePosition (owner.getTextFromValue (valueMax.getValue()));

            triggerChangeMessage (notification);
        }
    }

    void setMinAndMaxValues (double newMinValue, double newMaxValue, const NotificationType notification)
    {
        // The maximum value only applies to sliders that are in two- or three-value mode.
        jassert (style == Slider::TwoValueHorizontal || style == Slider::TwoValueVertical
                  || style == Slider::ThreeValueHorizontal || style == Slider::ThreeValueVertical);

        if (newMaxValue < newMinValue)
            std::swap (newMaxValue, newMinValue);

        newMinValue = constrainedValue (newMinValue);
        newMaxValue = constrainedValue (newMaxValue);

        if (lastValueMax != newMaxValue || lastValueMin != newMinValue)
        {
            lastValueMax = newMaxValue;
            lastValueMin = newMinValue;
            valueMin = newMinValue;
            valueMax = newMaxValue;
            owner.repaint();

            triggerChangeMessage (notification);
        }
    }

    double getMinValue() const
    {
        // The minimum value only applies to sliders that are in two- or three-value mode.
        jassert (style == Slider::TwoValueHorizontal || style == Slider::TwoValueVertical
                  || style == Slider::ThreeValueHorizontal || style == Slider::ThreeValueVertical);

        return valueMin.getValue();
    }

    double getMaxValue() const
    {
        // The maximum value only applies to sliders that are in two- or three-value mode.
        jassert (style == Slider::TwoValueHorizontal || style == Slider::TwoValueVertical
                  || style == Slider::ThreeValueHorizontal || style == Slider::ThreeValueVertical);

        return valueMax.getValue();
    }

    void triggerChangeMessage (const NotificationType notification)
    {
        if (notification != dontSendNotification)
        {
            if (notification == sendNotificationSync)
                handleAsyncUpdate();
            else
                triggerAsyncUpdate();

            owner.valueChanged();
        }
    }

    void handleAsyncUpdate() override
    {
        cancelPendingUpdate();

        Component::BailOutChecker checker (&owner);
        MicronSlider* slider = &owner; // (must use an intermediate variable here to avoid a VS2005 compiler bug)
        listeners.callChecked (checker, &MicronSliderListener::sliderValueChanged, slider);  // (can't use Slider::Listener due to idiotic VC2005 bug)
    }

    void sendDragStart()
    {
        owner.startedDragging();

        Component::BailOutChecker checker (&owner);
        MicronSlider* slider = &owner; // (must use an intermediate variable here to avoid a VS2005 compiler bug)
        listeners.callChecked (checker, &MicronSliderListener::sliderDragStarted, slider);
    }

    void sendDragEnd()
    {
        owner.stoppedDragging();

        sliderBeingDragged = -1;

        Component::BailOutChecker checker (&owner);
        MicronSlider* slider = &owner; // (must use an intermediate variable here to avoid a VS2005 compiler bug)
        listeners.callChecked (checker, &MicronSliderListener::sliderDragEnded, slider);
    }

    struct DragInProgress
    {
        DragInProgress (Pimpl& p)  : owner (p)      { owner.sendDragStart(); }
        ~DragInProgress()                           { owner.sendDragEnd(); }

        Pimpl& owner;

        JUCE_DECLARE_NON_COPYABLE (DragInProgress)
    };

    void buttonClicked (Button* button) override
    {
        if (style == Slider::IncDecButtons)
        {
            const double delta = (button == incButton) ? interval : -interval;

            DragInProgress drag (*this);
            setValue (owner.snapValue (getValue() + delta, false), sendNotificationSync);
        }
    }

    void valueChanged (Value& value) override
    {
        if (value.refersToSameSourceAs (currentValue))
        {
            if (style != Slider::TwoValueHorizontal && style != Slider::TwoValueVertical)
                setValue (currentValue.getValue(), dontSendNotification);
        }
        else if (value.refersToSameSourceAs (valueMin))
            setMinValue (valueMin.getValue(), dontSendNotification, true);
        else if (value.refersToSameSourceAs (valueMax))
            setMaxValue (valueMax.getValue(), dontSendNotification, true);
    }

    void labelTextChanged (Label* label) override
    {
        const double newValue = owner.snapValue (owner.getValueFromText (label->getText()), false);

        if (newValue != (double) currentValue.getValue())
        {
            DragInProgress drag (*this);
            setValue (newValue, sendNotificationSync);
        }

        updateText(); // force a clean-up of the text, needed in case setValue() hasn't done this.
    }

    void updateText()
    {
        if (valueBox != nullptr)
            valueBox->setText (owner.getTextFromValue (currentValue.getValue()), dontSendNotification);
    }

    double constrainedValue (double value) const
    {
        if (interval > 0)
            value = minimum + interval * std::floor ((value - minimum) / interval + 0.5);

        if (value <= minimum || maximum <= minimum)
            value = minimum;
        else if (value >= maximum)
            value = maximum;

        return value;
    }

    float getLinearSliderPos (const double value) const
    {
        double pos;

        if (maximum > minimum)
        {
            if (value < minimum)
            {
                pos = 0.0;
            }
            else if (value > maximum)
            {
                pos = 1.0;
            }
            else
            {
                pos = owner.valueToProportionOfLength (value);
                jassert (pos >= 0 && pos <= 1.0);
            }
        }
        else
        {
            pos = 0.5;
        }

        if (isVertical() || style == Slider::IncDecButtons)
            pos = 1.0 - pos;

        return (float) (sliderRegionStart + pos * sliderRegionSize);
    }

    void setSliderStyle (const Slider::SliderStyle newStyle)
    {
        if (style != newStyle)
        {
            style = newStyle;
            owner.repaint();
            owner.lookAndFeelChanged();
        }
    }

    void setRotaryParameters (const float startAngleRadians,
                              const float endAngleRadians,
                              const bool stopAtEnd)
    {
        // make sure the values are sensible..
        jassert (rotaryStart >= 0 && rotaryEnd >= 0);
        jassert (rotaryStart < float_Pi * 4.0f && rotaryEnd < float_Pi * 4.0f);
        jassert (rotaryStart < rotaryEnd);

        rotaryStart = startAngleRadians;
        rotaryEnd = endAngleRadians;
        rotaryStop = stopAtEnd;
    }

    void setVelocityModeParameters (const double sensitivity, const int threshold,
                                    const double offset, const bool userCanPressKeyToSwapMode)
    {
        velocityModeSensitivity = sensitivity;
        velocityModeOffset = offset;
        velocityModeThreshold = threshold;
        userKeyOverridesVelocity = userCanPressKeyToSwapMode;
    }

    void setSkewFactorFromMidPoint (const double sliderValueToShowAtMidPoint)
    {
        if (maximum > minimum)
            skewFactor = log (0.5) / log ((sliderValueToShowAtMidPoint - minimum)
                                            / (maximum - minimum));
    }

    void setIncDecButtonsMode (const Slider::IncDecButtonMode mode)
    {
        if (incDecButtonMode != mode)
        {
            incDecButtonMode = mode;
            owner.lookAndFeelChanged();
        }
    }

    void setTextBoxStyle (const Slider::TextEntryBoxPosition newPosition,
                          const bool isReadOnly,
                          const int textEntryBoxWidth,
                          const int textEntryBoxHeight)
    {
        if (textBoxPos != newPosition
             || editableText != (! isReadOnly)
             || textBoxWidth != textEntryBoxWidth
             || textBoxHeight != textEntryBoxHeight)
        {
            textBoxPos = newPosition;
            editableText = ! isReadOnly;
            textBoxWidth = textEntryBoxWidth;
            textBoxHeight = textEntryBoxHeight;

            owner.repaint();
            owner.lookAndFeelChanged();
        }
    }

    void setTextBoxIsEditable (const bool shouldBeEditable)
    {
        editableText = shouldBeEditable;

        if (valueBox != nullptr)
            valueBox->setEditable (shouldBeEditable && owner.isEnabled());
    }

    void showTextBox()
    {
        jassert (editableText); // this should probably be avoided in read-only sliders.

        if (valueBox != nullptr)
            valueBox->showEditor();
    }

    void hideTextBox (const bool discardCurrentEditorContents)
    {
        if (valueBox != nullptr)
        {
            valueBox->hideEditor (discardCurrentEditorContents);

            if (discardCurrentEditorContents)
                updateText();
        }
    }

    void setTextValueSuffix (const String& suffix)
    {
        if (textSuffix != suffix)
        {
            textSuffix = suffix;
            updateText();
        }
    }

    void lookAndFeelChanged (LookAndFeel& lf)
    {
        if (textBoxPos != Slider::NoTextBox)
        {
            const String previousTextBoxContent (valueBox != nullptr ? valueBox->getText()
                                                                     : owner.getTextFromValue (currentValue.getValue()));

            valueBox = nullptr;
            owner.addAndMakeVisible (valueBox = lf.createSliderTextBox (owner));

            valueBox->setWantsKeyboardFocus (false);
            valueBox->setText (previousTextBoxContent, dontSendNotification);

            if (valueBox->isEditable() != editableText) // (avoid overriding the single/double click flags unless we have to)
                valueBox->setEditable (editableText && owner.isEnabled());

            valueBox->addListener (this);

            if (style == Slider::LinearBar || style == Slider::LinearBarVertical)
            {
                valueBox->addMouseListener (&owner, false);
                valueBox->setMouseCursor (MouseCursor::ParentCursor);
            }
            else
            {
                valueBox->setTooltip (owner.getTooltip());
            }
        }
        else
        {
            valueBox = nullptr;
        }

        if (style == Slider::IncDecButtons)
        {
            owner.addAndMakeVisible (incButton = lf.createSliderButton (true));
            incButton->addListener (this);

            owner.addAndMakeVisible (decButton = lf.createSliderButton (false));
            decButton->addListener (this);

            if (incDecButtonMode != Slider::incDecButtonsNotDraggable)
            {
                incButton->addMouseListener (&owner, false);
                decButton->addMouseListener (&owner, false);
            }
            else
            {
                incButton->setRepeatSpeed (300, 100, 20);
                incButton->addMouseListener (decButton, false);

                decButton->setRepeatSpeed (300, 100, 20);
                decButton->addMouseListener (incButton, false);
            }

            const String tooltip (owner.getTooltip());
            incButton->setTooltip (tooltip);
            decButton->setTooltip (tooltip);
        }
        else
        {
            incButton = nullptr;
            decButton = nullptr;
        }

        owner.setComponentEffect (lf.getSliderEffect());

        owner.resized();
        owner.repaint();
    }

    void showPopupMenu()
    {
        PopupMenu m;
        m.setLookAndFeel (&owner.getLookAndFeel());
        m.addItem (1, TRANS ("Velocity-sensitive mode"), true, isVelocityBased);
        m.addSeparator();

        if (isRotary())
        {
            PopupMenu rotaryMenu;
            rotaryMenu.addItem (2, TRANS ("Use circular dragging"),           true, style == Slider::Rotary);
            rotaryMenu.addItem (3, TRANS ("Use left-right dragging"),         true, style == Slider::RotaryHorizontalDrag);
            rotaryMenu.addItem (4, TRANS ("Use up-down dragging"),            true, style == Slider::RotaryVerticalDrag);
            rotaryMenu.addItem (5, TRANS ("Use left-right/up-down dragging"), true, style == Slider::RotaryHorizontalVerticalDrag);

            m.addSubMenu (TRANS ("Rotary mode"), rotaryMenu);
        }

        m.showMenuAsync (PopupMenu::Options(),
                         ModalCallbackFunction::forComponent (sliderMenuCallback, &owner));
    }

    static void sliderMenuCallback (const int result, MicronSlider* slider)
    {
        if (slider != nullptr)
        {
            switch (result)
            {
                case 1:   slider->setVelocityBasedMode (! slider->getVelocityBasedMode()); break;
                case 2:   slider->setSliderStyle (Slider::Rotary); break;
                case 3:   slider->setSliderStyle (Slider::RotaryHorizontalDrag); break;
                case 4:   slider->setSliderStyle (Slider::RotaryVerticalDrag); break;
                case 5:   slider->setSliderStyle (Slider::RotaryHorizontalVerticalDrag); break;
                default:  break;
            }
        }
    }

    int getThumbIndexAt (const MouseEvent& e)
    {
        const bool isTwoValue   = (style == Slider::TwoValueHorizontal   || style == Slider::TwoValueVertical);
        const bool isThreeValue = (style == Slider::ThreeValueHorizontal || style == Slider::ThreeValueVertical);

        if (isTwoValue || isThreeValue)
        {
            const float mousePos = (float) (isVertical() ? e.y : e.x);

            const float normalPosDistance = std::abs (getLinearSliderPos (currentValue.getValue()) - mousePos);
            const float minPosDistance    = std::abs (getLinearSliderPos (valueMin.getValue()) - 0.1f - mousePos);
            const float maxPosDistance    = std::abs (getLinearSliderPos (valueMax.getValue()) + 0.1f - mousePos);

            if (isTwoValue)
                return maxPosDistance <= minPosDistance ? 2 : 1;

            if (normalPosDistance >= minPosDistance && maxPosDistance >= minPosDistance)
                return 1;

            if (normalPosDistance >= maxPosDistance)
                return 2;
        }

        return 0;
    }

    //==============================================================================
    void handleRotaryDrag (const MouseEvent& e)
    {
        const int dx = e.x - sliderRect.getCentreX();
        const int dy = e.y - sliderRect.getCentreY();

        if (dx * dx + dy * dy > 25)
        {
            double angle = std::atan2 ((double) dx, (double) -dy);
            while (angle < 0.0)
                angle += double_Pi * 2.0;

            if (rotaryStop && ! e.mouseWasClicked())
            {
                if (std::abs (angle - lastAngle) > double_Pi)
                {
                    if (angle >= lastAngle)
                        angle -= double_Pi * 2.0;
                    else
                        angle += double_Pi * 2.0;
                }

                if (angle >= lastAngle)
                    angle = jmin (angle, (double) jmax (rotaryStart, rotaryEnd));
                else
                    angle = jmax (angle, (double) jmin (rotaryStart, rotaryEnd));
            }
            else
            {
                while (angle < rotaryStart)
                    angle += double_Pi * 2.0;

                if (angle > rotaryEnd)
                {
                    if (smallestAngleBetween (angle, rotaryStart)
                         <= smallestAngleBetween (angle, rotaryEnd))
                        angle = rotaryStart;
                    else
                        angle = rotaryEnd;
                }
            }

            const double proportion = (angle - rotaryStart) / (rotaryEnd - rotaryStart);
            valueWhenLastDragged = owner.proportionOfLengthToValue (jlimit (0.0, 1.0, proportion));
            lastAngle = angle;
        }
    }

    void handleAbsoluteDrag (const MouseEvent& e)
    {
        const int mousePos = (isHorizontal() || style == Slider::RotaryHorizontalDrag) ? e.x : e.y;
        double newPos = (mousePos - sliderRegionStart) / (double) sliderRegionSize;

        if (style == Slider::RotaryHorizontalDrag
            || style == Slider::RotaryVerticalDrag
            || style == Slider::IncDecButtons
            || ((style == Slider::LinearHorizontal || style == Slider::LinearVertical || style == Slider::LinearBar || style == Slider::LinearBarVertical)
                && ! snapsToMousePos))
        {
            const int mouseDiff = (style == Slider::RotaryHorizontalDrag
                                     || style == Slider::LinearHorizontal
                                     || style == Slider::LinearBar
                                     || (style == Slider::IncDecButtons && incDecDragDirectionIsHorizontal()))
                                    ? e.x - mouseDragStartPos.x
                                    : mouseDragStartPos.y - e.y;

            newPos = owner.valueToProportionOfLength (valueOnMouseDown)
                       + mouseDiff * (1.0 / pixelsForFullDragExtent);

            if (style == Slider::IncDecButtons)
            {
                incButton->setState (mouseDiff < 0 ? Button::buttonNormal : Button::buttonDown);
                decButton->setState (mouseDiff > 0 ? Button::buttonNormal : Button::buttonDown);
            }
        }
        else if (style == Slider::RotaryHorizontalVerticalDrag)
        {
			const double dragAmount = (e.x - mouseDragStartPos.x) + (mouseDragStartPos.y - e.y);
            const double mouseDiff = dragAmount / (mouseFineAdjust ? 30.0 : 1.0);

            newPos = owner.valueToProportionOfLength (valueOnMouseDown) + mouseDiff * (1.0 / pixelsForFullDragExtent);
        }
        else
        {
            if (isVertical())
			{
				if (mouseFineAdjust)
				{
					const double dragAmount = mouseDragStartPos.y - e.y;
					const double mouseDiff = dragAmount / (mouseFineAdjust ? 30.0 : 1.0);

					newPos = owner.valueToProportionOfLength (valueOnMouseDown) + mouseDiff * (1.0 / pixelsForFullDragExtent);
				}
				else
				{
					newPos = 1.0 - newPos;
				}
			}
			else
			{	// TODO: horizontal case is untested:
				if (mouseFineAdjust)
				{
					const double dragAmount = e.x - mouseDragStartPos.x;
					const double mouseDiff = dragAmount / (mouseFineAdjust ? 30.0 : 1.0);

					newPos = owner.valueToProportionOfLength (valueOnMouseDown) + mouseDiff * (1.0 / pixelsForFullDragExtent);
				}
			}
        }

        valueWhenLastDragged = owner.proportionOfLengthToValue (jlimit (0.0, 1.0, newPos));
    }

    void handleVelocityDrag (const MouseEvent& e)
    {
        const int mouseDiff = style == Slider::RotaryHorizontalVerticalDrag
                                ? (e.x - mousePosWhenLastDragged.x) + (mousePosWhenLastDragged.y - e.y)
                                : (isHorizontal()
                                    || style == Slider::RotaryHorizontalDrag
                                    || (style == Slider::IncDecButtons && incDecDragDirectionIsHorizontal()))
                                      ? e.x - mousePosWhenLastDragged.x
                                      : e.y - mousePosWhenLastDragged.y;

        const double maxSpeed = jmax (200, sliderRegionSize);
        double speed = jlimit (0.0, maxSpeed, (double) abs (mouseDiff));

        if (speed != 0)
        {
            speed = 0.2 * velocityModeSensitivity
                      * (1.0 + std::sin (double_Pi * (1.5 + jmin (0.5, velocityModeOffset
                                                                    + jmax (0.0, (double) (speed - velocityModeThreshold))
                                                                        / maxSpeed))));

            if (mouseDiff < 0)
                speed = -speed;

            if (isVertical() || style == Slider::RotaryVerticalDrag
                 || (style == Slider::IncDecButtons && ! incDecDragDirectionIsHorizontal()))
                speed = -speed;

            const double currentPos = owner.valueToProportionOfLength (valueWhenLastDragged);

            valueWhenLastDragged = owner.proportionOfLengthToValue (jlimit (0.0, 1.0, currentPos + speed));

            e.source.enableUnboundedMouseMovement (true, false);
            mouseWasHidden = true;
        }
    }

    void mouseDown (const MouseEvent& e)
    {
        mouseWasHidden = false;
        incDecDragged = false;
        useDragEvents = false;
        mouseDragStartPos = mousePosWhenLastDragged = e.getPosition();
        currentDrag = nullptr;
		mouseFineAdjust = e.mods.isRightButtonDown();

        if (owner.isEnabled())
        {
			if (e.eventTime - prevMouseDownTime < RelativeTime(MicronSlider::doubleClickDuration))
			{
				mouseDoubleClick();
			}
			else
			{
				prevMouseDownTime = e.eventTime;

				if (e.mods.isPopupMenu() && menuEnabled)
				{
					showPopupMenu();
				}
				else if (canDoubleClickToValue() && e.mods.isAltDown())
				{
					mouseDoubleClick();
				}
				else if (maximum > minimum)
				{
					useDragEvents = true;

					if (valueBox != nullptr)
						valueBox->hideEditor (true);

					sliderBeingDragged = getThumbIndexAt (e);

					minMaxDiff = (double) valueMax.getValue() - (double) valueMin.getValue();

					lastAngle = rotaryStart + (rotaryEnd - rotaryStart)
												* owner.valueToProportionOfLength (currentValue.getValue());

					valueWhenLastDragged = (sliderBeingDragged == 2 ? valueMax
																	: (sliderBeingDragged == 1 ? valueMin
																							   : currentValue)).getValue();
					valueOnMouseDown = valueWhenLastDragged;

					if (popupDisplayEnabled)
					{
						PopupDisplayComponent* const popup = new PopupDisplayComponent (owner);
						popupDisplay = popup;

						if (parentForPopupDisplay != nullptr)
							parentForPopupDisplay->addChildComponent (popup);
						else
							popup->addToDesktop (0);

						popup->setVisible (true);
					}

					currentDrag = new DragInProgress (*this);
					mouseDrag (e);
				}
			}
        }
    }

    void mouseDrag (const MouseEvent& e)
    {
		if (e.getDistanceFromDragStart() > 3)
			resetDoubleClickTimer();

        if (useDragEvents
             && maximum > minimum
             && ! ((style == Slider::LinearBar || style == Slider::LinearBarVertical) && e.mouseWasClicked() && valueBox != nullptr && valueBox->isEditable()))
        {
            if (style == Slider::Rotary)
            {
                handleRotaryDrag (e);
            }
            else
            {
                if (style == Slider::IncDecButtons && ! incDecDragged)
                {
                    if (e.getDistanceFromDragStart() < 10 || e.mouseWasClicked())
                        return;

                    incDecDragged = true;
                    mouseDragStartPos = e.getPosition();
                }

                if (isVelocityBased == (userKeyOverridesVelocity && e.mods.testFlags (ModifierKeys::ctrlModifier
                                                                                        | ModifierKeys::commandModifier
                                                                                        | ModifierKeys::altModifier))
                     || (maximum - minimum) / sliderRegionSize < interval)
                    handleAbsoluteDrag (e);
                else
                    handleVelocityDrag (e);
            }

            valueWhenLastDragged = jlimit (minimum, maximum, valueWhenLastDragged);

            if (sliderBeingDragged == 0)
            {
                setValue (owner.snapValue (valueWhenLastDragged, true),
                          sendChangeOnlyOnRelease ? dontSendNotification : sendNotificationSync);
            }
            else if (sliderBeingDragged == 1)
            {
                setMinValue (owner.snapValue (valueWhenLastDragged, true),
                             sendChangeOnlyOnRelease ? dontSendNotification : sendNotificationAsync, true);

                if (e.mods.isShiftDown())
                    setMaxValue (getMinValue() + minMaxDiff, dontSendNotification, true);
                else
                    minMaxDiff = (double) valueMax.getValue() - (double) valueMin.getValue();
            }
            else if (sliderBeingDragged == 2)
            {
                setMaxValue (owner.snapValue (valueWhenLastDragged, true),
                             sendChangeOnlyOnRelease ? dontSendNotification : sendNotificationAsync, true);

                if (e.mods.isShiftDown())
                    setMinValue (getMaxValue() - minMaxDiff, dontSendNotification, true);
                else
                    minMaxDiff = (double) valueMax.getValue() - (double) valueMin.getValue();
            }

            mousePosWhenLastDragged = e.getPosition();
        }
    }

    void mouseUp()
    {
        if (owner.isEnabled()
             && useDragEvents
             && (maximum > minimum)
             && (style != Slider::IncDecButtons || incDecDragged))
        {
            restoreMouseIfHidden();

            if (sendChangeOnlyOnRelease && valueOnMouseDown != (double) currentValue.getValue())
                triggerChangeMessage (sendNotificationAsync);

            currentDrag = nullptr;
            popupDisplay = nullptr;

            if (style == Slider::IncDecButtons)
            {
                incButton->setState (Button::buttonNormal);
                decButton->setState (Button::buttonNormal);
            }
        }
        else if (popupDisplay != nullptr)
        {
            popupDisplay->startTimer (2000);
        }

        currentDrag = nullptr;
    }

    bool canDoubleClickToValue() const
    {
        return doubleClickToValue
                && style != Slider::IncDecButtons
                && minimum <= doubleClickReturnValue
                && maximum >= doubleClickReturnValue;
    }

    void mouseDoubleClick()
    {
        if (canDoubleClickToValue())
        {
            DragInProgress drag (*this);
            setValue (doubleClickReturnValue, sendNotificationSync);
        }
		resetDoubleClickTimer();
    }

	void resetDoubleClickTimer()
	{	// ensure next click does not look like another double-click
		prevMouseDownTime = Time::getCurrentTime() - RelativeTime(1.0);
	}

    bool mouseWheelMove (const MouseEvent& e, const MouseWheelDetails& wheel)
    {
        if (scrollWheelEnabled
             && style != Slider::TwoValueHorizontal
             && style != Slider::TwoValueVertical)
        {
            if (maximum > minimum && ! e.mods.isAnyMouseButtonDown())
            {
                if (valueBox != nullptr)
                    valueBox->hideEditor (false);

                const double value = (double) currentValue.getValue();
                const double proportionDelta = (wheel.deltaX != 0 ? -wheel.deltaX : wheel.deltaY)
                                                   * (wheel.isReversed ? -0.15f : 0.15f);
                const double currentPos = owner.valueToProportionOfLength (value);
                const double newValue = owner.proportionOfLengthToValue (jlimit (0.0, 1.0, currentPos + proportionDelta));

                double delta = (newValue != value) ? jmax (std::abs (newValue - value), interval) : 0;
                if (value > newValue)
                    delta = -delta;

                DragInProgress drag (*this);
                setValue (owner.snapValue (value + delta, false), sendNotificationSync);
            }

            return true;
        }

        return false;
    }

    void modifierKeysChanged (const ModifierKeys& modifiers)
    {
        if (style != Slider::IncDecButtons
             && style != Slider::Rotary
             && isVelocityBased == modifiers.isAnyModifierKeyDown())
        {
            restoreMouseIfHidden();
        }
    }

    void restoreMouseIfHidden()
    {
        if (mouseWasHidden)
        {
            mouseWasHidden = false;

            const Array<MouseInputSource>& mouseSources = Desktop::getInstance().getMouseSources();

            for (MouseInputSource* mi = mouseSources.begin(), * const e = mouseSources.end(); mi != e; ++mi)
                mi->enableUnboundedMouseMovement (false);

            const double pos = sliderBeingDragged == 2 ? getMaxValue()
                                                       : (sliderBeingDragged == 1 ? getMinValue()
                                                                                  : (double) currentValue.getValue());
            Point<int> mousePos;

            if (isRotary())
            {
                mousePos = Desktop::getLastMouseDownPosition();

                const int delta = roundToInt (pixelsForFullDragExtent * (owner.valueToProportionOfLength (valueOnMouseDown)
                                                                           - owner.valueToProportionOfLength (pos)));

                if (style == Slider::RotaryHorizontalDrag)      mousePos += Point<int> (-delta, 0);
                else if (style == Slider::RotaryVerticalDrag)   mousePos += Point<int> (0, delta);
                else                                    mousePos += Point<int> (delta / -2, delta / 2);
            }
            else
            {
                const int pixelPos = (int) getLinearSliderPos (pos);

                mousePos = owner.localPointToGlobal (Point<int> (isHorizontal() ? pixelPos : (owner.getWidth() / 2),
                                                                 isVertical()   ? pixelPos : (owner.getHeight() / 2)));
            }

            Desktop::setMousePosition (mousePos);
        }
    }

    //==============================================================================
    void paint (Graphics& g, LookAndFeel& lf)
    {
        if (style != Slider::IncDecButtons)
        {
            if (isRotary())
            {
                const float sliderPos = (float) owner.valueToProportionOfLength (lastCurrentValue);
                jassert (sliderPos >= 0 && sliderPos <= 1.0f);

                lf.drawRotarySlider (g,
                                     sliderRect.getX(), sliderRect.getY(),
                                     sliderRect.getWidth(), sliderRect.getHeight(),
                                     sliderPos, rotaryStart, rotaryEnd, owner);
            }
            else
            {
                lf.drawLinearSlider (g,
                                     sliderRect.getX(), sliderRect.getY(),
                                     sliderRect.getWidth(), sliderRect.getHeight(),
                                     getLinearSliderPos (lastCurrentValue),
                                     getLinearSliderPos (lastValueMin),
                                     getLinearSliderPos (lastValueMax),
                                     owner.getSliderStyle(), owner);
            }

            if ((style == Slider::LinearBar || style == Slider::LinearBarVertical) && valueBox == nullptr)
            {
                g.setColour (owner.findColour (MicronSlider::textBoxOutlineColourId));
                g.drawRect (0, 0, owner.getWidth(), owner.getHeight(), 1);
            }
        }
    }

    void resized (const Rectangle<int>& localBounds, LookAndFeel& lf)
    {
        int minXSpace = 0;
        int minYSpace = 0;

        if (textBoxPos == Slider::TextBoxLeft || textBoxPos == Slider::TextBoxRight)
            minXSpace = 30;
        else
            minYSpace = 15;

        const int tbw = jmax (0, jmin (textBoxWidth,  localBounds.getWidth() - minXSpace));
        const int tbh = jmax (0, jmin (textBoxHeight, localBounds.getHeight() - minYSpace));

        if (style == Slider::LinearBar || style == Slider::LinearBarVertical)
        {
            if (valueBox != nullptr)
                valueBox->setBounds (localBounds);
        }
        else
        {
            if (textBoxPos == Slider::NoTextBox)
            {
                sliderRect = localBounds;
            }
            else if (textBoxPos == Slider::TextBoxLeft)
            {
                valueBox->setBounds (0, (localBounds.getHeight() - tbh) / 2, tbw, tbh);
                sliderRect.setBounds (tbw, 0, localBounds.getWidth() - tbw, localBounds.getHeight());
            }
            else if (textBoxPos == Slider::TextBoxRight)
            {
                valueBox->setBounds (localBounds.getWidth() - tbw, (localBounds.getHeight() - tbh) / 2, tbw, tbh);
                sliderRect.setBounds (0, 0, localBounds.getWidth() - tbw, localBounds.getHeight());
            }
            else if (textBoxPos == Slider::TextBoxAbove)
            {
                valueBox->setBounds ((localBounds.getWidth() - tbw) / 2, 0, tbw, tbh);
                sliderRect.setBounds (0, tbh, localBounds.getWidth(), localBounds.getHeight() - tbh);
            }
            else if (textBoxPos == Slider::TextBoxBelow)
            {
                valueBox->setBounds ((localBounds.getWidth() - tbw) / 2, localBounds.getHeight() - tbh, tbw, tbh);
                sliderRect.setBounds (0, 0, localBounds.getWidth(), localBounds.getHeight() - tbh);
            }
        }

        const int indent = lf.getSliderThumbRadius (owner);

        if (style == Slider::LinearBar)
        {
            const int barIndent = 1;
            sliderRegionStart = barIndent;
            sliderRegionSize = localBounds.getWidth() - barIndent * 2;

            sliderRect.setBounds (sliderRegionStart, barIndent,
                                  sliderRegionSize, localBounds.getHeight() - barIndent * 2);
        }
        else if (style == Slider::LinearBarVertical)
        {
            const int barIndent = 1;
            sliderRegionStart = barIndent;
            sliderRegionSize = localBounds.getHeight() - barIndent * 2;

            sliderRect.setBounds (barIndent, sliderRegionStart,
                                  localBounds.getWidth() - barIndent * 2, sliderRegionSize);
        }
        else if (isHorizontal())
        {
            sliderRegionStart = sliderRect.getX() + indent;
            sliderRegionSize = jmax (1, sliderRect.getWidth() - indent * 2);

            sliderRect.setBounds (sliderRegionStart, sliderRect.getY(),
                                  sliderRegionSize, sliderRect.getHeight());
        }
        else if (isVertical())
        {
            sliderRegionStart = sliderRect.getY() + indent;
            sliderRegionSize = jmax (1, sliderRect.getHeight() - indent * 2);

            sliderRect.setBounds (sliderRect.getX(), sliderRegionStart,
                                  sliderRect.getWidth(), sliderRegionSize);
        }
        else
        {
            sliderRegionStart = 0;
            sliderRegionSize = 100;
        }

        if (style == Slider::IncDecButtons)
            resizeIncDecButtons();
    }

    void resizeIncDecButtons()
    {
        Rectangle<int> buttonRect (sliderRect);

        if (textBoxPos == Slider::TextBoxLeft || textBoxPos == Slider::TextBoxRight)
            buttonRect.expand (-2, 0);
        else
            buttonRect.expand (0, -2);

        incDecButtonsSideBySide = buttonRect.getWidth() > buttonRect.getHeight();

        if (incDecButtonsSideBySide)
        {
            decButton->setBounds (buttonRect.removeFromLeft (buttonRect.getWidth() / 2));
            decButton->setConnectedEdges (Button::ConnectedOnRight);
            incButton->setConnectedEdges (Button::ConnectedOnLeft);
        }
        else
        {
            decButton->setBounds (buttonRect.removeFromBottom (buttonRect.getHeight() / 2));
            decButton->setConnectedEdges (Button::ConnectedOnTop);
            incButton->setConnectedEdges (Button::ConnectedOnBottom);
        }

        incButton->setBounds (buttonRect);
    }

    //==============================================================================
    MicronSlider& owner;
    Slider::SliderStyle style;

    ListenerList <MicronSliderListener> listeners;
    Value currentValue, valueMin, valueMax;
    double lastCurrentValue, lastValueMin, lastValueMax;
    double minimum, maximum, interval, doubleClickReturnValue;
    double valueWhenLastDragged, valueOnMouseDown, skewFactor, lastAngle;
    double velocityModeSensitivity, velocityModeOffset, minMaxDiff;
    int velocityModeThreshold;
    float rotaryStart, rotaryEnd;
    Point<int> mouseDragStartPos, mousePosWhenLastDragged;
	bool mouseFineAdjust;
	Time prevMouseDownTime; // for double-click detection
    int sliderRegionStart, sliderRegionSize;
    int sliderBeingDragged;
    int pixelsForFullDragExtent;
    Rectangle<int> sliderRect;
    ScopedPointer<DragInProgress> currentDrag;

    Slider::TextEntryBoxPosition textBoxPos;
    String textSuffix;
    int numDecimalPlaces;
    int textBoxWidth, textBoxHeight;
    Slider::IncDecButtonMode incDecButtonMode;

    bool editableText;
    bool doubleClickToValue;
    bool isVelocityBased;
    bool userKeyOverridesVelocity;
    bool rotaryStop;
    bool incDecButtonsSideBySide;
    bool sendChangeOnlyOnRelease;
    bool popupDisplayEnabled;
    bool menuEnabled;
    bool useDragEvents;
    bool mouseWasHidden;
    bool incDecDragged;
    bool scrollWheelEnabled;
    bool snapsToMousePos;

    ScopedPointer<Label> valueBox;
    ScopedPointer<Button> incButton, decButton;

    //==============================================================================
    class PopupDisplayComponent  : public BubbleComponent,
                                   public Timer
    {
    public:
        PopupDisplayComponent (MicronSlider& s)
            : owner (s),
              font (s.getLookAndFeel().getSliderPopupFont())
        {
            setAlwaysOnTop (true);
            setAllowedPlacement (owner.getLookAndFeel().getSliderPopupPlacement());
        }

        void paintContent (Graphics& g, int w, int h)
        {
            g.setFont (font);
            g.setColour (findColour (TooltipWindow::textColourId, true));
            g.drawFittedText (text, Rectangle<int> (w, h), Justification::centred, 1);
        }

        void getContentSize (int& w, int& h)
        {
            w = font.getStringWidth (text) + 18;
            h = (int) (font.getHeight() * 1.6f);
        }

        void updatePosition (const String& newText)
        {
            text = newText;
            BubbleComponent::setPosition (&owner);
            repaint();
        }

        void timerCallback() override
        {
            owner.pimpl->popupDisplay = nullptr;
        }

    private:
        MicronSlider& owner;
        Font font;
        String text;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PopupDisplayComponent)
    };

    ScopedPointer <PopupDisplayComponent> popupDisplay;
    Component* parentForPopupDisplay;

    //==============================================================================
    static double smallestAngleBetween (const double a1, const double a2) noexcept
    {
        return jmin (std::abs (a1 - a2),
                     std::abs (a1 + double_Pi * 2.0 - a2),
                     std::abs (a2 + double_Pi * 2.0 - a1));
    }
};


//==============================================================================
MicronSlider::MicronSlider()
{
    init (Slider::LinearHorizontal, Slider::TextBoxLeft);
}

MicronSlider::MicronSlider (const String& name)  : Slider (name)
{
    init (Slider::LinearHorizontal, Slider::TextBoxLeft);
}

MicronSlider::MicronSlider (Slider::SliderStyle style, Slider::TextEntryBoxPosition textBoxPos)
{
    init (style, textBoxPos);
}

void MicronSlider::init (Slider::SliderStyle style, Slider::TextEntryBoxPosition textBoxPos)
{
    setWantsKeyboardFocus (false);
    setRepaintsOnMouseActivity (true);

    pimpl = new Pimpl (*this, style, textBoxPos);

    lookAndFeelChanged();
    updateText();

    pimpl->registerListeners();
	
	setDoubleClickReturnValue(true, 0.0f);
}

MicronSlider::~MicronSlider() {}

//==============================================================================
void MicronSlider::addListener (MicronSliderListener* const listener)       { pimpl->listeners.add (listener); }
void MicronSlider::removeListener (MicronSliderListener* const listener)    { pimpl->listeners.remove (listener); }

//==============================================================================
Slider::SliderStyle MicronSlider::getSliderStyle() const noexcept     { return pimpl->style; }
void MicronSlider::setSliderStyle (const Slider::SliderStyle newStyle)        { pimpl->setSliderStyle (newStyle); }

void MicronSlider::setRotaryParameters (const float startAngleRadians, const float endAngleRadians, const bool stopAtEnd)
{
    pimpl->setRotaryParameters (startAngleRadians, endAngleRadians, stopAtEnd);
}

void MicronSlider::setVelocityBasedMode (bool vb)                 { pimpl->isVelocityBased = vb; }
bool MicronSlider::getVelocityBasedMode() const noexcept          { return pimpl->isVelocityBased; }
bool MicronSlider::getVelocityModeIsSwappable() const noexcept    { return pimpl->userKeyOverridesVelocity; }
int MicronSlider::getVelocityThreshold() const noexcept           { return pimpl->velocityModeThreshold; }
double MicronSlider::getVelocitySensitivity() const noexcept      { return pimpl->velocityModeSensitivity; }
double MicronSlider::getVelocityOffset() const noexcept           { return pimpl->velocityModeOffset; }

void MicronSlider::setVelocityModeParameters (const double sensitivity, const int threshold,
                                        const double offset, const bool userCanPressKeyToSwapMode)
{
    jassert (threshold >= 0);
    jassert (sensitivity > 0);
    jassert (offset >= 0);

    pimpl->setVelocityModeParameters (sensitivity, threshold, offset, userCanPressKeyToSwapMode);
}

double MicronSlider::getSkewFactor() const noexcept               { return pimpl->skewFactor; }
void MicronSlider::setSkewFactor (const double factor)            { pimpl->skewFactor = factor; }

void MicronSlider::setSkewFactorFromMidPoint (const double sliderValueToShowAtMidPoint)
{
    pimpl->setSkewFactorFromMidPoint (sliderValueToShowAtMidPoint);
}

int MicronSlider::getMouseDragSensitivity() const noexcept        { return pimpl->pixelsForFullDragExtent; }

void MicronSlider::setMouseDragSensitivity (const int distanceForFullScaleDrag)
{
    jassert (distanceForFullScaleDrag > 0);

    pimpl->pixelsForFullDragExtent = distanceForFullScaleDrag;
}

void MicronSlider::setIncDecButtonsMode (const Slider::IncDecButtonMode mode)             { pimpl->setIncDecButtonsMode (mode); }

Slider::TextEntryBoxPosition MicronSlider::getTextBoxPosition() const noexcept    { return pimpl->textBoxPos; }
int MicronSlider::getTextBoxWidth() const noexcept                                { return pimpl->textBoxWidth; }
int MicronSlider::getTextBoxHeight() const noexcept                               { return pimpl->textBoxHeight; }

void MicronSlider::setTextBoxStyle (const Slider::TextEntryBoxPosition newPosition, const bool isReadOnly,
                              const int textEntryBoxWidth, const int textEntryBoxHeight)
{
    pimpl->setTextBoxStyle (newPosition, isReadOnly, textEntryBoxWidth, textEntryBoxHeight);
}

bool MicronSlider::isTextBoxEditable() const noexcept                     { return pimpl->editableText; }
void MicronSlider::setTextBoxIsEditable (const bool shouldBeEditable)     { pimpl->setTextBoxIsEditable (shouldBeEditable); }
void MicronSlider::showTextBox()                                          { pimpl->showTextBox(); }
void MicronSlider::hideTextBox (const bool discardCurrentEditorContents)  { pimpl->hideTextBox (discardCurrentEditorContents); }

void MicronSlider::setChangeNotificationOnlyOnRelease (bool onlyNotifyOnRelease)
{
    pimpl->sendChangeOnlyOnRelease = onlyNotifyOnRelease;
}

bool MicronSlider::getSliderSnapsToMousePosition() const noexcept                 { return pimpl->snapsToMousePos; }
void MicronSlider::setSliderSnapsToMousePosition (const bool shouldSnapToMouse)   { pimpl->snapsToMousePos = shouldSnapToMouse; }

void MicronSlider::setPopupDisplayEnabled (const bool enabled, Component* const parentComponentToUse)
{
    pimpl->popupDisplayEnabled = enabled;
    pimpl->parentForPopupDisplay = parentComponentToUse;
}

Component* MicronSlider::getCurrentPopupDisplay() const noexcept      { return pimpl->popupDisplay.get(); }

//==============================================================================
void MicronSlider::colourChanged()        { lookAndFeelChanged(); }
void MicronSlider::lookAndFeelChanged()   { pimpl->lookAndFeelChanged (getLookAndFeel()); }
void MicronSlider::enablementChanged()    { repaint(); }

//==============================================================================
double MicronSlider::getMaximum() const noexcept      { return pimpl->maximum; }
double MicronSlider::getMinimum() const noexcept      { return pimpl->minimum; }
double MicronSlider::getInterval() const noexcept     { return pimpl->interval; }

void MicronSlider::setRange (double newMin, double newMax, double newInt)
{
    pimpl->setRange (newMin, newMax, newInt);
}

Value& MicronSlider::getValueObject() noexcept        { return pimpl->currentValue; }
Value& MicronSlider::getMinValueObject() noexcept     { return pimpl->valueMin; }
Value& MicronSlider::getMaxValueObject() noexcept     { return pimpl->valueMax; }

double MicronSlider::getValue() const                 { return pimpl->getValue(); }

void MicronSlider::setValue (double newValue, const NotificationType notification)
{
    pimpl->setValue (newValue, notification);
}

double MicronSlider::getMinValue() const      { return pimpl->getMinValue(); }
double MicronSlider::getMaxValue() const      { return pimpl->getMaxValue(); }

void MicronSlider::setMinValue (double newValue, const NotificationType notification, bool allowNudgingOfOtherValues)
{
    pimpl->setMinValue (newValue, notification, allowNudgingOfOtherValues);
}

void MicronSlider::setMaxValue (double newValue, const NotificationType notification, bool allowNudgingOfOtherValues)
{
    pimpl->setMaxValue (newValue, notification, allowNudgingOfOtherValues);
}

void MicronSlider::setMinAndMaxValues (double newMinValue, double newMaxValue, const NotificationType notification)
{
    pimpl->setMinAndMaxValues (newMinValue, newMaxValue, notification);
}

void MicronSlider::setDoubleClickReturnValue (bool isDoubleClickEnabled,  double valueToSetOnDoubleClick)
{
    pimpl->doubleClickToValue = isDoubleClickEnabled;
    pimpl->doubleClickReturnValue = valueToSetOnDoubleClick;
}

double MicronSlider::getDoubleClickReturnValue (bool& isEnabledResult) const
{
    isEnabledResult = pimpl->doubleClickToValue;
    return pimpl->doubleClickReturnValue;
}

void MicronSlider::updateText()
{
    pimpl->updateText();
}

void MicronSlider::setTextValueSuffix (const String& suffix)
{
    pimpl->setTextValueSuffix (suffix);
}

String MicronSlider::getTextValueSuffix() const
{
    return pimpl->textSuffix;
}

String MicronSlider::getTextFromValue (double v)
{
    if (getNumDecimalPlacesToDisplay() > 0)
        return String (v, getNumDecimalPlacesToDisplay()) + getTextValueSuffix();

    return String (roundToInt (v)) + getTextValueSuffix();
}

double MicronSlider::getValueFromText (const String& text)
{
    String t (text.trimStart());

    if (t.endsWith (getTextValueSuffix()))
        t = t.substring (0, t.length() - getTextValueSuffix().length());

    while (t.startsWithChar ('+'))
        t = t.substring (1).trimStart();

    return t.initialSectionContainingOnly ("0123456789.,-")
            .getDoubleValue();
}

double MicronSlider::proportionOfLengthToValue (double proportion)
{
    const double skew = getSkewFactor();

    if (skew != 1.0 && proportion > 0.0)
        proportion = exp (log (proportion) / skew);

    return getMinimum() + (getMaximum() - getMinimum()) * proportion;
}

double MicronSlider::valueToProportionOfLength (double value)
{
    const double n = (value - getMinimum()) / (getMaximum() - getMinimum());
    const double skew = getSkewFactor();

    return skew == 1.0 ? n : pow (n, skew);
}

double MicronSlider::snapValue (double attemptedValue, const bool)
{
    return attemptedValue;
}

int MicronSlider::getNumDecimalPlacesToDisplay() const noexcept    { return pimpl->numDecimalPlaces; }

//==============================================================================
int MicronSlider::getThumbBeingDragged() const noexcept            { return pimpl->sliderBeingDragged; }

void MicronSlider::startedDragging() {}
void MicronSlider::stoppedDragging() {}
void MicronSlider::valueChanged() {}

//==============================================================================
void MicronSlider::setPopupMenuEnabled (const bool menuEnabled)   { pimpl->menuEnabled = menuEnabled; }
void MicronSlider::setScrollWheelEnabled (const bool enabled)     { pimpl->scrollWheelEnabled = enabled; }

bool MicronSlider::isHorizontal() const noexcept   { return pimpl->isHorizontal(); }
bool MicronSlider::isVertical() const noexcept     { return pimpl->isVertical(); }

float MicronSlider::getPositionOfValue (const double value)   { return pimpl->getPositionOfValue (value); }

//==============================================================================
void MicronSlider::paint (Graphics& g)        { pimpl->paint (g, getLookAndFeel()); }
void MicronSlider::resized()                  { pimpl->resized (getLocalBounds(), getLookAndFeel()); }

void MicronSlider::focusOfChildComponentChanged (FocusChangeType)     { repaint(); }

void MicronSlider::mouseDown (const MouseEvent& e)    { pimpl->mouseDown (e); }
void MicronSlider::mouseUp (const MouseEvent&)        { pimpl->mouseUp(); }

void MicronSlider::modifierKeysChanged (const ModifierKeys& modifiers)
{
    if (isEnabled())
        pimpl->modifierKeysChanged (modifiers);
}

void MicronSlider::mouseDrag (const MouseEvent& e)
{
    if (isEnabled())
        pimpl->mouseDrag (e);
}

void MicronSlider::mouseDoubleClick (const MouseEvent&)
{
// NOTE: disabled this standard double-click event, instead we detect it manually in mousedown() and it behaves more nicely.
//    if (isEnabled())
//        pimpl->mouseDoubleClick();
}

void MicronSlider::resetDoubleClickTimer()
{
	pimpl->resetDoubleClickTimer();
}

void MicronSlider::mouseWheelMove (const MouseEvent& e, const MouseWheelDetails& wheel)
{
    if (! (isEnabled() && pimpl->mouseWheelMove (e, wheel)))
        Component::mouseWheelMove (e, wheel);
}
