/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include "PluginProcessor.h"
#include <juce_gui_basics/juce_gui_basics.h>
#include "CustomLookAndFeel.h"
#include "HorizontalSelector.h"
#include "CustomSlider.h"


class aboutPanel : public juce::Component
{
public: // <--- ADICIONADO: Construtores/Métodos devem ser públicos
    aboutPanel();
    ~aboutPanel() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

    void mouseDown(const juce::MouseEvent& event) override;



private: // <--- ADICIONADO: Membros internos são privados
    juce::Label titleLabel;
    juce::Label contentLabel;

    // Dimensões do painel de informações que será centralizado
    int panelWidth = 300;
    int panelHeight = 250;

    // Lembrete: O detector de vazamento deve estar aqui.
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(aboutPanel)
};

struct CustomRotarySlider : public CustomSlider
{
	CustomRotarySlider() : CustomSlider()   
                                        
    {
        setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    }
};

//==============================================================================
/**
*/
class TeLeQAudioProcessorEditor  : public juce::AudioProcessorEditor,
                                   public juce::Button::Listener,
                                   juce::Timer
                               //    public juce::ListBoxModel
{
public:
    TeLeQAudioProcessorEditor (TeLeQAudioProcessor&);
    ~TeLeQAudioProcessorEditor() override;
    void closeAboutOverlay();

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

    void buttonClicked(juce::Button* button) override;



    // Métodos da ListBoxModel
    //int getNumRows() override;
//    void paintListBoxItem(int row, juce::Graphics& g, int width, int height, bool rowIsSelected) override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
   // juce::Grid bandGrid;
    TeLeQAudioProcessor& processorRef;
    juce::Image backgroundImage;
    CustomLookAndFeel customLookAndFeel;

    BarMeterComponent inputMeterL;
    BarMeterComponent inputMeterR;
    BarMeterComponent outputMeterL;
    BarMeterComponent outputMeterR;

    void timerCallback() override; // callback do Timer

    juce::Rectangle<float> drivePanel;      // Painel de Saturação (Esquerda)
    juce::Rectangle<float> telefyPanel;     // Painel do Filtro Telefônico (Direita)
    juce::Component drivePanelContainer;
    juce::Component telefyPanelContainer;

    // LOW BAND LABELS
    juce::Label lowGainLabel, lowFreqLabel, lowShelfLabel;
    // LOW-MID BAND LABELS
    juce::Label lowMidGainLabel, lowMidFreqLabel, lowMidQLabel;
    // HIGH-MID BAND LABELS
    juce::Label highMidGainLabel, highMidFreqLabel, highMidQLabel;
    // HIGH BAND LABELS
    juce::Label highGainLabel, highFreqLabel, highShelfLabel;

    juce::Path createHighQPath(juce::Rectangle<int> area);
    juce::Path createLowQPath(juce::Rectangle<int> area);

    void drawPowerLabelSymbol(juce::Graphics& g, juce::Rectangle<float> bounds);

    // ... variáveis de bounds para os 4 símbolos, calculadas no resized()
    juce::Rectangle<float> hpfSymbolBounds;
    juce::Rectangle<float> lpfSymbolBounds;
    juce::Rectangle<float> driveSymbolBounds;
    juce::Rectangle<float> telefySymbolBounds;


    juce::FlexBox mainEqFlex;
    juce::Component lowBandContainer;
    juce::Component lowMidBandContainer;
    juce::Component highMidBandContainer;
    juce::Component highBandContainer;
    TeLeQAudioProcessor& audioProcessor;

    juce::Rectangle<int> symbolArea;
    juce::Rectangle<int> titleArea;
    juce::Rectangle<int> brandArea;
    juce::Rectangle<float> mainPanelArea;

    juce::ComboBox hpfSlopeBox;
    juce::ComboBox lpfSlopeBox;
	juce::ComboBox telefyModeBox;
	juce::ComboBox driveModeBox;

    CustomRotarySlider hpfSlider,
        lpfSlider,
        lowFreqSlider, lowGainSlider,
        lowMidFreqSlider, lowMidGainSlider, lowMidQSlider,
        highMidFreqSlider, highMidGainSlider, highMidQSlider,
        highFreqSlider, highGainSlider,
        telefyToneSlider, telefyIntensitySlider,
        inputGainSlider, outputGainSlider,
        driveGainSlider, driveMixSlider, telefyAmountSlider;
    
    juce::TextButton lowShelfBellButton{ "BELL" };
    juce::TextButton highShelfBellButton{ "BELL" };
    juce::ToggleButton hpfActivateButton{ "" };
    juce::ToggleButton lpfActivateButton{ "" };
    juce::ToggleButton driveActivateButton{ "" };
    juce::ToggleButton telefyActivateButton{ "" };

	using APVTS = juce::AudioProcessorValueTreeState;
	using Attachment = APVTS::SliderAttachment;

    Attachment         hpfAttachment,
        lpfAttachment,
        lowFreqAttachment, lowGainAttachment,
        lowMidFreqAttachment, lowMidGainAttachment, lowMidQAttachment,
        highMidFreqAttachment, highMidGainAttachment, highMidQAttachment,
        highFreqAttachment, highGainAttachment,
        telefyToneAttachment, telefyIntensityAttachment,
        inputGainAttachment, outputGainAttachment,
        driveGainAttachment, driveMixAttachment,
        telefyAmountAttachment;


    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;

    // Toggles Low/High Shelf/Bell (TextButton)
    std::unique_ptr<ButtonAttachment> lowShelfBellAttachment;
    std::unique_ptr<ButtonAttachment> highShelfBellAttachment;

    // Toggles HPF/LPF Activate (ToggleButton)
    // Note que ButtonAttachment funciona tanto para TextButton quanto para ToggleButton
    std::unique_ptr<ButtonAttachment> hpfActivateAttachment;
    std::unique_ptr<ButtonAttachment> lpfActivateAttachment;
	std::unique_ptr<ButtonAttachment> driveActivateAttachment;
	std::unique_ptr<ButtonAttachment> telefyActivateAttachment;

    using ComboBoxAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;

    std::unique_ptr<ComboBoxAttachment> hpfSlopeAttachment; // Usando o alias corrigido
    std::unique_ptr<ComboBoxAttachment> lpfSlopeAttachment; // Usando o alias corrigido
	std::unique_ptr<ComboBoxAttachment> telefyModeAttachment;
    std::unique_ptr<ComboBoxAttachment> driveModeAttachment;

    std::vector<juce::Component*> getComps();

    juce::ColourGradient backgroundGradient;

    std::unique_ptr<juce::DrawableButton> logoImage;
    std::unique_ptr<aboutPanel> aboutOverlay;





    static constexpr int numKnobs = 10;

    std::map<juce::Slider*, std::unique_ptr<juce::Label>> sliderLabels;

    std::array<std::unique_ptr<CustomSlider>, numKnobs> sliders;
    std::array<std::unique_ptr<juce::Label>, numKnobs> labels;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TeLeQAudioProcessorEditor)
};
