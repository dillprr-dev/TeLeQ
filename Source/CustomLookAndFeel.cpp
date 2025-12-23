/*
  ==============================================================================

    CustomLookAndFeel.cpp
    Created: 9 Nov 2025 5:52:03pm
    Author:  Dill

  ==============================================================================
*/

#include "CustomLookAndFeel.h"
#include "CustomSlider.h"

CustomLookAndFeel::CustomLookAndFeel()
{
    setColour(juce::Slider::thumbColourId, ledColour);
    setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);

    foregroundArcColour = juce::Colour(0xFF4DB6AC);
    backgroundArcColour = juce::Colour(0xFF2A4D66);
    ledColour = juce::Colour(0xFF4DB6AC);
}

juce::Font CustomLookAndFeel::getTextButtonFont(juce::TextButton&, int buttonHeight)
{
    // Defina o tamanho da fonte desejado aqui (ex: 10.0f)
    // O JUCE passará a altura do botão, mas usaremos um tamanho fixo para o texto.

    // Retorna a fonte desejada (ex: 10.0f, não negrito)
    return juce::Font(10.0f);

    // Se quiser usar uma das suas fontes customizadas:
    // return getLatoBlackFont(10.0f); 
}

void CustomLookAndFeel::drawRotarySlider(juce::Graphics& g,
    int x, int y, int width, int height,
    float sliderPosProportional,
    float rotaryStartAngle, float rotaryEndAngle,
    juce::Slider&)
{
    // Conversão para float
    auto bounds = juce::Rectangle<float>(static_cast<float>(x),
        static_cast<float>(y),
        static_cast<float>(width),
        static_cast<float>(height));
    auto centre = bounds.getCentre();


    // =========================================================
    // 1. CÁLCULO DE RAIO E ÂNGULO
    // =========================================================
    float radius = juce::jmin(static_cast<float>(width), static_cast<float>(height)) * 0.40f;
    float angle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);

    // =========================================================
    // 2. SOMBRA REALISTA (Direcional - Embaixo e Direita)
    // =========================================================
    // Sombra apenas na base e lado direito (não invade os lados)
    float shadowOffsetX = radius * 0.07f;  // Sombra para a direita
    float shadowOffsetY = radius * 0.10f;  // Sombra para baixo

    // Desenha sombra como uma elipse deformada apenas no canto inferior-direito
    juce::Path shadowPath;
    shadowPath.addEllipse(centre.getX() - radius + shadowOffsetX,
        centre.getY() - radius + shadowOffsetY,
        radius * 2.0f,
        radius * 2.0f);

    g.setColour(juce::Colours::black.withAlpha(0.35f));
    g.fillPath(shadowPath);

    // =========================================================
    // 3. CORPO PRINCIPAL DO KNOB (Gradiente com luz unidirecional)
    // =========================================================
    // Gradiente: luz forte no topo-esquerda, sombra no fundo-direita
    juce::ColourGradient bodyGradient(
        juce::Colours::darkgrey.brighter(0.6f),   // Luz no topo-esquerda
        centre.getX() - radius * 0.3f,
        centre.getY() - radius * 0.4f,
        juce::Colours::darkgrey.darker(0.2f),    // Sombra no fundo-direita
        centre.getX() + radius * 0.4f,
        centre.getY() + radius * 0.5f,
        false
    );

    g.setGradientFill(bodyGradient);
    g.fillEllipse(centre.getX() - radius, centre.getY() - radius, radius * 2.0f, radius * 2.0f);

    // =========================================================
    // 4. BRILHO FRONTAL (Apenas no topo-esquerda)
    // =========================================================
    // Reflexo sutil e realista apenas no lado iluminado
    juce::ColourGradient glossGradient(
        juce::Colours::white.withAlpha(0.08f),
        centre.getX() - radius * 0.5f,
        centre.getY() - radius * 0.7f,
        juce::Colours::white.withAlpha(0.0f),
        centre.getX() + radius * 0.2f,
        centre.getY() + radius * 0.2f,
        false
    );
    g.setGradientFill(glossGradient);
    g.fillEllipse(centre.getX() - radius * 0.7f,
        centre.getY() - radius * 0.8f,
        radius * 1.4f,
        radius * 0.7f);


    // =========================================================
    // 6. INDICADOR CHANFRADO (Mantém original com melhoria)
    // =========================================================
    float indicatorLength = radius * 0.90f;
    float indicatorThickness = radius * 0.11f;
    float innerOffset = radius * 0.42f;

    juce::Point<float> startPoint(
        centre.getX() + (indicatorLength - innerOffset) * std::cos(angle - juce::MathConstants<float>::halfPi),
        centre.getY() + (indicatorLength - innerOffset) * std::sin(angle - juce::MathConstants<float>::halfPi)
    );

    juce::Point<float> endPoint(
        centre.getX() + indicatorLength * std::cos(angle - juce::MathConstants<float>::halfPi),
        centre.getY() + indicatorLength * std::sin(angle - juce::MathConstants<float>::halfPi)
    );

    juce::Path notchPath;
    notchPath.addLineSegment({ startPoint, endPoint }, indicatorThickness);

    // Cor do chanfro: lado sombra (escuro)
    g.setColour(juce::Colours::black.withAlpha(0.8f));
    g.strokePath(notchPath, juce::PathStrokeType(indicatorThickness, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

}

juce::Label* CustomLookAndFeel::createSliderTextBox(juce::Slider& slider)
{
    auto* label = LookAndFeel_V4::createSliderTextBox(slider);
    label->setColour(juce::Label::textColourId, juce::Colours::white.withAlpha(0.8f));
    label->setColour(juce::Label::backgroundColourId, juce::Colours::black.withAlpha(0.5f));
    label->setColour(juce::TextEditor::textColourId, juce::Colours::white.withAlpha(0.9f));
    label->setColour(juce::TextEditor::backgroundColourId, juce::Colours::black.withAlpha(0.3f));
    label->setColour(juce::TextEditor::outlineColourId, ledColour.darker(0.3f));
    label->setJustificationType(juce::Justification::centred);
    return label;
}

void CustomLookAndFeel::drawComboBox(juce::Graphics& g, int width, int height,
    bool isButtonDown,
    int buttonX, int buttonY, int buttonW, int buttonH,
    juce::ComboBox& box)
{
    // ========================================
    // 1. FUNDO TRANSPARENTE
    // ========================================
    g.setColour(juce::Colours::transparentBlack);
    g.fillRect(0, 0, width, height);

    // ========================================
    // 2. DESENHAR SETINHA DISCRETA
    // ========================================

    float arrowSize = juce::jmin(static_cast<float>(buttonW), static_cast<float>(buttonH)) * 0.4f;
    float arrowX = static_cast<float>(buttonX) + static_cast<float>(buttonW) / 2.0f;
    float arrowY = static_cast<float>(buttonY) + static_cast<float>(buttonH) / 2.0f;

    juce::Path arrow;

    if (isButtonDown)
    {
        // Setinha apontando para cima quando aberto
        arrow.addTriangle(
            arrowX - arrowSize * 0.5f, arrowY + arrowSize * 0.3f,
            arrowX + arrowSize * 0.5f, arrowY + arrowSize * 0.3f,
            arrowX, arrowY - arrowSize * 0.3f
        );
    }
    else
    {
        // Setinha apontando para baixo quando fechado
        arrow.addTriangle(
            arrowX - arrowSize * 0.5f, arrowY - arrowSize * 0.3f,
            arrowX + arrowSize * 0.5f, arrowY - arrowSize * 0.3f,
            arrowX, arrowY + arrowSize * 0.3f
        );
    }

    // ========================================
    // 3. COR BASE DA SETINHA
    // ========================================
    g.setColour(juce::Colours::white.withAlpha(0.6f));
    g.fillPath(arrow);

    // ========================================
    // 4. EFEITO HOVER
    // ========================================
    if (box.isMouseOver() && !isButtonDown)
    {
        g.setColour(ledColour.withAlpha(0.4f));
        g.fillPath(arrow);
    }

    // ========================================
    // 5. EFEITO PRESSED
    // ========================================
    if (isButtonDown)
    {
        g.setColour(ledColour.brighter(0.2f));
        g.fillPath(arrow);
    }
}

void CustomLookAndFeel::positionComboBoxText(juce::ComboBox& box, juce::Label& label)
{
    label.setBounds(0, 0, 0, 0);
}

void CustomLookAndFeel::drawLinearSlider(juce::Graphics& g,
    int x, int y, int width, int height,
    float sliderPos,
    float minSliderPos,
    float maxSliderPos,
    const juce::Slider::SliderStyle style,
    juce::Slider& slider)
{
    auto bounds = juce::Rectangle<float>(x, y, width, height);

    // ==========================================================
    // TRACK (fundo) – mantém o mesmo visual que você gostou
    // ==========================================================
    const float trackThickness =
        (style == juce::Slider::LinearHorizontal ? height : width) * 0.28f;

    g.setColour(juce::Colours::darkgrey.darker(0.4f)); // sólido
    if (style == juce::Slider::LinearHorizontal)
    {
        float cy = bounds.getCentreY() - trackThickness * 0.5f;
        g.fillRect(bounds.getX(), cy, bounds.getWidth(), trackThickness);
    }
    else
    {
        float cx = bounds.getCentreX() - trackThickness * 0.5f;
        g.fillRect(cx, bounds.getY(), trackThickness, bounds.getHeight());
    }

    // ==========================================================
    // TRACK PREENCHIDO – mantém o ledColour
    // ==========================================================
    

    if (style == juce::Slider::LinearHorizontal)
    {
        g.setColour(juce::Colours::lightblue.darker(1.0f));
        float cy = bounds.getCentreY() - trackThickness * 0.5f;
        g.fillRect(bounds.getX(), cy,
            sliderPos - bounds.getX(), trackThickness);
    }
    else
    {
        g.setColour(juce::Colours::darkgrey.darker(1.4f));
        float cx = bounds.getCentreX() - trackThickness * 0.5f;
        g.fillRect(cx, sliderPos, trackThickness,
            bounds.getBottom() - sliderPos);
    }

    // ==========================================================
    // TIP (fader cap) – apenas ajustes pedidos
    // ==========================================================

    // Cor sólida — cinza azulado
    juce::Colour tipColour = juce::Colour::fromRGB(63, 63, 73);

    if (style == juce::Slider::LinearHorizontal)
    {
        // HORIZONTAL — inverter proporções:
        //
        // Antes: largo e baixo
        // Agora: mais ALTO e mais ESTREITO
        //
        const float tipWidth = width * 0.12f;  // antes 0.14 — agora mais estreito
        const float tipHeight = height * 0.50f; // antes ~0.65 — agora mais alto

        float cx = sliderPos - tipWidth * 0.5f;
        float cy = bounds.getCentreY() - tipHeight * 0.5f;

        g.setColour(tipColour);
        g.fillRoundedRectangle(cx, cy, tipWidth, tipHeight, 2.0f);

        g.setColour(juce::Colours::black.withAlpha(0.5f));
        g.drawRoundedRectangle(cx, cy, tipWidth, tipHeight, 2.0f, 1.0f);
    }
    else
    {
        // VERTICAL — apenas um pouco menor (pedido)
        const float tipWidth = width * 0.45f;  // levemente mais estreito
        const float tipHeight = height * 0.10f; // antes 0.14 — agora menor

        float cx = bounds.getCentreX() - tipWidth * 0.5f;
        float cy = sliderPos - tipHeight * 0.5f;

        g.setColour(tipColour);
        g.fillRoundedRectangle(cx, cy, tipWidth, tipHeight, 2.0f);

        g.setColour(juce::Colours::black.withAlpha(0.5f));
        g.drawRoundedRectangle(cx, cy, tipWidth, tipHeight, 2.0f, 1.0f);
    }
}

juce::Slider::SliderLayout CustomLookAndFeel::getSliderLayout(juce::Slider& slider)
{
    // A lógica DEVE ser condicional ao tipo de slider:
    if (slider.getSliderStyle() == juce::Slider::RotaryHorizontalVerticalDrag)
    {
        juce::Slider::SliderLayout layout;

        // Usamos getLocalBounds() da SLIDER, não do LookAndFeel
        auto bounds = slider.getLocalBounds();

        int size = juce::jmin(bounds.getWidth(), bounds.getHeight());

        // Área do Slider (Knob): Fixo e Centralizado
        layout.sliderBounds = bounds.withSize(size, size).withCentre(bounds.getCentre());

        // Área do Textbox: Centralizada sobre o Knob
        int textBoxW = 60;
        int textBoxH = 20;

        if (slider.getTextBoxPosition() != juce::Slider::NoTextBox)
        {
            layout.textBoxBounds = {
                bounds.getCentreX() - textBoxW / 2,
                bounds.getCentreY() + textBoxH,
                textBoxW,
                textBoxH
            };
        }
        else
        {
            layout.textBoxBounds = { 0, 0, 0, 0 };
        }

        return layout;
    }

    // Para todos os outros sliders (Lineares), chama a implementação da classe base
    // Note que não há ::juce::Slider:: prefixo aqui.
    return LookAndFeel_V4::getSliderLayout(slider);
}



void CustomLookAndFeel::drawToggleButton(juce::Graphics& g, juce::ToggleButton& button,
    bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown)
{
    auto bounds = button.getLocalBounds().toFloat();

    // ========================================
    // 1. DIMENSÕES DO LED
    // ========================================
    float ledSize = juce::jmin(bounds.getWidth(), bounds.getHeight()) * 0.8f;
    float ledX = bounds.getCentreX() - ledSize / 2.0f;
    float ledY = bounds.getCentreY() - ledSize / 2.0f;

    // ========================================
    // 2. ESTADO DO TOGGLE
    // ========================================
    bool isToggleOn = button.getToggleState();

    // ========================================
    // 3. DESENHAR LED (OFF - Cinza escuro)
    // ========================================
    if (!isToggleOn)
    {
        // Fundo cinza escuro (LED desligado)
        g.setColour(juce::Colours::darkgrey.withAlpha(0.5f));
        g.fillEllipse(ledX, ledY, ledSize, ledSize);

        // Borda sutil
        g.setColour(juce::Colours::black.withAlpha(0.4f));
        g.drawEllipse(ledX, ledY, ledSize, ledSize, 1.0f);
    }
    // ========================================
    // 4. DESENHAR LED (ON - Verde brilhante)
    // ========================================
    else
    {
        // Cor verde do LED
        juce::Colour ledGreen = juce::Colour(0xFF00FF00);  // Verde puro

        // Sombra/Profundidade (fundo mais escuro)
        g.setColour(juce::Colours::black.withAlpha(0.3f));
        g.fillEllipse(ledX + 0.5f, ledY + 0.5f, ledSize - 1.0f, ledSize - 1.0f);

        // Gradiente do corpo do LED (luz no topo, mais escuro na base)
        juce::ColourGradient ledGradient(
            ledGreen.brighter(0.4f),              // Verde mais claro no topo
            ledX + ledSize * 0.3f,
            ledY + ledSize * 0.2f,
            ledGreen.darker(0.1f),                // Verde mais escuro na base
            ledX + ledSize * 0.7f,
            ledY + ledSize * 0.8f,
            false
        );
        g.setGradientFill(ledGradient);
        g.fillEllipse(ledX, ledY, ledSize, ledSize);

        // Brilho/Glow (aura ao redor do LED)
        g.setColour(ledGreen.withAlpha(0.2f));
        g.drawEllipse(ledX - 2.0f, ledY - 2.0f, ledSize + 4.0f, ledSize + 4.0f, 1.5f);
        g.drawEllipse(ledX - 1.0f, ledY - 1.0f, ledSize + 2.0f, ledSize + 2.0f, 1.0f);

        // Reflexo de luz (ponto brilhante)
        g.setColour(juce::Colours::white.withAlpha(0.6f));
        float reflexoSize = ledSize * 0.25f;
        g.fillEllipse(ledX + ledSize * 0.25f, ledY + ledSize * 0.25f, reflexoSize, reflexoSize);

        // Borda do LED (contorno brilhante)
        g.setColour(ledGreen.brighter(0.2f));
        g.drawEllipse(ledX, ledY, ledSize, ledSize, 1.5f);
    }

    // ========================================
    // 5. EFEITO HOVER (Opcional)
    // ========================================
    if (shouldDrawButtonAsHighlighted)
    {
        if (isToggleOn)
        {
            g.setColour(juce::Colours::white.withAlpha(0.15f));
            g.fillEllipse(ledX, ledY, ledSize, ledSize);
        }
    }
}