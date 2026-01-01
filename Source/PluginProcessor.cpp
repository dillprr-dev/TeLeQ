/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace
{
    // ===== Tape Saturator (pre-emphasis + soft shaper + de-emphasis) =====
    static double tapeSaturator(double x, SaturatorFilters& f)
    {
        // PRE-EMPHASIS
        x = f.pre1.processSample(x);
        x = f.pre2.processSample(x);

        // saturação (SoftClippper assimétrico)
        double a = x + 0.04 * x * x;
        double sat = std::tanh(a * 0.9);

		// POST-DE-EMPHASIS
        sat = f.post1.processSample(sat);
        sat = f.post2.processSample(sat);
        sat = f.post3.processSample(sat);

        return sat;
    }

    // ===== Tube Saturator (triode-like) =====
    static double tubeSaturator(double x, SaturatorFilters& f)
    {
		// PRE-EMPHASIS 
        x = f.pre1.processSample(x);
        x = f.pre2.processSample(x);

		// saturação (modelo polinomial de tríodo)
        double x2 = x * x;
        double x4 = x2 * x2;
        double nonlin = 0.85 * x + 0.15 * x2 + 0.04 * x4;
        double sat = std::tanh(nonlin * 1.1);

		// POST-DE-EMPHASIS 
        sat = f.post1.processSample(sat);
        sat = f.post2.processSample(sat);

        return sat;
    }

    // ===== FET Saturator (knee rapido / compressivo) =====
    static double fetSaturator(double x, SaturatorFilters& f)
    {
		// PRE-EMPHASIS 
        x = f.pre1.processSample(x);
        x = f.pre2.processSample(x);

		// saturação (soft-knee estilo FET)
        double a = x + 0.03 * x * x;
        double sat = a / (1.0 + 0.55 * std::abs(a));

		// POST-DE-EMPHASIS 
        sat = f.post1.processSample(sat);
		sat = f.post2.processSample(sat);

        return sat;
    }
}

namespace TelefySat
{
    // === 1. Telefy FET+ (hiper agressivo) ===
    static inline double fetPlus(double x)
    {
        // Assimetria forte
        double a = x + 0.10 * x * x;

        // FET soft-knee
        double knee = a / (1.0 + 0.35 * std::abs(a));

        // Arredonda topo (simulação de saturação do drain)
        double sat = std::tanh(knee * 1.5);

        return sat;
    }

    // === 2. Telefy Rasp (AM clipping / telefonização distorcida) ===
    static inline double rasp(double x)
    {
        // Assimetria típica de AM overdrive
        double a = x + 0.12 * x * x;

        // Overdrive forte com tanh
        double b = std::tanh(a * 2.4);

        // raspagem (mistura polinomial)
        double rasp = 0.7 * b + 0.3 * (b * b);

        return rasp;
    }
    // ===== Função Wrapper =====
    static double telefySaturator(double x, int type)
    {
        switch (type)
        {
        case 0: return fetPlus(x); // "Distort"
        case 1: return rasp(x); // "Obliterate"
        default: return x;
        }
    }




}




//==============================================================================
TeLeQAudioProcessor::TeLeQAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{

}

TeLeQAudioProcessor::~TeLeQAudioProcessor()
{
}

//==============================================================================
const juce::String TeLeQAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool TeLeQAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool TeLeQAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool TeLeQAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double TeLeQAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int TeLeQAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int TeLeQAudioProcessor::getCurrentProgram()
{
    return 0;
}

void TeLeQAudioProcessor::setCurrentProgram (int index)
{
    juce::ignoreUnused(index);
}

const juce::String TeLeQAudioProcessor::getProgramName (int index)
{
    juce::ignoreUnused(index); 
    return {};
}

void TeLeQAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
    juce::ignoreUnused(index, newName);
}

//==============================================================================
void TeLeQAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    juce::dsp::ProcessSpec spec;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = getTotalNumOutputChannels(); //1; 
    spec.sampleRate = sampleRate;

    leftChain.prepare(spec);
    rightChain.prepare(spec);

    leftTelefyChain.prepare(spec);
    rightTelefyChain.prepare(spec);

    // inicializa auto gain por canal (usa numero de canais de saida)
    autoGains.clear();
    autoGains.resize((size_t)spec.numChannels);
    driveSmoothed.reset(sampleRate, 0.02); // 20 ms suave

    // === PREPARE SATURATOR FILTERS ===
    for (int ch = 0; ch < getTotalNumOutputChannels(); ++ch)
    {
        auto spec = juce::dsp::ProcessSpec{ sampleRate, (juce::uint32)samplesPerBlock, 1 };

        tapeFilters[ch].pre1.prepare(spec);
        tapeFilters[ch].pre2.prepare(spec);
        tapeFilters[ch].post1.prepare(spec);
        tapeFilters[ch].post2.prepare(spec);
		tapeFilters[ch].post3.prepare(spec);

        tubeFilters[ch].pre1.prepare(spec);
        tubeFilters[ch].pre2.prepare(spec);
        tubeFilters[ch].post1.prepare(spec);
        tubeFilters[ch].post2.prepare(spec);

        fetFilters[ch].pre1.prepare(spec);
        fetFilters[ch].pre2.prepare(spec);
        fetFilters[ch].post1.prepare(spec);
        fetFilters[ch].post2.prepare(spec);
    }
    //Prepare Tape Filters
    for (int ch = 0; ch < getTotalNumOutputChannels(); ++ch)
    {
        // --- PRE (Muito sutil - apenas proteção de subsônicos) ---
        tapeFilters[ch].pre1.coefficients = juce::dsp::IIR::Coefficients<double>::makeHighPass(sampleRate, 20.0);  // Remove DC
        tapeFilters[ch].pre2.coefficients = juce::dsp::IIR::Coefficients<double>::makeHighPass(sampleRate, 20.0);  // Bypass (mesmo filtro)

        // --- POST (Muito suave - apenas suavização) ---
        tapeFilters[ch].post1.coefficients = juce::dsp::IIR::Coefficients<double>::makeLowPass(sampleRate, 21000.0);  // Proteção de aliasing
        tapeFilters[ch].post2.coefficients = juce::dsp::IIR::Coefficients<double>::makeHighPass(sampleRate, 20.0);    // Neutral
        tapeFilters[ch].post3.coefficients = juce::dsp::IIR::Coefficients<double>::makeHighPass(sampleRate, 20.0);    // Neutral
    }

    //Prepare Tube Filters
    for (int ch = 0; ch < getTotalNumOutputChannels(); ++ch)
    {
        // --- PRE (Apenas proteção) ---
        tubeFilters[ch].pre1.coefficients = juce::dsp::IIR::Coefficients<double>::makeHighPass(sampleRate, 20.0);   // Remove DC
        tubeFilters[ch].pre2.coefficients = juce::dsp::IIR::Coefficients<double>::makeHighPass(sampleRate, 20.0);   // Neutral

        // --- POST (Apenas proteção de aliasing) ---
        tubeFilters[ch].post1.coefficients = juce::dsp::IIR::Coefficients<double>::makeLowPass(sampleRate, 21000.0); // Proteção
        tubeFilters[ch].post2.coefficients = juce::dsp::IIR::Coefficients<double>::makeHighPass(sampleRate, 20.0);   // Neutral
    }

    //Prepare FET Filters
    for (int ch = 0; ch < getTotalNumOutputChannels(); ++ch)
    {
        // --- PRE (Apenas proteção) ---
        fetFilters[ch].pre1.coefficients = juce::dsp::IIR::Coefficients<double>::makeHighPass(sampleRate, 20.0);    // Remove DC
        fetFilters[ch].pre2.coefficients = juce::dsp::IIR::Coefficients<double>::makeHighPass(sampleRate, 20.0);    // Neutral

        // --- POST (Apenas proteção de aliasing) ---
        fetFilters[ch].post1.coefficients = juce::dsp::IIR::Coefficients<double>::makeLowPass(sampleRate, 21000.0);  // Proteção
        fetFilters[ch].post2.coefficients = juce::dsp::IIR::Coefficients<double>::makeHighPass(sampleRate, 20.0);   // Neutral
    }

    driveSmoothed.reset(sampleRate, 0.02);
    // ...
    // Inicialize telefyAutoGain (assumindo que é um std::vector<AutoGainRMS>)
    telefyAutoGain.clear();
    telefyAutoGain.resize((size_t)spec.numChannels);

    updateFilters(); 
}
void TeLeQAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool TeLeQAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void TeLeQAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    juce::ignoreUnused(midiMessages);
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // 1. LIMPEZA DE CANAIS
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    auto chainSettings = getChainSettings(apvts);
    const int numChannels = buffer.getNumChannels();
    const int numSamples = buffer.getNumSamples();

    // =====================================================================
    // CONVERSÃO FLOAT -> DOUBLE E GANHO DE ENTRADA
    // =====================================================================

    juce::AudioBuffer<FilterCoefficientType> doubleBuffer(numChannels, numSamples);
    doubleBuffer.clear();

    // Copia o sinal de entrada para doubleBuffer
    for (int ch = 0; ch < numChannels; ++ch)
    {
        auto* srcFloat = buffer.getReadPointer(ch);
        auto* destDouble = doubleBuffer.getWritePointer(ch);
        for (int i = 0; i < numSamples; ++i)
            destDouble[i] = static_cast<FilterCoefficientType>(srcFloat[i]);
    }

    // Aplica o Ganho de Entrada
    const double inputGain = juce::Decibels::decibelsToGain(chainSettings.inputGain);
    doubleBuffer.applyGain(inputGain);

    // Input Meters
    if (numChannels >= 1)
    {
        auto peakL = doubleBuffer.getMagnitude(0, 0, numSamples);
        inputPeakL.store(juce::jmax(inputPeakL.load(), static_cast<float>(peakL)));
        DBG("Input L (post-gain): " << static_cast<float>(peakL));
    }

    if (numChannels >= 2)
    {
        auto peakR = doubleBuffer.getMagnitude(1, 0, numSamples);
        inputPeakR.store(juce::jmax(inputPeakR.load(), static_cast<float>(peakR)));
        DBG("Input R (post-gain): " << static_cast<float>(peakR));
    }

    // =====================================================================
    // PROCESSAMENTO EM SÉRIE: Input Gain -> Drive -> EQ -> Telefy -> Output
    // =====================================================================

    updateEffectActivationStates();

    // 1. DRIVE
    if (chainSettings.Drive > 0.0)
    {
        updateDrive(doubleBuffer, chainSettings);
    }

    // 2. EQ PRINCIPAL
    updateFilters();
    updateFilterActivationStates();

    juce::dsp::AudioBlock<FilterCoefficientType> eqBlock(doubleBuffer);

    if (eqBlock.getNumChannels() > 0)
        leftChain.process(juce::dsp::ProcessContextReplacing<FilterCoefficientType>(eqBlock.getSingleChannelBlock(0)));
    if (eqBlock.getNumChannels() > 1)
        rightChain.process(juce::dsp::ProcessContextReplacing<FilterCoefficientType>(eqBlock.getSingleChannelBlock(1)));

    // 3. TELEFY
    if (chainSettings.telefyAmount > 0.0)
    {
        double telefySliderValue = chainSettings.telefyAmount;  // 0.0 a 1.0

        // Mix sobe de 0% para 100% ao longo de todo o slider
        double telefyMixLevel = juce::jmap(telefySliderValue, 0.0, 1.0, 0.0, 1.0);

        // Drive sobe até 50% no meio (0.5) e permanece em 50% até o final
        double telefyDriveLevel = juce::jmin(telefySliderValue * 2.0, 0.5);
        // Explicação: telefySliderValue * 2.0 faz subir 2x mais rápido (0 -> 1.0 em 0.5)
        // juce::jmin(..., 0.5) limita em 0.5 (50%)

        DBG("Telefy Slider: " << telefySliderValue
            << " | Mix: " << (telefyMixLevel * 100) << "% | Drive: " << (telefyDriveLevel * 100) << "%");

        // Criar cópia do buffer para processamento do Telefy
        juce::AudioBuffer<FilterCoefficientType> telefyBuffer(numChannels, numSamples);
        telefyBuffer.makeCopyOf(doubleBuffer);

        // Aplicar Saturação Telefy com o nível de drive calculado
        if (telefyDriveLevel > 0.0)
        {
            // Temporariamente modificar chainSettings para usar o drive correto
            ChainSettings modifiedSettings = chainSettings;
            modifiedSettings.telefyAmount = telefyDriveLevel;
            updateTelefyDrive(telefyBuffer, modifiedSettings);
        }

        // Aplicar Filtro Band-Pass
        updateTelefyFilter(chainSettings);
        juce::dsp::AudioBlock<FilterCoefficientType> telefyBlock(telefyBuffer);

        if (telefyBlock.getNumChannels() > 0)
            leftTelefyChain.process(juce::dsp::ProcessContextReplacing<FilterCoefficientType>(telefyBlock.getSingleChannelBlock(0)));
        if (telefyBlock.getNumChannels() > 1)
            rightTelefyChain.process(juce::dsp::ProcessContextReplacing<FilterCoefficientType>(telefyBlock.getSingleChannelBlock(1)));

        // Fazer o blend final: Telefy wet + Dry (baseado no mix calculado)
        // Com compensação de ganho para manter volume consistente
        for (int ch = 0; ch < numChannels; ++ch)
        {
            auto* dest = doubleBuffer.getWritePointer(ch);
            const double* telefy = telefyBuffer.getReadPointer(ch);

            // Ganho de compensação: quanto maior o mix, maior a compensação
            // A banda passa reduz o volume, então compensamos aumentando
            double compensationGain = 1.0 + (telefyMixLevel * 0.5);  // +0% a +50% de ganho

            for (int i = 0; i < numSamples; ++i)
            {
                double blended = dest[i] * (1.0 - telefyMixLevel) + telefy[i] * telefyMixLevel;
                dest[i] = blended * compensationGain;
            }
        }
    }

    // =====================================================================
    // GANHO DE SAÍDA E OUTPUT METERS
    // =====================================================================

    const double outputGain = juce::Decibels::decibelsToGain(chainSettings.outputGain);
    doubleBuffer.applyGain(outputGain);

    if (numChannels >= 1)
    {
        auto currentOutputPeakL = doubleBuffer.getMagnitude(0, 0, doubleBuffer.getNumSamples());
        outputPeakL.store((float)juce::jmax((double)outputPeakL.load(), currentOutputPeakL));
    }
    if (numChannels >= 2)
    {
        auto currentOutputPeakR = doubleBuffer.getMagnitude(1, 0, doubleBuffer.getNumSamples());
        outputPeakR.store((float)juce::jmax((double)outputPeakR.load(), currentOutputPeakR));
    }

    // =====================================================================
    // CONVERSÃO DOUBLE -> FLOAT
    // =====================================================================

    for (int channel = 0; channel < numChannels; ++channel)
    {
        auto* srcDouble = doubleBuffer.getReadPointer(channel);
        auto* destFloat = buffer.getWritePointer(channel);

        for (int i = 0; i < numSamples; ++i)
            destFloat[i] = static_cast<float>(srcDouble[i]);
    }
}

//==============================================================================
bool TeLeQAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* TeLeQAudioProcessor::createEditor()
{
    return new TeLeQAudioProcessorEditor (*this);
    //return new juce::GenericAudioProcessorEditor(*this);
}

//==============================================================================
void TeLeQAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    juce::MemoryOutputStream mos(destData, true);
    apvts.state.writeToStream(mos);
}

void TeLeQAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    auto tree = juce::ValueTree::readFromData(data, sizeInBytes);
    if (tree.isValid())
    {
        apvts.replaceState(tree);
        updateFilters();
    }

}

ChainSettings getChainSettings(juce::AudioProcessorValueTreeState& apvts)
{
    ChainSettings settings;

    // HPF / LPF
    settings.hpfFreq = apvts.getRawParameterValue("HPFFreq")->load();
    settings.lpfFreq = apvts.getRawParameterValue("LPFFreq")->load();
    settings.hpfSlope = static_cast<Slope>(apvts.getRawParameterValue("HPF_Slope")->load());
    settings.lpfSlope = static_cast<Slope>(apvts.getRawParameterValue("LPF_Slope")->load());
    settings.hpfActive = apvts.getRawParameterValue("HPFActive")->load() > 0.5f;
    settings.lpfActive = apvts.getRawParameterValue("LPFActive")->load() > 0.5f;

    // Low Band
    settings.lowFreq = apvts.getRawParameterValue("LowFreq")->load();
    settings.lowGain = apvts.getRawParameterValue("LowGain")->load();
	settings.lowBell = apvts.getRawParameterValue("LowBell")->load() > 0.5f;

    // Low Mid Band
    settings.lmfFreq = apvts.getRawParameterValue("LowMidFreq")->load();
    settings.lmfGain = apvts.getRawParameterValue("LowMidGain")->load();
    settings.lmfQ = apvts.getRawParameterValue("LowMidQ")->load();

    // High Mid Band
    settings.hmfFreq = apvts.getRawParameterValue("HighMidFreq")->load();
    settings.hmfGain = apvts.getRawParameterValue("HighMidGain")->load();
    settings.hmfQ = apvts.getRawParameterValue("HighMidQ")->load();

    // High Band
    settings.highFreq = apvts.getRawParameterValue("HighFreq")->load();
    settings.highGain = apvts.getRawParameterValue("HighGain")->load();
    settings.highBell = apvts.getRawParameterValue("HighBell")->load() > 0.5f;

    // Drive
    settings.Drive = apvts.getRawParameterValue("DriveAmount")->load();
    settings.driveActive = apvts.getRawParameterValue("driveActivate")->load() > 0.5f;
    settings.driveType = static_cast<int>(apvts.getRawParameterValue("DriveType")->load());
	settings.Mix = apvts.getRawParameterValue("Mix")->load(); 
    settings.inputGain = apvts.getRawParameterValue("InputGain")->load();
    settings.outputGain = apvts.getRawParameterValue("OutputGain")->load();

    // Telefy
    settings.telefyActive = apvts.getRawParameterValue("telefyActivate")->load() > 0.5f;
	settings.telefyFreq = apvts.getRawParameterValue("TelefyFreq")->load();
	settings.telefyQ = apvts.getRawParameterValue("TelefyQ")->load(); 
	settings.telefySatType = static_cast<int>(apvts.getRawParameterValue("DistortionType")->load());
    settings.telefyAmount = apvts.getRawParameterValue("TelefyAmount")->load();


    return settings;
}

void TeLeQAudioProcessor::updateCoefficients(Coefficients& old, const Coefficients& replacements)
{
    *old = *replacements;
}

void TeLeQAudioProcessor::updateDrive(juce::AudioBuffer<double>& buffer, const ChainSettings& chainSettings)
{
    const int numSamples = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();

    // REMOVIDO: A criação e cópia do 'dryBuffer' não é mais necessária, 
    // pois o blend será feito no processBlock (via dryMixBuffer).

    if (numChannels == 0 || numSamples == 0)
    {
        DBG("Alerta: buffer de audio vazio em updateDrive.");
        return;
    }

    // === DRIVE PROCESSING ===
    driveSmoothed.setTargetValue(chainSettings.Drive * 6.0);
    const int driveType = chainSettings.driveType;

    for (int channel = 0; channel < numChannels; ++channel)
    {
        // channelData é o buffer WET de entrada/saída (driveBuffer no processBlock)
        auto* channelData = buffer.getWritePointer(channel);

        // Ponteiro para o AutoGain RMS específico deste canal
        AutoGainRMS* agPtr = nullptr;
        if (channel < (int)autoGains.size())
        {
            agPtr = &autoGains[(size_t)channel];
        }

        // Loop principal (melhor unificar o loop para evitar repetição de código)
        for (int sample = 0; sample < numSamples; ++sample)
        {
            // O valor atual de channelData[sample] é o 'input' (sinal Pós-InputGain)
            double input = channelData[sample];
            double satOutput = 0.0; // Variável para o sinal saturado

            const double driveValue = driveSmoothed.getNextValue();
            const double driveAmount = juce::jlimit(0.1, 10.0, driveValue);
            double x = input * driveAmount; // Pré-gain

            // --- Aplicação da Saturação ---
            switch (driveType)
            {
            case 0: // TAPE
                satOutput = tapeSaturator(x, tapeFilters[channel]);
                break;
            case 1: // TUBE
                satOutput = tubeSaturator(x, tubeFilters[channel]);
                break;
            case 2: // FET
                satOutput = fetSaturator(x, fetFilters[channel]);
                break;
            default:
                satOutput = x; // Se tipo inválido, passa o pré-gain
                break;
            }

            // --- Aplicação do Auto-Gain ---
            if (agPtr != nullptr)
            {
                // Auto-gain RMS (processa input original e o output saturado)
                satOutput = agPtr->process(input, satOutput);
            }
            // else: Se o autoGain falhar (erro na inicialização), satOutput fica com o valor saturado

            // Sobrescreve o buffer com o sinal 100% WET (Saturado + Auto-Ganho)
            channelData[sample] = satOutput;
        }
    }

    // REMOVIDO: Toda a seção "// === WET/DRY MIX ==="
}

void TeLeQAudioProcessor::updateFilters()
{
    auto chainSettings = getChainSettings(apvts);
    updateLowCut(chainSettings);
    updateLowFilter(chainSettings);
    updateLowMidFilter(chainSettings);
    updateHighMidFilter(chainSettings);
    updateHighFilter(chainSettings);
    updateHighCut(chainSettings);
}



void TeLeQAudioProcessor::updateLowCut(const ChainSettings& chainSettings)
{
    auto lowCutCoefficients = juce::dsp::FilterDesign<double>::designIIRHighpassHighOrderButterworthMethod(chainSettings.hpfFreq, getSampleRate(), 2 * (chainSettings.hpfSlope + 1));
    auto& leftHighPass = leftChain.get<ChainPositions::HighPass>();
    auto& rightHighPass = rightChain.get<ChainPositions::HighPass>();
    
    bool hpfActive = chainSettings.hpfActive;

    leftHighPass.setBypassed<0>(true);
    leftHighPass.setBypassed<1>(true);
    rightHighPass.setBypassed<0>(true);
    rightHighPass.setBypassed<1>(true);
    
    switch (chainSettings.hpfSlope)
    {
        case Slope::Slope12:
        {
            *leftHighPass.get<0>().coefficients = *lowCutCoefficients[0];
            leftHighPass.setBypassed<0>(!hpfActive);

            *rightHighPass.get<0>().coefficients = *lowCutCoefficients[0];
            rightHighPass.setBypassed<0>(!hpfActive);
            break;
        }
        case Slope::Slope24:
        {
            *leftHighPass.get<0>().coefficients = *lowCutCoefficients[0];
            leftHighPass.setBypassed<0>(!hpfActive);
            *leftHighPass.get<1>().coefficients = *lowCutCoefficients[1];
            leftHighPass.setBypassed<1>(!hpfActive);

            *rightHighPass.get<0>().coefficients = *lowCutCoefficients[0];
            rightHighPass.setBypassed<0>(!hpfActive);
            *rightHighPass.get<1>().coefficients = *lowCutCoefficients[1];
            rightHighPass.setBypassed<1>(!hpfActive);
            break;
        }
    }
    
}

void TeLeQAudioProcessor::updateLowFilter(const ChainSettings& chainSettings)
{
    if (chainSettings.lowBell)
    {
        FilterCoefficientType fixedLowQ{ 1.2 };
        auto LowPeakCoefficients = juce::dsp::IIR::Coefficients<FilterCoefficientType>::makePeakFilter(
            getSampleRate(),
            chainSettings.lowFreq,
            fixedLowQ,
            juce::Decibels::decibelsToGain(chainSettings.lowGain));

        *leftChain.get<ChainPositions::LowBand>().coefficients = *LowPeakCoefficients;
        *rightChain.get<ChainPositions::LowBand>().coefficients = *LowPeakCoefficients;
    }
    else
    {
        auto LowCoefficients = juce::dsp::IIR::Coefficients<FilterCoefficientType>::makeLowShelf(
            getSampleRate(),
            chainSettings.lowFreq,
            0.5f, // Q fixo para shelf  
            juce::Decibels::decibelsToGain(chainSettings.lowGain));

        *leftChain.get<ChainPositions::LowBand>().coefficients = *LowCoefficients;
        *rightChain.get<ChainPositions::LowBand>().coefficients = *LowCoefficients;
    }
}


void TeLeQAudioProcessor::updateLowMidFilter(const ChainSettings& chainSettings)
{
    auto LowMidCoefficients = juce::dsp::IIR::Coefficients<FilterCoefficientType>::makePeakFilter(
        getSampleRate(),
        chainSettings.lmfFreq,
        chainSettings.lmfQ,
        juce::Decibels::decibelsToGain(chainSettings.lmfGain));

    *leftChain.get<ChainPositions::LowMidBand>().coefficients = *LowMidCoefficients;
    *rightChain.get<ChainPositions::LowMidBand>().coefficients = *LowMidCoefficients;
}

void TeLeQAudioProcessor::updateHighMidFilter(const ChainSettings& chainSettings)
{
    auto HighMidCoefficients = juce::dsp::IIR::Coefficients<FilterCoefficientType>::makePeakFilter(
        getSampleRate(),
        chainSettings.hmfFreq,
        chainSettings.hmfQ,
        juce::Decibels::decibelsToGain(chainSettings.hmfGain));



    *leftChain.get<ChainPositions::HighMidBand>().coefficients = *HighMidCoefficients;
    *rightChain.get<ChainPositions::HighMidBand>().coefficients = *HighMidCoefficients;
}

void TeLeQAudioProcessor::updateHighFilter(const ChainSettings& chainSettings)
{
    if (chainSettings.highBell)
    {
        FilterCoefficientType fixedHighQ{ 1.0 };
        auto HighPeakCoefficients = juce::dsp::IIR::Coefficients<FilterCoefficientType>::makePeakFilter(
            getSampleRate(),
            chainSettings.highFreq,
            fixedHighQ,
            juce::Decibels::decibelsToGain(chainSettings.highGain));

        *leftChain.get<ChainPositions::HighBand>().coefficients = *HighPeakCoefficients;
        *rightChain.get<ChainPositions::HighBand>().coefficients = *HighPeakCoefficients;
    }
    else
    {
        auto HighCoefficients = juce::dsp::IIR::Coefficients<FilterCoefficientType>::makeHighShelf(
            getSampleRate(),
            chainSettings.highFreq,
            0.5f, // Q fixo para shelf  
            juce::Decibels::decibelsToGain(chainSettings.highGain));

        *leftChain.get<ChainPositions::HighBand>().coefficients = *HighCoefficients;
        *rightChain.get<ChainPositions::HighBand>().coefficients = *HighCoefficients;
    }
}

void TeLeQAudioProcessor::updateHighCut(const ChainSettings& chainSettings)
{
    auto highCutCoefficients = juce::dsp::FilterDesign<double>::designIIRLowpassHighOrderButterworthMethod(chainSettings.lpfFreq, getSampleRate(), 2 * (chainSettings.lpfSlope + 1));
    auto& leftLowPass = leftChain.get<ChainPositions::LowPass>();
    auto& rightLowPass = rightChain.get<ChainPositions::LowPass>();

    bool lpfActive = chainSettings.lpfActive;

    leftLowPass.setBypassed<0>(true);
    leftLowPass.setBypassed<1>(true);
    rightLowPass.setBypassed<0>(true);
    rightLowPass.setBypassed<1>(true);

    switch (chainSettings.lpfSlope)
    {
        case Slope::Slope12:
        {
            *leftLowPass.get<0>().coefficients = *highCutCoefficients[0];
            leftLowPass.setBypassed<0>(!lpfActive);

            *rightLowPass.get<0>().coefficients = *highCutCoefficients[0];
            rightLowPass.setBypassed<0>(!lpfActive);
            break;
        }
        case Slope::Slope24:
        {
            *leftLowPass.get<0>().coefficients = *highCutCoefficients[0];
            leftLowPass.setBypassed<0>(!lpfActive);
            *leftLowPass.get<1>().coefficients = *highCutCoefficients[1];
            leftLowPass.setBypassed<1>(!lpfActive);

            *rightLowPass.get<0>().coefficients = *highCutCoefficients[0];
            rightLowPass.setBypassed<0>(!lpfActive);
            *rightLowPass.get<1>().coefficients = *highCutCoefficients[1];
            rightLowPass.setBypassed<1>(!lpfActive);
            break;
        }
    }
}

void TeLeQAudioProcessor::updateFilterActivationStates()
{
    // Obter valores atuais dos knobs
    float hpfFreqValue = *apvts.getRawParameterValue("HPFFreq");
    float lpfFreqValue = *apvts.getRawParameterValue("LPFFreq");

    // Obter os parâmetros de ativação
    auto* hpfActiveParam = apvts.getParameter("HPFActive");
    auto* lpfActiveParam = apvts.getParameter("LPFActive");

    // HPF: Ativar quando frequência > 18 Hz (valor mínimo)
    // Range: 18.f a 450.f
    bool hpfShouldBeOn = hpfFreqValue > 17.0f;
    if ((hpfActiveParam->getValue() >= 0.5f) != hpfShouldBeOn)
    {
        hpfActiveParam->setValueNotifyingHost(hpfShouldBeOn ? 1.0f : 0.0f);
    }

    // LPF: Ativar quando frequência < 22001 Hz (valor máximo)
    // Range: 2000.f a 22000.f
    bool lpfShouldBeOn = lpfFreqValue < 22001.0f;
    if ((lpfActiveParam->getValue() >= 0.5f) != lpfShouldBeOn)
    {
        lpfActiveParam->setValueNotifyingHost(lpfShouldBeOn ? 1.0f : 0.0f);
    }
}

void TeLeQAudioProcessor::updateEffectActivationStates()
{
	float driveAmount = *apvts.getRawParameterValue("DriveAmount");
	float telefyAmount = *apvts.getRawParameterValue("TelefyAmount");

	auto* driveActiveParam = apvts.getParameter("driveActivate");
	auto* telefyActiveParam = apvts.getParameter("telefyActivate");

	bool driveShouldBeOn = driveAmount > 0.0f;
    if ((driveActiveParam->getValue() >= 0.5f) != driveShouldBeOn)
    {
        driveActiveParam->setValueNotifyingHost(driveShouldBeOn ? 1.0f : 0.0f);
	}

    bool telefyShouldBeOn = telefyAmount > 0.0f;
    if ((telefyActiveParam->getValue() >= 0.5f) != telefyShouldBeOn)
    {
        telefyActiveParam->setValueNotifyingHost(telefyShouldBeOn ? 1.0f : 0.0f);
    }



}



void TeLeQAudioProcessor::updateTelefyFilter(const ChainSettings& chainSettings)
{
    auto TelefyCoefficients = juce::dsp::IIR::Coefficients<FilterCoefficientType>::makeBandPass(
        getSampleRate(),
        chainSettings.telefyFreq,
        chainSettings.telefyQ);

    // Configura os coeficientes na nova cadeia Telefy
    *leftTelefyChain.get<0>().coefficients = *TelefyCoefficients;
    *rightTelefyChain.get<0>().coefficients = *TelefyCoefficients;

    // O bypass (ativação) agora é feito na TelefyChain
    // Nota: O TelefyChain tem apenas um elemento, então o índice é 0 (get<0>).
    leftTelefyChain.setBypassed<0>(!chainSettings.telefyActive);
    rightTelefyChain.setBypassed<0>(!chainSettings.telefyActive);
}

void TeLeQAudioProcessor::updateTelefyDrive(juce::AudioBuffer<double>& buffer,
    const ChainSettings& chainSettings)
{
    if (!chainSettings.telefyActive)
        return;

    const int numSamples = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();


    // === TELEFY AMOUNT único (controla drive) ===
    const double amount = juce::jlimit(0.0, 1.0, chainSettings.telefyAmount);

    // Drive sobe linearmente
    const double drive = amount;

    const int satType = chainSettings.telefySatType;

    // === PROCESSAMENTO POR CANAL ===
    for (int ch = 0; ch < numChannels; ++ch)
    {
        auto* samples = buffer.getWritePointer(ch);
        AutoGainRMS& ag = telefyAutoGain[ch];


        for (int i = 0; i < numSamples; ++i)
        {
            // 1. Sinal Seco (antes do drive/saturação)
            const double dry = samples[i];

            // 2. Pré-gain proporcional ao drive
            double x = dry * (1.0 + drive * 5.0);

            // 3. Saturador dedicado do Telefy
            double sat = TelefySat::telefySaturator(x, satType);

            // 4. Auto-Gain RMS → normaliza entre o original (dry) e o saturado (sat)
            sat = ag.process(dry, sat);

            // 5. SOBRESCREVE o buffer com o sinal SATURADO (Wet)
            samples[i] = sat;

        }
    }
}

juce::AudioProcessorValueTreeState::ParameterLayout 
      TeLeQAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    
    juce::StringArray slopeOptions;
    slopeOptions.add("12 dB/Oct"); // indice 0
    slopeOptions.add("24 dB/Oct"); // indice 1

    juce::StringArray telefyDriveOptions;
    telefyDriveOptions.add("Distort"); // indice 0
    telefyDriveOptions.add("Obliterate"); // indice 1



    layout.add(std::make_unique<juce::AudioParameterBool>("HPFActive", "HPF Activate", false));
    layout.add(std::make_unique<juce::AudioParameterBool>("LPFActive", "LPF Activate", false));    

    layout.add (std::make_unique<juce::AudioParameterFloat> ("HPFFreq", 
                                                             "HPF", 
                                                              juce::NormalisableRange<float>(17.f, 450.f, 1.f,0.8f), 
                                                              17.f));

    layout.add(std::make_unique<juce::AudioParameterFloat>("LPFFreq",
                                                           "LPF",
                                                            juce::NormalisableRange<float>(2000.f, 22001.f, 1.f, 0.8f),
                                                            22001.f));

    layout.add(std::make_unique<juce::AudioParameterChoice>("HPF_Slope", "HPF Slope", slopeOptions, 0));
    layout.add(std::make_unique<juce::AudioParameterChoice>("LPF_Slope", "LPF Slope", slopeOptions, 0));


    layout.add(std::make_unique<juce::AudioParameterFloat>("LowFreq", "Low Freq",juce::NormalisableRange<float>(30.f, 500.f, 1.f, 0.4f), 60.f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("LowGain", "Low Gain", juce::NormalisableRange<float>(-18.f, 18.f, 0.1f, 1.f), 0.0f));
    layout.add(std::make_unique<juce::AudioParameterBool>("LowBell", "Low Bell", false));

    layout.add(std::make_unique<juce::AudioParameterFloat>("LowMidFreq", "Low Mid Freq", juce::NormalisableRange<float>(200.f, 2500.f, 1.f, 0.4f), 300.f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("LowMidGain", "Low Mid Gain", juce::NormalisableRange<float>(-18.f, 18.f, 0.1f, 1.f), 0.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("LowMidQ", "Low Mid Q", juce::NormalisableRange<float>(0.4f, 4.f, 0.1f, 0.6f), 1.f));

    layout.add(std::make_unique<juce::AudioParameterFloat>("HighMidFreq", "High Mid Freq", juce::NormalisableRange<float>(600.f, 7200.f, 1.f, 0.4f), 2500.f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("HighMidGain", "High Mid Gain", juce::NormalisableRange<float>(-18.f, 18.f, 0.1f, 1.f), 0.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("HighMidQ", "High Mid Q", juce::NormalisableRange<float>(0.4f, 4.f, 0.1f, 0.6f), 1.f));

    layout.add(std::make_unique<juce::AudioParameterFloat>("HighFreq", "High Freq", juce::NormalisableRange<float>(1500.f, 18000.f, 1.f, 0.4f), 7000.f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("HighGain", "High Gain", juce::NormalisableRange<float>(-18.f, 18.f, 0.1f, 1.f), 0.0f));
    layout.add(std::make_unique<juce::AudioParameterBool>("HighBell", "High Bell", false));

    juce::StringArray driveTypes{ "Tape", "Tube", "FET" };

    layout.add(std::make_unique<juce::AudioParameterFloat>("DriveAmount", "Drive", juce::NormalisableRange<float>(0.0f, 1.f, 0.01f, 1.0f), 0.0f));
    layout.add(std::make_unique<juce::AudioParameterBool>("driveActivate", "Drive Activate", false));
    layout.add(std::make_unique<juce::AudioParameterChoice>("DriveType",            
                                                            "Drive Type",
                                                             driveTypes, 0));
    layout.add(std::make_unique<juce::AudioParameterFloat>("Mix", "Mix", juce::NormalisableRange<float>(0.f, 1.f, 0.01f, 1.f), 1.f));

    layout.add(std::make_unique<juce::AudioParameterBool>("telefyActivate", "Telefy Activate", false));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "TelefyFreq", "Tone",
        juce::NormalisableRange<float>(300.f, 3000.f, 1.f, 0.5f), 1100.f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "TelefyQ", "Intensity",
        juce::NormalisableRange<float>(0.4f, 5.f, 0.1f, 0.6f), 1.2f));

    layout.add(std::make_unique<juce::AudioParameterFloat>("TelefyAmount", "Telefy", juce::NormalisableRange<float>(0.0f, 1.f, 0.01f, 1.f), 0.0f));
    layout.add(std::make_unique<juce::AudioParameterChoice>("DistortionType", "Telefy Type", telefyDriveOptions, 0));

    layout.add(std::make_unique<juce::AudioParameterFloat>("InputGain", "Input Gain", juce::NormalisableRange<float>(-24.f, 12.f, 0.5f, 1.f), 0.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>("OutputGain", "Output Gain", juce::NormalisableRange<float>(-24.f, 12.f, 0.5f, 1.f), 0.0f));



    return layout;
}
//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new TeLeQAudioProcessor();
}