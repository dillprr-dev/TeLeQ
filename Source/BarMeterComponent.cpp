/*
  ==============================================================================
    BarMeterComponent.cpp
    Created: 5 Dec 2025 8:02:07pm
    Author:  Dill
  ==============================================================================
*/
#include "BarMeterComponent.h"

BarMeterComponent::BarMeterComponent()
{
    setOpaque(false);
}

void BarMeterComponent::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();

    // 1. BACKGROUND SUTIL (cinzento muito escuro com baixa opacidade)
    g.setColour(juce::Colour(0x501a1a1a));
    g.fillRect(bounds);

    // 2. CALCULAR ALTURA DO MEDIDOR
    float meterHeight = bounds.getHeight() * this->peak;

    // 3. CALCULAR O TOPO DO MEDIDOR (crescer de baixo para cima)
    float startY = bounds.getBottom() - meterHeight;

    // 4. DEFINIR A ÁREA DE PREENCHIMENTO
    juce::Rectangle<float> meterArea(
        bounds.getX(),
        startY,
        bounds.getWidth(),
        meterHeight
    );

    // 5. DESENHAR GRADIENTE VERTICAL (verde embaixo → vermelho em cima)
    // A cor muda de acordo com a ALTURA da barra, não com o nível
    juce::ColourGradient gradient(
        juce::Colour(0xFFe74c3c),  // Vermelho no topo (clipping)
        bounds.getX(),
        bounds.getY(),
        juce::Colour(0xFF2ecc71),  // Verde na base
        bounds.getX(),
        bounds.getBottom(),
        false
    );
    g.setGradientFill(gradient);
    g.fillRect(meterArea);

    // 7. BORDA SUTIL (muito discreta)
    g.setColour(juce::Colour(0x802a2a2a));
    g.drawRect(bounds, 0.7f);  // Linha muito fina (0.5px)
}

void BarMeterComponent::update(float newPeakValue)
{
    DBG("BarMeterComponent - Valor Bruto Recebido: " << newPeakValue);

    // Limita o valor entre 0.0 e 1.0
    peak = juce::jlimit(0.0f, 1.0f, newPeakValue);
    DBG("Meter value: " << peak);

    // Força o JUCE a redesenhar o componente
    repaint();
}