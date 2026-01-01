#pragma once
#include <JuceHeader.h>

class CustomSlider : public juce::Slider
{
public:
    CustomSlider();

    enum ParamType
    {
        Gain,
        Frequency,
        QFactor,
        Percentage
    };

    ParamType paramType = Gain;

    juce::String getTextFromValue(double value) override;
    void focusLost(juce::Component::FocusChangeType cause) override;

    // NOVO: Callbacks para drag
    std::function<void()> onDragStart;
    std::function<void()> onDragEnd;

private:
    // NOVO: Variável para rastrear se está arrastando
    bool isDraggingNow = false;

    // NOVO: Sobrescrever eventos de mouse
    void mouseDown(const juce::MouseEvent& event) override;
    void mouseDrag(const juce::MouseEvent& event) override;
    void mouseUp(const juce::MouseEvent& event) override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CustomSlider)
};