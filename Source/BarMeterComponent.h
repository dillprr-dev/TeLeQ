/*
  ==============================================================================
    BarMeterComponent.h
    Created: 5 Dec 2025 8:02:07pm
    Author:  Dill
  ==============================================================================
*/
#pragma once
#include <JuceHeader.h>

class BarMeterComponent : public juce::Component
{
public:
    BarMeterComponent();
    ~BarMeterComponent() override = default;

    // Método principal para o desenho do medidor
    void paint(juce::Graphics& g) override;

    // Método para receber o novo valor de pico
    void update(float newPeakValue);

private:
    // O valor de pico atual (normalizado: 0.0 a 1.0)
    float peak = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BarMeterComponent)
};