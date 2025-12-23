// ================= CustomSlider.cpp (Corrigido) =================

#include "CustomSlider.h"



CustomSlider::CustomSlider()
{
	// Inicia com textbox oculto
	// Usar TextBoxHidden é uma opção mais clara do que NoTextBox para alternância.
	setPaintingIsUnclipped(true);
	setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
}

//void CustomSlider::paintOverChildren(juce::Graphics& g)
//{
//	// Opcional: Se houver necessidade de desenhar algo sobre o textbox, faça aqui.
//	// Garante que o textbox (um filho do slider) seja desenhado
//	juce::Slider::paintOverChildren(g);
//}

void CustomSlider::mouseDown(const juce::MouseEvent& event)
{
	isBeingDragged = true;

	toFront(false);

	// Mostra o textbox quando clica
	setTextBoxStyle(juce::Slider::TextBoxBelow, true, 50, 18);
	setTextValueSuffix(" dB");

	// Chama a implementacao da classe base
	juce::Slider::mouseDown(event);
}

void CustomSlider::mouseUp(const juce::MouseEvent& event)
{
	isBeingDragged = false;

	// Esconde o textbox quando solta, usando o estilo de ocultacao
	setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);

	// Chama a implementacao da classe base
	juce::Slider::mouseUp(event);
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

	case Percentage:
		// Se o parâmetro for 0.0 a 1.0, multiplicamos por 100
		return juce::String(value * 100.0, 0) + " %";

	default:
		return juce::String(value, 2);
	}
}

void CustomSlider::focusLost(juce::Component::FocusChangeType cause)
{
	// Verifica se o componente perdeu o foco E NAO está sendo arrastado.
	// Se ainda estiver sendo arrastado, o TextBox deve permanecer visível.
	if (!isBeingDragged)
	{
		// Se perder o foco e nao estiver sendo arrastado, esconde
		setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
	}

	// Chama a implementacao da classe base
	juce::Slider::focusLost(cause);
}