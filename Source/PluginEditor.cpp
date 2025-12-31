/*

  ==============================================================================



    This file contains the basic framework code for a JUCE plugin editor.



  ==============================================================================
  */
#include "PluginProcessor.h"
#include "PluginEditor.h"


//Carrega a fonte do Titulo do PLugin
juce::Font static getFrankRuehlFont(float height)
{
    static auto typeface = juce::Typeface::createSystemTypefaceFor(
        BinaryData::FrankRuehlCLM_ttf,
        BinaryData::FrankRuehlCLM_ttfSize
    );

    // Cria a juce::Font com FontOptions e o Typeface carregado
    return juce::Font(juce::FontOptions(typeface).withHeight(height));
}

//Carrega a fonte do Titulo da marca
juce::Font static getLatoBlackFont(float height)
{
    static auto typeface = juce::Typeface::createSystemTypefaceFor(
        BinaryData::LatoBlack_ttf,
        BinaryData::LatoBlack_ttfSize
    );

    // Cria a juce::Font com FontOptions e o Typeface carregado
    return juce::Font(juce::FontOptions(typeface).withHeight(height));
}

//Carrega a fonte do estilo do botão telefy
juce::Font static getPHONESFont(float height)
{
    // Usando o nome exato do recurso confirmado pelo usuário.
    static auto typeface = juce::Typeface::createSystemTypefaceFor(
        BinaryData::PHONES_TTF,
        BinaryData::PHONES_TTFSize
    );

    // Cria a juce::Font com FontOptions e o Typeface carregado
    return juce::Font(juce::FontOptions(typeface).withHeight(height));
}

/* /aboutPanel::aboutPanel()
{
    // O painel INTEIRO (o overlay) precisa ser transparente para o efeito dimmer.
    setOpaque(false);
    // 1. Configurar Títulos e Conteúdo
    titleLabel.setText("Sobre o Audio File Player", juce::dontSendNotification);
    titleLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    titleLabel.setFont(20.0f); // LINHA CORRIGIDA (OU juce::FontOptions(20.0f))
    titleLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(titleLabel);
    contentLabel.setText(
        "Version: (pre-alpha) 0.1\n"
        "Developed: Diego Pereira/CRAB AUDIO\n"
        "Utilitário VST para testes e debugs em JUCE.",
        juce::dontSendNotification
    );
    contentLabel.setColour(juce::Label::textColourId, juce::Colours::lightgrey);
    contentLabel.setFont(14.0f); // LINHA CORRIGIDA (OU juce::FontOptions(14.0f))
    contentLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(contentLabel);
} */

aboutPanel::~aboutPanel()
{
    // Destrutor padrão
}

void TeLeQAudioProcessorEditor::closeAboutOverlay()
{
    // A classe Editor pode acessar o membro privado 'aboutOverlay'
    aboutOverlay.reset();
    resized();
}

void aboutPanel::paint(juce::Graphics& g)
{
    DBG("AboutPanel paint...");
    // 1. Desenha o fundo "Dimmer" (preto com 67% de opacidade)
    g.setColour(juce::Colour(0xAA000000));
    g.fillRect(getLocalBounds());

    // 2. Desenha o painel de informações centralizado (o retângulo sólido)
    juce::Rectangle<int> panelArea(panelWidth, panelHeight);
    panelArea.setCentre(getLocalBounds().getCentre());
    g.setColour(juce::Colours::darkgrey);
    g.fillRoundedRectangle(panelArea.toFloat(), 10.0f);

    // 3. Desenha a borda para destacar
    g.setColour(juce::Colours::cyan);
    g.drawRoundedRectangle(panelArea.toFloat(), 10.0f, 2.0f);

    // 4. Desenha instrução de fechar
    g.setColour(juce::Colours::white.withAlpha(0.6f));
    g.setFont(12.0f);
    g.drawText("Clique em qualquer lugar para fechar.",
        panelArea.getCentreX() - 100, panelArea.getBottom() - 30,
        200, 20, juce::Justification::centred, true);
}

void aboutPanel::resized()
{
    // Obtém a área de desenho do painel (o retângulo centralizado)
    juce::Rectangle<int> panelArea(panelWidth, panelHeight);
    panelArea.setCentre(getLocalBounds().getCentre());

    // Define os bounds dos labels dentro da área do painel
    juce::Rectangle<int> innerArea = panelArea.reduced(20);
    titleLabel.setBounds(innerArea.removeFromTop(40));
    innerArea.removeFromTop(10); // Espaço
    contentLabel.setBounds(innerArea.removeFromTop(100));
}

void aboutPanel::mouseDown(const juce::MouseEvent& event)
{
    juce::ignoreUnused(event);
    DBG("aboutPanel::mouseDown");

    // 1. Encontra o Editor pai
    if (auto* parent = findParentComponentOfClass<TeLeQAudioProcessorEditor>())
    {
        // 2. Chama a função pública do Editor para que ele se encarregue de fechar
        parent->closeAboutOverlay(); // <--- CHAMA FUNÇÃO PÚBLICA!
    }
}

//==============================================================================

TeLeQAudioProcessorEditor::TeLeQAudioProcessorEditor(TeLeQAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p), processorRef(p),
    hpfAttachment(audioProcessor.apvts, "HPFFreq", hpfSlider),
    lpfAttachment(audioProcessor.apvts, "LPFFreq", lpfSlider),
    lowFreqAttachment(audioProcessor.apvts, "LowFreq", lowFreqSlider),
    lowGainAttachment(audioProcessor.apvts, "LowGain", lowGainSlider),
    lowMidFreqAttachment(audioProcessor.apvts, "LowMidFreq", lowMidFreqSlider),
    lowMidGainAttachment(audioProcessor.apvts, "LowMidGain", lowMidGainSlider),
    lowMidQAttachment(audioProcessor.apvts, "LowMidQ", lowMidQSlider),
    highMidFreqAttachment(audioProcessor.apvts, "HighMidFreq", highMidFreqSlider),
    highMidGainAttachment(audioProcessor.apvts, "HighMidGain", highMidGainSlider),
    highMidQAttachment(audioProcessor.apvts, "HighMidQ", highMidQSlider),
    highFreqAttachment(audioProcessor.apvts, "HighFreq", highFreqSlider),
    highGainAttachment(audioProcessor.apvts, "HighGain", highGainSlider),
    telefyToneAttachment(audioProcessor.apvts, "TelefyFreq", telefyToneSlider),
    telefyIntensityAttachment(audioProcessor.apvts, "TelefyQ", telefyIntensitySlider),
    inputGainAttachment(audioProcessor.apvts, "InputGain", inputGainSlider),
    outputGainAttachment(audioProcessor.apvts, "OutputGain", outputGainSlider),
    driveGainAttachment(audioProcessor.apvts, "DriveAmount", driveGainSlider),
    driveMixAttachment(audioProcessor.apvts, "Mix", driveMixSlider),
	telefyAmountAttachment(audioProcessor.apvts, "TelefyAmount", telefyAmountSlider)        


{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.


    hpfActivateAttachment = std::make_unique<ButtonAttachment>(audioProcessor.apvts, "HPFActive", hpfActivateButton);
    lpfActivateAttachment = std::make_unique<ButtonAttachment>(audioProcessor.apvts, "LPFActive", lpfActivateButton);
    hpfActivateButton.setEnabled(false);  // Impede cliques
    lpfActivateButton.setEnabled(false);  // Impede cliques
    driveActivateAttachment = std::make_unique<ButtonAttachment>(audioProcessor.apvts, "driveActivate", driveActivateButton);
    telefyActivateAttachment = std::make_unique<ButtonAttachment>(audioProcessor.apvts, "telefyActivate", telefyActivateButton);
	driveActivateButton.setEnabled(false);  // Impede cliques
	telefyActivateButton.setEnabled(false);  // Impede cliques 
    lowShelfBellAttachment = std::make_unique<ButtonAttachment>(audioProcessor.apvts, "LowBell", lowShelfBellButton);
    highShelfBellAttachment = std::make_unique<ButtonAttachment>(audioProcessor.apvts, "HighBell", highShelfBellButton);
    lowShelfBellButton.setButtonText("SHELF");
    highShelfBellButton.setButtonText("SHELF");

    // 1. HPF Slope Box
    hpfSlopeBox.addItem("12 dB/Oct", 1); // ID 1
    hpfSlopeBox.addItem("24 dB/Oct", 2); // ID 2
    hpfSlopeBox.setSelectedId(1);
    // Você não precisa do addListener(this) se estiver usando attachments
    addAndMakeVisible(hpfSlopeBox);

    // 2. LPF Slope Box
    lpfSlopeBox.addItem("12 dB/Oct", 1); // ID 1
    lpfSlopeBox.addItem("24 dB/Oct", 2); // ID 2
    lpfSlopeBox.setSelectedId(1);
    addAndMakeVisible(lpfSlopeBox);

	driveModeBox.addItem("TAPE", 1); // ID 1  
	driveModeBox.addItem("TUBE", 2); // ID 2
	driveModeBox.addItem("FET", 3); // ID 3 
	addAndMakeVisible(driveModeBox);

	telefyModeBox.addItem("TYPE 1", 1); // ID 1
	telefyModeBox.addItem("TYPE 2", 2); // ID 2
	addAndMakeVisible(telefyModeBox);   

    addAndMakeVisible(inputMeterL);
    addAndMakeVisible(inputMeterR);
    addAndMakeVisible(outputMeterL);
    addAndMakeVisible(outputMeterR);

    // Inicia o timer para atualizar a UI 30 vezes por segundo
    startTimerHz(50);

    // Anexo do HPF Slope
    hpfSlopeAttachment = std::make_unique<ComboBoxAttachment>(
        audioProcessor.apvts,
        "HPF_Slope",                                        // Parameter ID do Processador
        hpfSlopeBox
    );

    // Anexo do LPF Slope
    lpfSlopeAttachment = std::make_unique<ComboBoxAttachment>(
        audioProcessor.apvts,
        "LPF_Slope",                                        // Parameter ID do Processador
        lpfSlopeBox
    );

    driveModeAttachment = std::make_unique<ComboBoxAttachment>(
        audioProcessor.apvts,
        "DriveType",                                        // Parameter ID do Processador
        driveModeBox
	);

    telefyModeAttachment = std::make_unique<ComboBoxAttachment>(
        audioProcessor.apvts,
        "DistortionType",                                        // Parameter ID do Processador
        telefyModeBox
	);

    setLookAndFeel(&customLookAndFeel);


    lowShelfBellButton.setClickingTogglesState(true);
    highShelfBellButton.setClickingTogglesState(true);

    // Garanta que todos os seus sliders usem o estilo RotaryHorizontalVerticalDrag 
    // para que drawRotarySlider seja chamado.

// INPUT GAIN
    inputGainSlider.setSliderStyle(CustomSlider::LinearVertical);
    inputGainSlider.setRange(-50.0, 30.0, 0.1);
    inputGainSlider.setValue(0.0);
    addAndMakeVisible(inputGainSlider);

    // OUTPUT GAIN
    outputGainSlider.setSliderStyle(CustomSlider::LinearVertical);
    outputGainSlider.setRange(-50.0, 30.0, 0.1);
    outputGainSlider.setValue(0.0);
    addAndMakeVisible(outputGainSlider);


    // Seu loop de configuração (Ajustado)
    for (auto* comp : getComps())
    {
        if (auto* slider = dynamic_cast<CustomSlider*>(comp))
        {
            // Aplica Rotary a TODOS, exceto aqueles que ja foram configurados
            // para LinearVertical (inputGainSlider e outputGainSlider)
            if (slider->getSliderStyle() != CustomSlider::LinearVertical)
            {
                slider->setSliderStyle(CustomSlider::RotaryHorizontalVerticalDrag);
            }
        }
        addAndMakeVisible(comp); // Adiciona todos
    }

    setSize(500, 600);

    auto tempLogoDrawable = juce::Drawable::createFromImageData(

        BinaryData::Logo_svg,
        BinaryData::Logo_svgSize);

    if (tempLogoDrawable)
    {
        // 1. CRIAÇÃO: O logoImage DEVE ser inicializado primeiro
        logoImage = std::make_unique<juce::DrawableButton>("About", juce::DrawableButton::ImageFitted);

        // 2. CONFIGURAÇÃO (Agora é seguro usar logoImage->)
        // logoImage->setAlwaysOnTop(true); // Remova, pois não é estritamente necessário e pode causar problemas
        logoImage->setColour(juce::TextButton::buttonColourId, juce::Colours::red); // Diagnóstico visual

        // 3. setImages e Transferência de Propriedade (Correta)
        logoImage->setImages(tempLogoDrawable.get());
        tempLogoDrawable.release();

        // 3. Estado inicial: (Garantir que o texto seja exibido)
        lowShelfBellButton.setButtonText("SHELF");
        highShelfBellButton.setButtonText("SHELF");

        // 4. ADIÇÃO e LISTENERS
        addAndMakeVisible(*logoImage);
        logoImage->addListener(this);
        lowShelfBellButton.addListener(this);
        highShelfBellButton.addListener(this);
        resized();

    }

}



TeLeQAudioProcessorEditor::~TeLeQAudioProcessorEditor()

{
    stopTimer();
}

juce::Path TeLeQAudioProcessorEditor::createHighQPath(juce::Rectangle<int> area)
{
    juce::Path p;
    float halfWidth = area.getWidth() * 0.5f;
    float height = area.getHeight();

    // Começa na base esquerda da área do ícone (High Q)
    p.startNewSubPath(area.getX(), area.getBottom());

    // Linha até o pico central (no topo do iconArea)
    p.lineTo(area.getX() + halfWidth, area.getY());

    // Linha até a base direita da área do ícone
    p.lineTo(area.getX() + area.getWidth(), area.getBottom());

    return p;
}

juce::Path TeLeQAudioProcessorEditor::createLowQPath(juce::Rectangle<int> area)
{
    juce::Path p;
    float halfWidth = area.getWidth() * 0.5f;
    float height = area.getHeight();
    float curveFactor = 0.5f; // Fator de profundidade da curva

    // Ponto de partida na base esquerda
    p.startNewSubPath(area.getX(), area.getBottom());

    // Curva quadrática para o fundo do "U" (o ponto de controle vai determinar a abertura)
    // Ponto de controle: no centro, mas puxado para CIMA (o que era o topo da área original)
    p.quadraticTo(area.getX() + halfWidth, area.getY() + height * curveFactor,
        area.getX() + area.getWidth(), area.getBottom());

    return p;
}


float gainToNormalizedDb(float gain, float minDb = -60.0f, float maxDb = 0.0f)
{
    if (gain <= 0.0f) return 0.0f;
    float db = juce::Decibels::gainToDecibels(gain, minDb);
    float normalized = juce::jmap(db, minDb, maxDb, 0.0f, 1.0f);
    return juce::jlimit(0.0f, 1.0f, normalized);
}

void TeLeQAudioProcessorEditor::timerCallback()
{
    // 1. Decaimento e Leitura dos Picos
    // O decaimento suave (multiplicar por 0.95) é crucial para que o medidor caia lentamente

    DBG("Timer ativo!");

    // === INPUT METERS ===
    // Canal L
    float decayedInputL = audioProcessor.getInputPeakL() * 0.95f;
    float inputDbL = gainToNormalizedDb(decayedInputL);
    inputMeterL.update(inputDbL);
    DBG("Input L: " << juce::Decibels::gainToDecibels(decayedInputL) << " dB");

    // Canal R
    float decayedInputR = audioProcessor.getInputPeakR() * 0.95f;
    float inputDbR = gainToNormalizedDb(decayedInputR);
    inputMeterR.update(inputDbR);
    DBG("Input R: " << juce::Decibels::gainToDecibels(decayedInputR) << " dB");

    // === OUTPUT METERS ===
    // Canal L
    float decayedOutputL = audioProcessor.getOutputPeakL() * 0.95f;
    float outputDbL = gainToNormalizedDb(decayedOutputL);
    outputMeterL.update(outputDbL);
    DBG("Output L: " << juce::Decibels::gainToDecibels(decayedOutputL) << " dB");

    // Canal R
    float decayedOutputR = audioProcessor.getOutputPeakR() * 0.95f;
    float outputDbR = gainToNormalizedDb(decayedOutputR);
    outputMeterR.update(outputDbR);
    DBG("Output R: " << juce::Decibels::gainToDecibels(decayedOutputR) << " dB");

    // === ATUALIZAR OS VALORES ATÔMICOS NO PROCESSOR ===
    // Use .store() para atribuir valores a std::atomic
    audioProcessor.inputPeakL.store(decayedInputL);
    audioProcessor.inputPeakR.store(decayedInputR);
    audioProcessor.outputPeakL.store(decayedOutputL);
    audioProcessor.outputPeakR.store(decayedOutputR);
}


//==============================================================================

void TeLeQAudioProcessorEditor::paint(juce::Graphics& g)

{
    DBG("Editor paint...");
    //Carrega a imagem de fundo com gradiente via BinaryData
    g.drawImage(backgroundImage,
        getLocalBounds().toFloat(),
        juce::RectanglePlacement::stretchToFit);

    // Desenha o painel principal com cantos arredondados
    //juce::Rectangle<float> bounds = getLocalBounds().toFloat();
    const float cornerRadius = 15.0f;
    juce::Colour panelColour = juce::Colour(0x201F4363);
    // 3. Preencher o painel com cantos arredondados
    g.setColour(panelColour);
    //g.fillRoundedRectangle(mainPanelArea, cornerRadius);
    // 4. Desenhar a borda do painel
    const float borderThickness = 0.5f;
    juce::Colour borderColor = juce::Colour(0x40000000); // Uma versão mais clara do painel para o contorno
    g.setColour(borderColor);
    //g.drawRoundedRectangle(mainPanelArea, cornerRadius, borderThickness);



    // =========================================================================
    // === NOVO: DESENHAR APENAS LINHAS DIVISÓRIAS VERTICAIS ===
    // =========================================================================

    // 1. Definições de Estilo
    const float contourThickness = 0.5f;
    const juce::Colour contourColour = juce::Colour(0x40000000);
    g.setColour(contourColour);

    // Obtém as áreas dos CONTÊINERES
    const auto lowArea = lowBandContainer.getBounds().toFloat();
    const auto lowMidArea = lowMidBandContainer.getBounds().toFloat();
    const auto highMidArea = highMidBandContainer.getBounds().toFloat();
    const auto highArea = highBandContainer.getBounds().toFloat();


    // 2. Coordenadas Y para o desenho vertical
    // A linha deve começar no topo dos containers (menos a margem de 5px)
    const float topY = mainPanelArea.getY();
    const float verticalStopOffset = (lowMidArea.getBottom() - mainPanelArea.getY()) * 0.0265f;

    // A linha deve parar na altura da base onde o EQ visualmente termina (logo acima dos filtros HPF/LPF)
    // Usamos a mesma coordenada de âncora Y de antes para uma parada limpa.
    const float angleBaseY = lowMidArea.getBottom() + (lowMidArea.getBottom() - mainPanelArea.getY()) * 0.0265f;
    // --- 3. Calcular Coordenadas X das Divisões Verticais ---
    // Divisão 1: Low / LowMid -> Ponto médio entre o fim da Low e o início da LowMid
    float division1X = (lowArea.getRight() + lowMidArea.getX()) / 2.0f;
    // Divisão 2: LowMid / HighMid -> Ponto médio entre o fim da LowMid e o início da HighMid
    float division2X = (lowMidArea.getRight() + highMidArea.getX()) / 2.0f;
    // Divisão 3: HighMid / High -> Ponto médio entre o fim da HighMid e o início da High
    float division3X = (highMidArea.getRight() + highArea.getX()) / 2.0f;

    const float bandRadius = 15.0f; // Raio que você mencionou (o mesmo do mainPanelArea)

    g.setColour(juce::Colours::white.withAlpha(0.7f));

    // LABELS DOS KNOBS DO EQ
    auto drawKnobLabel = [&](juce::Component& knob, const juce::String& text, int offsetFromTop)
        {
            auto bounds = knob.getBounds();
            g.drawText(text,
                bounds.getX(),
                bounds.getY() - offsetFromTop, // Posição Y: Acima do topo do knob
                bounds.getWidth(),
                15, // Altura reservada para o texto (ex: 12px)
                juce::Justification::centred,
                false);
        };

    // --- Low Band ---
    // OFFSET: 15px acima do topo do knob (Ganho é o knob de cima)
    drawKnobLabel(lowGainSlider, "Gain", 15);
    drawKnobLabel(lowFreqSlider, "Freq", 15);
    // Para o botão Shelf/Bell, o label é o tipo de filtro (Shelf/Bell), que pode ser desenhado dinamicamente.

    // --- Low-Mid Band ---
    drawKnobLabel(lowMidGainSlider, "Gain", 15);
    drawKnobLabel(lowMidFreqSlider, "Freq", 15);
    drawKnobLabel(lowMidQSlider, "Q", 15);

    // --- High-Mid Band ---
    drawKnobLabel(highMidGainSlider, "Gain", 15);
    drawKnobLabel(highMidFreqSlider, "Freq", 15);
    drawKnobLabel(highMidQSlider, "Q", 15);

    // --- High Band ---
    drawKnobLabel(highGainSlider, "Gain", 15);
    drawKnobLabel(highFreqSlider, "Freq", 15);

    g.drawText("HPF",
        hpfSlider.getX() - 15,
        hpfSlider.getY() - 20,
        hpfSlider.getWidth() + 30, 18,
        juce::Justification::centred, false);

    g.drawText("LPF",
        lpfSlider.getX() - 15,
        lpfSlider.getY() - 20,
        lpfSlider.getWidth() + 30, 18,
        juce::Justification::centred, false);

    // LABELS DO GAIN
    g.setFont(juce::Font(15.0f).withStyle(juce::Font::bold));

    g.drawText("IN",
        inputGainSlider.getX() + inputGainSlider.getWidth() / 2,
        inputGainSlider.getY() - 18,
        inputGainSlider.getWidth(), 30,
        juce::Justification::centred, false);

    g.drawText("OUT",
        outputGainSlider.getX() - inputGainSlider.getWidth() / 2,
        outputGainSlider.getY() - 18,
        outputGainSlider.getWidth(), 30,
        juce::Justification::centred, false);

    //desenha o SYMBOL
    g.setColour(juce::Colour(0xFF507B88));
    g.setFont(getPHONESFont(60.0f));
    g.drawFittedText("T", symbolArea,
    juce::Justification::centred, 1);

    //Desenha o BRAND TITTLE        
    g.setColour(juce::Colour(0xFF507B88));
    g.setFont(getLatoBlackFont(25.0f));
    g.drawFittedText("CRAB AUDIO", brandArea,
    juce::Justification::centred, 1);

    // Desenha o Painel de Saturação (Esquerda)
    g.setColour(panelColour);
    g.fillRoundedRectangle(drivePanel, cornerRadius);
    g.setColour(borderColor);
    g.drawRoundedRectangle(drivePanel, cornerRadius, borderThickness);


    // Desenha o Painel do Filtro Telefônico (Direita)
    g.setColour(panelColour);
    g.fillRoundedRectangle(telefyPanel, cornerRadius);
    g.setColour(borderColor);
    g.drawRoundedRectangle(telefyPanel, cornerRadius, borderThickness);

    // Labels dos painéis laterais
    g.setColour(juce::Colours::white.withAlpha(0.7f));
    g.setFont(juce::Font(11.0f).withStyle(juce::Font::bold));

    //g.setColour(juce::Colours::white.withAlpha(0.7f));
    g.drawText("DRIVE",
        drivePanel.getX(),
        drivePanel.getY() + 30,
        drivePanel.getWidth(), 15,
        juce::Justification::centred, false);

    g.drawText("MIX",
        drivePanel.getX(),
        drivePanel.getY() + 60,
        drivePanel.getWidth(), 15,
        juce::Justification::centred, false);

    g.drawText("TELEFY",
        telefyPanel.getX(),
        telefyPanel.getY() + 30,
        telefyPanel.getWidth(), 15,
        juce::Justification::centred, false);

    g.drawText("TONE",
        telefyPanel.getX(),
        telefyPanel.getY() + 60,
        telefyPanel.getWidth(), 15,
        juce::Justification::centred, false);

}

void TeLeQAudioProcessorEditor::resized()
{
    // === 1. OBTER BOUNDS DA JANELA ===
    // [MANTER] 'bounds' é a referência para o gradiente e os elementos externos (Logo, Title, Brand Title).
    auto bounds = getLocalBounds().toFloat();

    backgroundImage = juce::ImageFileFormat::loadFrom(
        BinaryData::Background_png,
        BinaryData::Background_pngSize
    );

    // === 3. CALCULAR POSIÇÃO E TAMANHO DO PAINEL PRINCIPAL ===
    float headerMargin = bounds.getHeight() * 0.05f;
    float sideMargin = bounds.getWidth() * 0.12f;
    float panelHeightRatio = 0.6f;
    float panelWidthRatio = 0.76f;
    float panelX = sideMargin;
    float panelY = headerMargin;
    float panelWidth = bounds.getWidth() * panelWidthRatio;
    float panelHeight = bounds.getHeight() * panelHeightRatio;
    mainPanelArea.setBounds(panelX, panelY, panelWidth, panelHeight);


    // === PAINÉIS LATERAIS ===
    float sideMarginX = sideMargin;          // Margem menor para os painéis laterais
    float sidePanelWidth = panelWidth * 0.45;       // Largura dos painéis laterais
    float sidePanelHeight = panelHeight * 0.30f;     // Altura (50% do painel principal)

    // Calcula a posição Y dos painéis laterais (alinhado com a base do painel principal)
    float sidePanelY = mainPanelArea.getBottom() + panelY * 0.40f;

    // === PAINEL DRIVE (Esquerda) ===
    float drivePanelX = sideMarginX;
    drivePanel.setBounds(drivePanelX, sidePanelY, sidePanelWidth, sidePanelHeight);
    drivePanelContainer.setBounds(drivePanel.toNearestInt());

    // === PAINEL TELEFY (Direita) ===
    float telefyPanelX = bounds.getWidth() - sideMarginX - sidePanelWidth;
    telefyPanel.setBounds(telefyPanelX, sidePanelY, sidePanelWidth, sidePanelHeight);
    telefyPanelContainer.setBounds(telefyPanel.toNearestInt());

    // === POSICIONAR COMPONENTES DENTRO DOS PAINÉIS ===
    // DRIVE PANEL (Saturação)
    auto driveBounds = drivePanel.toNearestInt().reduced(10);
    int driveSliderHeight = juce::roundToInt(sidePanelHeight * 0.15f);
    int driveSpacing = juce::roundToInt(sidePanelHeight * 0.15f);

    int driveYPos = driveBounds.getY() + driveSpacing * 2;  // Espaço para o label
    driveGainSlider.setSliderStyle(CustomSlider::LinearHorizontal);
    driveGainSlider.setBounds(driveBounds.getX(), driveYPos, driveBounds.getWidth(), driveSliderHeight);

    driveYPos += driveSliderHeight + driveSpacing;
    driveMixSlider.setSliderStyle(CustomSlider::LinearHorizontal);
    driveMixSlider.setBounds(driveBounds.getX(), driveYPos, driveBounds.getWidth(), driveSliderHeight);

    int btnSize = 14; // recomendo algo maior que 5px pra parecer LED
    driveActivateButton.setBounds(
        driveBounds.getRight() - btnSize,   // colado à direita
        driveBounds.getY(),             // um pouquinho abaixo da borda superior
        btnSize,
        btnSize
    );
    static int modeBoxWidth = driveBounds.getWidth() * 0.15f;  // 45% do painel
    static int modeBoxHeight = driveBounds.getHeight() * 0.25f;

    driveModeBox.setBounds(
        driveBounds.getX(),
        driveBounds.getY(),
        modeBoxWidth,
        modeBoxHeight
    );

    // TELEFY PANEL (Filtro Telefônico)
    auto telefyBounds = telefyPanel.toNearestInt().reduced(10);
    int telefySliderHeight = juce::roundToInt(sidePanelHeight * 0.15f);
    int telefySpacing = juce::roundToInt(sidePanelHeight * 0.15f);

    int telefyYPos = telefyBounds.getY() + telefySpacing * 2;  // Espaço para o label
    telefyAmountSlider.setSliderStyle(CustomSlider::LinearHorizontal);
    telefyAmountSlider.setBounds(telefyBounds.getX(), telefyYPos, telefyBounds.getWidth(), telefySliderHeight);

    telefyYPos += telefySliderHeight + telefySpacing;
    telefyToneSlider.setSliderStyle(CustomSlider::LinearHorizontal);
    telefyToneSlider.setBounds(telefyBounds.getX(), telefyYPos, telefyBounds.getWidth(), telefySliderHeight);

    telefyActivateButton.setBounds(
        telefyBounds.getX(),   // colado à direita
        telefyBounds.getY(),             // um pouquinho abaixo da borda superior
        btnSize,
        btnSize
    );    

    telefyModeBox.setBounds(
        telefyBounds.getRight() - modeBoxWidth,
        telefyBounds.getY(),
        modeBoxWidth,
        modeBoxHeight
    );

    // =========================================================================
    // === 4. ÁREA DO EQUALIZADOR E DIVISÃO (ALTERADO) ===
    // =========================================================================

    // [ALTERADO/COMENTADO]: Cria uma cópia da área para fazer a divisão vertical destrutiva.
    auto layoutArea = mainPanelArea.toNearestInt().reduced(10);

    // Altura reservada para a fileira de filtros na base (aprox. 15% do espaço vertical)
    int filterHeight = layoutArea.getHeight() / 7;

    // 4a. Área para as 4 Bandas (TOPO)
    // [ALTERADO]: Usa removeFromTop() na CÓPIA para definir a área das bandas.
    auto bandsArea = layoutArea.removeFromTop(layoutArea.getHeight() - filterHeight);

    // 4b. Área para os Filtros HPF/LPF (BASE)
    auto filtersArea = layoutArea; // O que sobrou na cópia é a área dos filtros

    // === 5. CONFIGURAR FLEXBOX PARA AS 4 BANDAS ===
    mainEqFlex.flexDirection = juce::FlexBox::Direction::row;
    mainEqFlex.justifyContent = juce::FlexBox::JustifyContent::spaceBetween;
    mainEqFlex.alignItems = juce::FlexBox::AlignItems::stretch;
    mainEqFlex.items.clear();
    mainEqFlex.items.add(juce::FlexItem(lowBandContainer).withFlex(1).withMargin(5));
    mainEqFlex.items.add(juce::FlexItem(lowMidBandContainer).withFlex(1).withMargin(5));
    mainEqFlex.items.add(juce::FlexItem(highMidBandContainer).withFlex(1).withMargin(5));
    mainEqFlex.items.add(juce::FlexItem(highBandContainer).withFlex(1).withMargin(5));

    // [ALTERADO]: FlexBox performa o layout APENAS na área superior (bandsArea)
    mainEqFlex.performLayout(bandsArea.toFloat());

    // =========================================================================
    // === 6. CALCULAR TAMANHOS DOS KNOBS ===
    // =========================================================================
    float eqWidth = static_cast<float>(bandsArea.getWidth());   // Usa a largura da bandsArea
    float eqHeight = static_cast<float>(bandsArea.getHeight()); // Usa a altura da bandsArea

    // 1. Cálculos usando float
    float bandWidthFloat = eqWidth / 3.6f;
    float knobSizeFloat = juce::jmin(bandWidthFloat - 15.0f, eqHeight / 4.f);
    float gainKnobSizeFloat = knobSizeFloat * 1.10f;
    float qKnobSizeFloat = knobSizeFloat * 0.85f;

    // 2. Conversão final para int
    int knobSize = juce::roundToInt(knobSizeFloat);
    int gainKnobSize = juce::roundToInt(gainKnobSizeFloat);
    int qKnobSize = juce::roundToInt(qKnobSizeFloat);
    int spacing = juce::roundToInt(knobSizeFloat / 6.0f);
    int toggleSize = juce::roundToInt(knobSizeFloat * 0.55f);
    // Espaçamento menor para puxar o toggle para cima
    int toggleSpacing = juce::roundToInt(spacing * 0.5f);

    // [NOVO]: Tamanho menor para os filtros HPF/LPF (70%)
    int filterKnobSize = juce::roundToInt(knobSizeFloat * 0.7f);

    // === 7. FUNÇÃO AUXILIAR PARA POSICIONAR ===
    auto positionKnobsFromTop = [&](juce::Component& container,
        juce::Component* knobTop,
        juce::Component* knobMiddle,
        juce::Component* knobBottom = nullptr)
        {
            container.setPaintingIsUnclipped(true);
            auto bounds = container.getBounds().reduced(5);
            int centerX = bounds.getCentreX();
            int yPos = bounds.getY() + 5;

            auto getCustom = [](juce::Component* c) { return static_cast<CustomSlider*>(c); };

            if (knobTop)
            {
                // Casting direto para CustomSlider para acessar o paramType
                if (auto* cs = static_cast<CustomSlider*>(knobTop))
                {
                    if (knobTop == &lowGainSlider || knobTop == &lowMidGainSlider ||
                        knobTop == &highMidGainSlider || knobTop == &highGainSlider)
                        cs->paramType = CustomSlider::Gain;
                    else if (knobTop == &hpfSlider)
                        cs->paramType = CustomSlider::Frequency;
                }

                int currentKnobSize = (knobTop == &lowGainSlider || knobTop == &lowMidGainSlider ||
                    knobTop == &highMidGainSlider || knobTop == &highGainSlider)
                    ? gainKnobSize : knobSize;

                knobTop->setBounds(centerX - currentKnobSize / 2, yPos, currentKnobSize, currentKnobSize);
                yPos += (currentKnobSize + spacing);
            }

            // 2. Knob do MEIO (Frequency)
            if (knobMiddle)
            {
                if (auto* cs = static_cast<CustomSlider*>(knobMiddle))
                    cs->paramType = CustomSlider::Frequency;

                knobMiddle->setBounds(centerX - knobSize / 2, yPos, knobSize, knobSize);
                yPos += (knobSize + spacing);
            }

            // 3. Knob da BASE (Q, LPF ou SHELF/BELL)
            if (knobBottom)
            {
                int finalKnobSize = knobSize;
                int finalY = yPos;

                if (auto* cs = static_cast<CustomSlider*>(knobBottom))
                {
                    if (knobBottom == &lowMidQSlider || knobBottom == &highMidQSlider)
                        cs->paramType = CustomSlider::QFactor;
                    else if (knobBottom == &lpfSlider)
                        cs->paramType = CustomSlider::Frequency;
                }

                if (knobBottom == &lowShelfBellButton || knobBottom == &highShelfBellButton)
                {
                    finalKnobSize = toggleSize;
                    finalY = yPos - spacing + toggleSpacing;
                }
                else if (knobBottom == &lowMidQSlider || knobBottom == &highMidQSlider)
                {
                    finalKnobSize = qKnobSize;
                }

                knobBottom->setBounds(centerX - finalKnobSize / 2, finalY, finalKnobSize, finalKnobSize);
            }
        };

    // === 8. APLICAR POSICIONAMENTO: BANDA LOW (Gain, Freq) ===
    positionKnobsFromTop(lowBandContainer, &lowGainSlider, &lowFreqSlider, &lowShelfBellButton);

    // === 9. APLICAR POSICIONAMENTO: BANDA LOW-MID (Gain, Freq, Q) ===
    positionKnobsFromTop(lowMidBandContainer, &lowMidGainSlider, &lowMidFreqSlider, &lowMidQSlider);

    // === 10. APLICAR POSICIONAMENTO: BANDA HIGH-MID (Gain, Freq, Q) ===
    positionKnobsFromTop(highMidBandContainer, &highMidGainSlider, &highMidFreqSlider, &highMidQSlider);

    // === 11. APLICAR POSICIONAMENTO: BANDA HIGH (LPF, Gain, Freq) ===
    positionKnobsFromTop(highBandContainer, &highGainSlider, &highFreqSlider, &highShelfBellButton);

    // =========================================================================

    // === 12. POSICIONAMENTO DOS FILTROS HPF/LPF (NOVO) ===

    // =========================================================================
// Tamanho menor para os toggles de ativação
    int activateToggleSize = juce::roundToInt(static_cast<float>(filterKnobSize) * 0.5f);

    // Distância de deslocamento Y (comum a ambos os toggles)
    // Margem Horizontal (usamos 5% do tamanho do knob, que era o 0.1 do seu *2.1)
    int xMargin = juce::roundToInt(static_cast<float>(filterKnobSize) * 0.05f);

    // Posição Y comum para os filtros (centralizados verticalmente na filtersArea)
    int filterYCenter = filtersArea.getCentreY();
    int filterTopY = filterYCenter - filterKnobSize / 2;

    // --- HPF SLIDER ---
    // Alinhamento com a banda LOW (extrema esquerda)
    auto lowX = lowBandContainer.getBounds().getCentreX();
    hpfSlider.setBounds(
        lowX - filterKnobSize / 2,
        filterTopY,
        filterKnobSize,
        filterKnobSize
    );



    // --- HPF Activate Button (Diagonal Direita Inferior - POSIÇÃO EXATA) ---
    // X: Começa na borda direita do slider e adiciona a margem.
    hpfActivateButton.setBounds(
        hpfSlider.getRight() + xMargin, // Borda direita + margem, depois subtrai 1/2 do tamanho do toggle para alinhamento
        hpfSlider.getBottom() - activateToggleSize,
        activateToggleSize,
        activateToggleSize
    );



    // --- LPF SLIDER (Inalterado) ---
    // Alinhamento com a banda HIGH (extrema direita)
    auto highX = highBandContainer.getBounds().getCentreX();
    lpfSlider.setBounds(
        highX - filterKnobSize / 2,
        filterTopY,
        filterKnobSize,
        filterKnobSize);



        // --- LPF Activate Button (Diagonal Esquerda Inferior - POSIÇÃO EXATA) ---

        // X: Começa na borda esquerda do slider e subtrai a margem.
        lpfActivateButton.setBounds(
        lpfSlider.getRight() - filterKnobSize - activateToggleSize - xMargin, // Borda esquerda - margem, depois subtrai 1/2 do tamanho do toggle para alinhamento
        lpfSlider.getBottom() - activateToggleSize,
        activateToggleSize,
        activateToggleSize);



    int comboHeight = juce::roundToInt(static_cast<float>(knobSize) * 0.3f);
    int comboWidth = juce::roundToInt(static_cast<float>(knobSize) * 0.4f);
    spacing = 3;      // Espaço entre o topo do toggle e o fundo da combo

    // --- HPF Slope Box ---

    // Posição X: Centralizada com o HPF Slider (não o toggle) para melhor alinhamento
    // O toggle é pequeno e deslocado, o Slider é a âncora visual.
    juce::Rectangle<int> hpfSliderBounds = hpfSlider.getBounds();
    int hpfComboX = hpfSliderBounds.getRight() - filterKnobSize - comboWidth / 2;

    // Posição Y: Abaixo do topo da área de filtros, mas ligeiramente acima do slider.
    // Usamos filterTopY (o topo do slider) e subtraímos a altura da combo + spacing.
    int hpfComboY = filterTopY - comboHeight;

    hpfSlopeBox.setBounds(hpfComboX, hpfComboY, comboWidth, comboHeight);

    // --- LPF Slope Box ---
    // Posição X: Centralizada com o LPF Slider
    juce::Rectangle<int> lpfSliderBounds = lpfSlider.getBounds();
    int lpfComboX = lpfSliderBounds.getRight() - comboWidth / 2;


    // Posição Y: Igual ao HPF para alinhamento horizontal
    int lpfComboY = filterTopY - comboHeight;

    lpfSlopeBox.setBounds(lpfComboX, lpfComboY, comboWidth, comboHeight);







    // --- 7. CÁLCULO E ARMAZENAMENTO DOS ELEMENTOS DE FORA DO PAINEL (usando bounds) ---

// Base: 55% da altura
    float gainAreaHeight = bounds.getHeight() * 0.55f;
    float gainAreaY = mainPanelArea.getBottom() - gainAreaHeight;



    // Base: 12% de margem (mesma do painel)
    float leftMargin = bounds.getWidth() * 0.0f;

    // Base: tamanho do toggle × multiplicador
    float gainSliderWidth = static_cast<float>(activateToggleSize) * 1.2f;
   
    //float meterWidth = static_cast<float>(activateToggleSize) * 1.5f;



    // Posições
    float inputSliderX = leftMargin;
    float inputMetersX = inputSliderX + gainSliderWidth;
    float outputSliderX = bounds.getWidth() - leftMargin - gainSliderWidth;

    // Posicionar
    inputGainSlider.setBounds(inputSliderX, gainAreaY, gainSliderWidth, gainAreaHeight);
    outputGainSlider.setBounds(outputSliderX, gainAreaY, gainSliderWidth, gainAreaHeight);

    // Defina a área (Exemplo: na borda inferior do painel principal)
    float meterWidth = gainSliderWidth / 3;
    float meterHeight = gainAreaHeight - gainAreaHeight*0.07f;
	float meterPadding = 18.0f; // Espaçamento entre o slider e o medidor 
    
    // --- Medidor de Entrada (Input Meter) ---
    inputMeterL.setBounds(
        inputSliderX + meterPadding + meterWidth, // Perto da borda esquerda do painel principal
        gainAreaY + 12.0f, // Alinhado ao topo do painel
        meterWidth,
        meterHeight
    );

    inputMeterR.setBounds(
        inputSliderX + meterPadding + meterWidth + inputMeterL.getWidth(), // Perto da borda esquerda do painel principal
        gainAreaY + 12.0f, // Alinhado ao topo do painel
        meterWidth,
        meterHeight
    );

    // --- Medidor de Saída (Output Meter) ---
    outputMeterL.setBounds(
        outputSliderX - meterPadding + meterWidth - outputMeterR.getWidth(), // Perto da borda esquerda do painel principal
        gainAreaY + 12.0f, // Alinhado ao topo do painel
        meterWidth,
        meterHeight
    );

    outputMeterR.setBounds(
        outputSliderX - meterPadding + meterWidth, // Perto da borda esquerda do painel principal
        gainAreaY + 12.0f, // Alinhado ao topo do painel
        meterWidth,
        meterHeight
    );




    // LOGO (Mantém a posição relativa ao bounds total)

    if (logoImage)
    {
        float baseSize = bounds.getHeight() * 0.12f;
        float logoW = baseSize * 1.0f;
        float logoH = baseSize;
        float logoY = bounds.getHeight() * 0.85f;
        float logoX = (bounds.getWidth() - logoW) / 2.0f;
        logoImage->setBounds((int)logoX, (int)logoY, (int)logoW, (int)logoH);
    }



    // PLUGIN SYMBOL (Mantém a posição relativa ao bounds total)
    float symbolW = bounds.getWidth() * 0.30f;
    float symbolH = bounds.getHeight() * 0.2f;
    float symbolY = bounds.getHeight() * 0.65f;
    float symbolX = (bounds.getWidth() - symbolW) / 2.0f;
    symbolArea.setBounds((int)symbolX, (int)symbolY, (int)symbolW, (int)symbolH);

    // BRAND TITTLE (Mantém a posição relativa ao bounds total)

    float brandW = bounds.getWidth() * 0.30f;
    float brandH = bounds.getHeight() * 0.02f;
    float brandY = bounds.getHeight() * 0.96f;
    float brandX = (bounds.getWidth() - brandW) / 2.0f;
    brandArea.setBounds((int)brandX, (int)brandY, (int)brandW, (int)brandH);

    // PLUGIN TITTLE (Mantém a posição relativa ao bounds total)
    float titleW = bounds.getWidth() * 0.30f;
    float titleH = bounds.getHeight() * 0.1f;
    float titleY = bounds.getHeight() * 0.575f;
    float titleX = (bounds.getWidth() - titleW) / 2.0f;
    titleArea.setBounds((int)titleX, (int)titleY, (int)titleW, (int)titleH);

    const float symbolSize = 13.0f;
    const float paddingAbove = 5.0f;
    const float paddingSide = 5.0f;

    // --- A. Filtro HPF (Topo Esquerdo) ---
    auto hpfLedArea = hpfActivateButton.getBounds().toFloat();
    hpfSymbolBounds.setBounds(
        hpfLedArea.getCentreX() - (symbolSize / 2.0f), // Centralizado em X
        filtersArea.getY() + paddingAbove,             // Acima da área de filtros
        symbolSize, symbolSize);

    // --- B. Filtro LPF (Topo Direito) ---
    auto lpfLedArea = lpfActivateButton.getBounds().toFloat();
    lpfSymbolBounds.setBounds(
        lpfLedArea.getCentreX() - (symbolSize / 2.0f), // Centralizado em X
        filtersArea.getY() + paddingAbove,             // Alinhado com o HPF
        symbolSize, symbolSize);

    // --- C. Drive (Lateral Esquerdo) ---
    auto driveLedArea = driveActivateButton.getBounds().toFloat();
    driveSymbolBounds.setBounds(
        driveLedArea.getX() - symbolSize - paddingSide, // À esquerda do LED
        driveLedArea.getCentreY() - (symbolSize / 2.0f), // Centralizado em Y
        symbolSize, symbolSize);

    // --- D. Telefy (Lateral Direito) ---
    auto telefyLedArea = telefyActivateButton.getBounds().toFloat();
    telefySymbolBounds.setBounds(
        telefyLedArea.getRight() + paddingSide,          // À direita do LED
        telefyLedArea.getCentreY() - (symbolSize / 2.0f), // Centralizado em Y
        symbolSize, symbolSize);

    // OVERLAY
    if (aboutOverlay)
    {
        aboutOverlay->setBounds(getLocalBounds());
    }
}

std::vector<juce::Component*> TeLeQAudioProcessorEditor::getComps()

{
    return
    {
        &lowShelfBellButton, &highShelfBellButton,
        &hpfActivateButton,  &lpfActivateButton,
		&driveActivateButton,& telefyActivateButton,
        &hpfSlider,
        &lpfSlider,
        &lowFreqSlider, &lowGainSlider,
        &lowMidFreqSlider, &lowMidGainSlider, &lowMidQSlider,
        &highMidFreqSlider, &highMidGainSlider, &highMidQSlider,
        &highFreqSlider, &highGainSlider,
        &telefyToneSlider, &telefyIntensitySlider,
        &inputGainSlider, &outputGainSlider,
        &driveGainSlider, &driveMixSlider, &telefyAmountSlider

    };
}



void TeLeQAudioProcessorEditor::buttonClicked(juce::Button* button)
{
    if (button == &lowShelfBellButton)
    {
        // O estado interno do botão (isDown) já foi alternado pelo setClickingTogglesState(true).
        if (lowShelfBellButton.getToggleState())
        {
            // O botão está ativado/apertado (DOWN) -> Modo BELL
            lowShelfBellButton.setButtonText("BELL");
        }
        else
        {
            // O botão está desativado/solto (UP) -> Modo SHELF
            lowShelfBellButton.setButtonText("SHELF");
        }
    }
    else if (button == &highShelfBellButton)
    {
        if (highShelfBellButton.getToggleState())
        {
            // O botão está ativado/apertado (DOWN) -> Modo BELL
            highShelfBellButton.setButtonText("BELL");
        }
        else
        {
            // O botão está desativado/solto (UP) -> Modo SHELF
            highShelfBellButton.setButtonText("SHELF");
        }
    }

    // NOTA: Seus Attachments (lowShelfBellAttachment e highShelfBellAttachment)
    // cuidam de enviar o valor binário (0 ou 1) para o AudioProcessor com base no estado de toggle.
}


/* 
    if (aboutButton && button == aboutButton.get())
    {
        // SE O OVERLAY ESTIVER FECHADO (nullptr), ABRE:
        if (aboutOverlay == nullptr)
        {
            aboutOverlay = std::make_unique<aboutPanel>();
            addAndMakeVisible(*aboutOverlay);
            resized(); // Garante o posicionamento e o redesenho
        }
        else
        {
            // SE O OVERLAY ESTIVER ABERTO, FECHA (clique no logo
        } */