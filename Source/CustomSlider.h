// ================= CustomSlider.h =================

#pragma once
#include <JuceHeader.h>

class CustomSlider : public juce::Slider
{
public:
    enum ParamType { Generic, Gain, Frequency, QFactor, Percentage };
    ParamType paramType = Generic;
    CustomSlider();
    ~CustomSlider() override = default;
    juce::String getTextFromValue(double value) override;

    void mouseDown(const juce::MouseEvent& event) override;
    void mouseUp(const juce::MouseEvent& event) override;
    void focusLost(juce::Component::FocusChangeType cause) override;

//protected:
//    juce::Slider::SliderLayout getSliderLayout() const override;
private:
    bool isBeingDragged = false;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CustomSlider)
};