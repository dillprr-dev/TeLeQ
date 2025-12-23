/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <juce_dsp/juce_dsp.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "BarMeterComponent.h"

using FilterCoefficientType = double;

struct SaturatorFilters
{
    juce::dsp::IIR::Filter<double> pre1, pre2;
    juce::dsp::IIR::Filter<double> post1, post2, post3;
};

//namespace Params
//{
    // Funs para criar os layouts de parmetros - A ser implementado
    // ...
//};

enum Slope
{
    Slope12,
    Slope24
};
// Struct para segurar os parmetros lidos do APVTS
struct ChainSettings
{
	bool hpfActive{ false }, lpfActive{ false }, driveActive{ false }, telefyActive{ false };

    FilterCoefficientType hpfFreq{ 0 }, lpfFreq{ 0 };
    FilterCoefficientType lowFreq{ 0 }, lowGain{ 0 }, lowBell{ false }, lowpeakQ{ 1. };
    FilterCoefficientType lmfFreq{ 0 }, lmfGain{ 0 }, lmfQ{ 0 };
    FilterCoefficientType hmfFreq{ 0 }, hmfGain{ 0 }, hmfQ{ 0 };
    FilterCoefficientType highFreq{ 0 }, highGain{ 0 }, highBell{ false }, highpeakQ{ 1.0 };

    Slope hpfSlope{ Slope::Slope12 }, lpfSlope{Slope::Slope12};

    FilterCoefficientType telefyFreq{ 1100.0 }, telefyQ{ 1.2 }, telefyAmount {1.0};

    double Drive{ 1.0 };   // intensidade da saturo
    double Mix{ 1.0 };    // mistura dry/wet


//	float driveAmount{ 0 };					   // quantidade de drive
    int driveType{ 0 };                       // tipo opcional
    int telefySatType{ 0};

    // INPUT / OUTPUT
    double inputGain{ 0 };     // ganho de entrada em dB
    double outputGain{ 0 };    // ganho de saida em dB
};

ChainSettings getChainSettings(juce::AudioProcessorValueTreeState& apvts);

struct AutoGainRMS
{
    double rmsIn = 1e-12;
    double rmsOut = 1e-12;
    double gain = 1.0;

    // smoothing tempo; ajuste conforme queira (0.003 - 0.01)
    double smoothing = 0.005;

    inline double process(double input, double output)
    {
        // atualiza RMS (exponencial simples)
        rmsIn = (1.0 - smoothing) * rmsIn + smoothing * (input * input);
        rmsOut = (1.0 - smoothing) * rmsOut + smoothing * (output * output);

        double target = (rmsOut > 1e-12 ? std::sqrt(rmsIn / rmsOut) : 1.0);

        // suaviza a transicao do ganho
        gain = (1.0 - smoothing) * gain + smoothing * target;

        return output * gain;
    }
};

//==============================================================================

class TeLeQAudioProcessor : public juce::AudioProcessor
{
public:
    //==============================================================================
    TeLeQAudioProcessor();
    ~TeLeQAudioProcessor() override;

    //==============================================================================
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

#ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
#endif

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    void updateDrive(juce::AudioBuffer<double>& buffer, const ChainSettings& chainSettings);
	void updateTelefyDrive(juce::AudioBuffer<double>& buffer, const ChainSettings& chainSettings);

    std::atomic<float> inputPeakL{ 0.0f };
    std::atomic<float> inputPeakR{ 0.0f };
    std::atomic<float> outputPeakL{ 0.0f };
    std::atomic<float> outputPeakR{ 0.0f };

    float getInputPeakL() const { return inputPeakL.load(); }
    float getInputPeakR() const { return inputPeakR.load(); }
    float getOutputPeakL() const { return outputPeakL.load(); }
    float getOutputPeakR() const { return outputPeakR.load(); }
    

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    juce::AudioProcessorValueTreeState apvts{ *this, nullptr,
        "Parameters", createParameterLayout() };

private:
    using Filter = juce::dsp::IIR::Filter<double>;
    using CutFilter = juce::dsp::ProcessorChain<Filter, Filter>;
    using TelefyChain = juce::dsp::ProcessorChain<juce::dsp::IIR::Filter<FilterCoefficientType>>;



    // NOVO: Instncias da Chain Telefy
    TelefyChain leftTelefyChain;
    TelefyChain rightTelefyChain;
    void updateFilterActivationStates();
    void updateEffectActivationStates();

    std::vector<AutoGainRMS> autoGains;
    std::vector<AutoGainRMS> telefyAutoGain;
    juce::SmoothedValue<double> driveSmoothed;

    SaturatorFilters tapeFilters[3];
    SaturatorFilters tubeFilters[2];
    SaturatorFilters fetFilters[2];
    // ==============================================================================


    using TelefyBand = juce::dsp::ProcessorChain < Filter>;
    void applyTelefyMix(juce::AudioBuffer<double>& wetBuffer,
        const juce::AudioBuffer<double>& dryBuffer,
        const ChainSettings& chainSettings);

    // 2. A nova cadeia principal (7 estgios)
    using MonoChain = juce::dsp::ProcessorChain<CutFilter,   // 0: HighPassCut
        Filter,     // 1: LowBand
        Filter,     // 2: LowMidBand
        Filter,     // 3: HighMidBand (HMF)
        TelefyBand,     // 4: TelefyBandPass
        Filter,     // 5: HighBand
        CutFilter>; // 6: LowPassCut

    MonoChain leftChain, rightChain;

    enum ChainPositions
    {
        HighPass,      // 0: Filtro de Corte HPF
        LowBand,                               // 1: EQ Band Baixa (Shelf/Peak)
        LowMidBand,                            // 2: EQ Band M	dia-Baixa (Peak)
        HighMidBand,                          // 3: EQ Band M	dia-Alta (Peak) - Seu HMF original
        TelefyBandPass,     // 4: Filtro Band-Pass + gain compensation (NOVO)
        HighBand,           // 5: EQ Band Alta (Shelf/Peak)
        LowPass         // 6: Filtro de Corte LPF
    };

 
    void TeLeQAudioProcessor::updateFilters();
    void updateLowCut(const ChainSettings& chainSettings);
	void updateLowFilter(const ChainSettings& chainSettings);
    void updateLowMidFilter(const ChainSettings& chainSettings);
	void updateHighMidFilter(const ChainSettings& chainSettings);
	void updateHighFilter(const ChainSettings& chainSettings);
    void updateHighCut(const ChainSettings& chainSettings);
    void updateTelefyFilter(const ChainSettings& chainSettings);
    //void updateDrive(const ChainSettings& chainSettings);
	//void updateInputOutputGains(const ChainSettings& chainSettings);
	using Coefficients = Filter::CoefficientsPtr;
	static void updateCoefficients(Coefficients& old, const Coefficients& replacements);

	template<typename ChainType, typename CoefficientType>
    void updateCutFilter(ChainType& leftHighPass,
        const CoefficientType& coefficients,
        const ChainSettings& chainSettings);
    

    //==============================================================================

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TeLeQAudioProcessor)
};
