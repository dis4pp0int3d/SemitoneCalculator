/*
  ==============================================================================

    Semitone Calculator - editor.
    Two themes: Default (graphite) and Liquid Glass (dark, frosted glass panels over soft
    coloured light). The theme switch and the plug-in info live behind the "?" button.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

#include <functional>

namespace sc
{
    // Colours of one theme.
    struct Palette
    {
        bool glass;
        juce::Colour bgTop, bgBottom;
        juce::Colour panel, panelBorder;      // cards (glass: tint over the frosted background)
        juce::Colour field, fieldBorder;      // combo boxes, segmented control
        juce::Colour text, textDim, textFaint;
        juce::Colour accent, accentEnd, accentText;
        juce::Colour menu;                    // popup menus and the info window
    };

    const Palette& palette(bool glass);

    // Segoe UI on Windows (the system's own font), the system sans elsewhere.
    juce::Font font(float height, const juce::String& style = "Regular");

    // Combo boxes, popup menus and the small buttons in the plug-in's look.
    class LookAndFeel : public juce::LookAndFeel_V4
    {
    public:
        void setGlass(bool g);
        bool isGlass() const { return glass; }

        void drawComboBox(juce::Graphics&, int width, int height, bool isButtonDown,
                          int buttonX, int buttonY, int buttonW, int buttonH, juce::ComboBox&) override;
        juce::Font getComboBoxFont(juce::ComboBox&) override { return font(15.0f, "Semibold"); }
        void positionComboBoxText(juce::ComboBox&, juce::Label&) override;
        juce::PopupMenu::Options getOptionsForComboBoxPopupMenu(juce::ComboBox&, juce::Label&) override;

        void drawPopupMenuBackground(juce::Graphics&, int width, int height) override;
        void drawPopupMenuItem(juce::Graphics&, const juce::Rectangle<int>& area, bool isSeparator, bool isActive,
                               bool isHighlighted, bool isTicked, bool hasSubMenu, const juce::String& text,
                               const juce::String& shortcutKeyText, const juce::Drawable* icon, const juce::Colour* textColour) override;
        juce::Font getPopupMenuFont() override { return font(14.0f); }
        void getIdealPopupMenuItemSize(const juce::String& text, bool isSeparator, int standardMenuItemHeight,
                                       int& idealWidth, int& idealHeight) override;
        int getPopupMenuBorderSize() override { return 6; }

        void drawButtonBackground(juce::Graphics&, juce::Button&, const juce::Colour&, bool highlighted, bool down) override;
        void drawButtonText(juce::Graphics&, juce::TextButton&, bool highlighted, bool down) override;
        juce::Font getTextButtonFont(juce::TextButton&, int) override { return font(14.0f, "Semibold"); }

    private:
        bool glass = false;
    };

    // Major | Minor
    class Segmented : public juce::Component
    {
    public:
        Segmented(juce::StringArray options);
        int getSelected() const { return selected; }
        void setSelected(int index, bool notify);
        std::function<void()> onChange;

        void paint(juce::Graphics&) override;
        void mouseDown(const juce::MouseEvent&) override;
        void mouseMove(const juce::MouseEvent&) override;
        void mouseExit(const juce::MouseEvent&) override;

    private:
        juce::StringArray options;
        int selected = 0, hover = -1;
    };

    // A pill switch with its label: "Audio pitch".
    class Switch : public juce::Button
    {
    public:
        Switch() : juce::Button("Audio pitch") { setClickingTogglesState(true); }
        void paintButton(juce::Graphics&, bool highlighted, bool down) override;
    };
}

//==============================================================================
class SemitoneCalculatorAudioProcessorEditor  : public juce::AudioProcessorEditor,
                                                private juce::Timer
{
public:
    SemitoneCalculatorAudioProcessorEditor (SemitoneCalculatorAudioProcessor&);
    ~SemitoneCalculatorAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

    // The frosted glass: a card that shows the blurred background through it.
    void paintPanel(juce::Graphics&, juce::Rectangle<float> area, float radius) const;

private:
    class InfoPopup;

    juce::ComboBox fromNoteBox;
    juce::ComboBox toNoteBox;
    sc::Segmented fromModeBox { { "Major", "Minor" } };
    sc::Segmented toModeBox { { "Major", "Minor" } };
    sc::Switch pitchToggleButton;
    juce::TextButton helpButton;

    juce::StringArray noteNames {
        "C", "C# / Db", "D", "D# / Eb", "E", "F",
        "F# / Gb", "G", "G# / Ab", "A", "A# / Bb", "B" };

    void updateCalculation();
    void applyTheme();
    void setTheme(bool glass);
    void showAboutPopup();
    void timerCallback() override;
    void renderBackground();

    bool useLiquidGlassTheme = false;
    int semitones = 0;
    float pop = 1.0f;                       // the result's little bounce when it changes
    juce::Rectangle<float> leftPanel, resultPanel;
    juce::Image background, frosted;        // the window background and its blurred copy

    sc::LookAndFeel lookAndFeel;
    std::unique_ptr<InfoPopup> infoPopup;

    SemitoneCalculatorAudioProcessor& audioProcessor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SemitoneCalculatorAudioProcessorEditor)
};
