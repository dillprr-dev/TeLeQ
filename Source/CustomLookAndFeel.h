#pragma once
#include <JuceHeader.h>
#include <juce_gui_basics/juce_gui_basics.h>

class CustomLookAndFeel : public juce::LookAndFeel_V4
{
public:
    // Construtor: Ideal para definir cores padrao que sero usadas.
    CustomLookAndFeel();
   

    juce::Typeface::Ptr phonesTypeface;


    // =====================================================
    // M	TODOS DE DESENHO A SEREM SOBRESCRITOS
    // 
    //     // Cores personalizadas que podemos usar nos desenhos

    // =============================================================

  //Desenha o knob rotativo principal
    void drawRotarySlider(juce::Graphics& g,
        int x, int y, int width, int height,
        float sliderPosProportional,
        float rotaryStartAngle, float rotaryEndAngle,
        juce::Slider& slider) override;




    // 2. Desenha a caixa de texto (opcional, mas bom para estilizar)
    juce::Label* createSliderTextBox(juce::Slider& slider) override;

    void drawComboBox(juce::Graphics& g, int width, int height,
        bool isButtonDown,
        int buttonX, int buttonY, int buttonW, int buttonH,
        juce::ComboBox& box) override;

    void positionComboBoxText(juce::ComboBox& box, juce::Label& label) override;

    void drawLinearSlider(juce::Graphics& g,
        int x, int y, int width, int height,
        float sliderPos,
        float minSliderPos,
        float maxSliderPos,
        const juce::Slider::SliderStyle style,
        juce::Slider& slider) override;

    void drawToggleButton(juce::Graphics& g, juce::ToggleButton& button,
        bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;

    juce::Font CustomLookAndFeel::getTextButtonFont(juce::TextButton&, int buttonHeight);

    juce::Slider::SliderLayout getSliderLayout(juce::Slider& slider) override;



private:
    juce::Colour ledColour;
    juce::Colour backgroundArcColour;
    juce::Colour foregroundArcColour;


};