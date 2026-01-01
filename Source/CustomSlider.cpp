// CustomSlider.cpp
#include "CustomSlider.h"
#include <cmath>

CustomSlider::CustomSlider()
{
    setPaintingIsUnclipped(true);
    setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
}

juce::String CustomSlider::getTextFromValue(double value)
{
    switch (paramType)
    {
    case Gain:
        return juce::String(value, 1) + " dB";
    case Frequency:
        if (value >= 1000.0)
            return juce::String(value / 1000.0, 2) + " kHz";
        return juce::String(value, 0) + " Hz";
    case QFactor:
        return juce::String(value, 1);
    case Percentage: {
        int percentValue = static_cast<int>(std::round(value * 100.0));
        return juce::String(percentValue) + "%";
    }
    default:
        return juce::String(value, 2);
    }
}

void CustomSlider::focusLost(juce::Component::FocusChangeType cause)
{
    if (!isDraggingNow)
    {
        setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    }
    juce::Slider::focusLost(cause);
}

// NOVO: Capturar início do drag
void CustomSlider::mouseDown(const juce::MouseEvent& event)
{
    // Só dispara se o clique for no slider mesmo (não em componentes filhos)
    if (event.eventComponent == this)
    {
        isDraggingNow = true;
        if (onDragStart)
            onDragStart();
    }

    // Chama a implementação da classe base
    juce::Slider::mouseDown(event);
}

// NOVO: Capturar movimento (arrastamento contínuo)
void CustomSlider::mouseDrag(const juce::MouseEvent& event)
{
    // Chama a implementação da classe base (que atualiza o valor)
    juce::Slider::mouseDrag(event);

    // Garante que onDragStart foi disparado
    if (!isDraggingNow)
    {
        isDraggingNow = true;
        if (onDragStart)
            onDragStart();
    }
}

//  NOVO: Capturar fim do drag
void CustomSlider::mouseUp(const juce::MouseEvent& event)
{
    juce::ignoreUnused(event);

    isDraggingNow = false;
    if (onDragEnd)
        onDragEnd();

    // Chama a implementação da classe base
    juce::Slider::mouseUp(event);
}