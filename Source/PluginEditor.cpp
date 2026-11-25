/*
  ==============================================================================

    Semitone Calculator - editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace
{
constexpr auto kDeveloperName = "lovesakiko";
constexpr auto kSupportEmail  = "metarealization.depersonal@gmail.com";
constexpr auto kBrand         = "metarealization\\>";

juce::String shortName(const juce::String& note) { return note.upToFirstOccurrenceOf(" ", false, false); }
juce::String minusSign() { return juce::String(juce::CharPointer_UTF8("\xe2\x88\x92")); }
}

//==============================================================================
namespace sc
{
const Palette& palette(bool glass)
{
    using C = juce::Colour;
    static const Palette graphite {
        false,
        C(0xff17171a), C(0xff0f0f11),
        C(0xff1d1d21), C(0xff2b2b31),
        C(0xff26262b), C(0xff35353c),
        C(0xfff3f3f5), C(0xffa2a2ab), C(0xff6b6b74),
        C(0xfff3f3f5), C(0xffd6d6db), C(0xff121214),
        C(0xff1f1f23) };
    static const Palette liquid {
        true,
        C(0xff0a0d1c), C(0xff05060d),
        C(0x14ffffff), C(0x40ffffff),
        C(0x12ffffff), C(0x26ffffff),
        C(0xfff4f7ff), C(0xffb4bdd6), C(0xff7a84a3),
        C(0xff8fe9ff), C(0xffb8a4ff), C(0xff070b18),
        C(0xf2121828) };
    return glass ? liquid : graphite;
}

juce::Font font(float height, const juce::String& style)
{
   #if JUCE_WINDOWS
    return juce::Font(juce::FontOptions("Segoe UI", style, height));
   #else
    return juce::Font(juce::FontOptions(height, style == "Semibold" || style == "Bold" ? juce::Font::bold : juce::Font::plain));
   #endif
}

// A field: rounded, with a light top edge on glass.
static bool glassFor(juce::Component& c)
{
    auto* lf = dynamic_cast<LookAndFeel*>(&c.getLookAndFeel());
    return lf != nullptr && lf->isGlass();
}

static void paintField(juce::Graphics& g, juce::Rectangle<float> r, float radius, bool hover, bool glass)
{
    const auto& c = palette(glass);
    g.setColour(hover ? c.field.brighter(0.08f).withMultipliedAlpha(glass ? 1.4f : 1.0f) : c.field);
    g.fillRoundedRectangle(r, radius);
    if (glass)
    {
        g.setGradientFill(juce::ColourGradient(juce::Colours::white.withAlpha(0.10f), r.getX(), r.getY(),
                                               juce::Colours::white.withAlpha(0.0f), r.getX(), r.getCentreY(), false));
        g.fillRoundedRectangle(r, radius);
    }
    g.setColour(c.fieldBorder);
    g.drawRoundedRectangle(r.reduced(0.5f), radius, 1.0f);
}

void LookAndFeel::setGlass(bool g)
{
    glass = g;
    const auto& p = palette(g);
    setColour(juce::ComboBox::textColourId, p.text);
    setColour(juce::PopupMenu::backgroundColourId, p.menu);
    setColour(juce::PopupMenu::textColourId, p.text);
    setColour(juce::PopupMenu::highlightedBackgroundColourId, p.accent.withAlpha(0.16f));
    setColour(juce::PopupMenu::highlightedTextColourId, p.text);
    setColour(juce::TextButton::textColourOffId, p.text);
    setColour(juce::TextButton::textColourOnId, p.text);
    setColour(juce::Label::textColourId, p.text);
}

void LookAndFeel::drawComboBox(juce::Graphics& g, int width, int height, bool, int, int, int, int, juce::ComboBox& box)
{
    const auto r = juce::Rectangle<float>(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height)).reduced(0.5f);
    paintField(g, r, 10.0f, box.isMouseOver(true), glass);
    const auto& p = palette(glass);
    juce::Path chevron;
    const float cx = r.getRight() - 18.0f, cy = r.getCentreY();
    chevron.startNewSubPath(cx - 4.5f, cy - 2.0f);
    chevron.lineTo(cx, cy + 2.5f);
    chevron.lineTo(cx + 4.5f, cy - 2.0f);
    g.setColour(p.textDim);
    g.strokePath(chevron, juce::PathStrokeType(1.6f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
}

void LookAndFeel::positionComboBoxText(juce::ComboBox& box, juce::Label& label)
{
    label.setBounds(4, 0, box.getWidth() - 30, box.getHeight());
    label.setFont(getComboBoxFont(box));
    label.setColour(juce::Label::textColourId, palette(glass).text);
}

juce::PopupMenu::Options LookAndFeel::getOptionsForComboBoxPopupMenu(juce::ComboBox& box, juce::Label& label)
{
    return LookAndFeel_V4::getOptionsForComboBoxPopupMenu(box, label).withStandardItemHeight(30);
}

void LookAndFeel::drawPopupMenuBackground(juce::Graphics& g, int width, int height)
{
    const auto& p = palette(glass);
    const auto r = juce::Rectangle<float>(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height));
    g.fillAll(p.menu);
    if (glass)
    {
        g.setGradientFill(juce::ColourGradient(juce::Colours::white.withAlpha(0.07f), 0.0f, 0.0f,
                                               juce::Colours::white.withAlpha(0.0f), 0.0f, 60.0f, false));
        g.fillRect(r);
    }
    g.setColour(p.fieldBorder);
    g.drawRect(r, 1.0f);
}

void LookAndFeel::drawPopupMenuItem(juce::Graphics& g, const juce::Rectangle<int>& area, bool isSeparator, bool isActive,
                                    bool isHighlighted, bool isTicked, bool, const juce::String& text,
                                    const juce::String&, const juce::Drawable*, const juce::Colour*)
{
    const auto& p = palette(glass);
    if (isSeparator)
    {
        g.setColour(p.fieldBorder);
        g.fillRect(area.reduced(10, 0).withHeight(1).withY(area.getCentreY()));
        return;
    }
    auto r = area.toFloat().reduced(4.0f, 1.0f);
    if (isHighlighted && isActive)
    {
        g.setColour(p.accent.withAlpha(glass ? 0.16f : 0.10f));
        g.fillRoundedRectangle(r, 7.0f);
    }
    if (isTicked)
    {
        g.setColour(p.accent);
        g.fillEllipse(juce::Rectangle<float>(6.0f, 6.0f).withCentre({ r.getX() + 12.0f, r.getCentreY() }));
    }
    g.setColour(isActive ? (isTicked ? p.text : p.text.withAlpha(0.85f)) : p.textFaint);
    g.setFont(isTicked ? font(14.0f, "Semibold") : font(14.0f));
    g.drawText(text, r.withTrimmedLeft(24.0f), juce::Justification::centredLeft, true);
}

void LookAndFeel::getIdealPopupMenuItemSize(const juce::String& text, bool isSeparator, int standardMenuItemHeight,
                                            int& idealWidth, int& idealHeight)
{
    if (isSeparator) { idealWidth = 50; idealHeight = 9; return; }
    idealHeight = standardMenuItemHeight > 0 ? standardMenuItemHeight : 30;
    idealWidth = juce::roundToInt(juce::GlyphArrangement::getStringWidth(font(14.0f, "Semibold"), text)) + 44;
}

void LookAndFeel::drawButtonBackground(juce::Graphics& g, juce::Button& b, const juce::Colour&, bool highlighted, bool down)
{
    const auto r = b.getLocalBounds().toFloat().reduced(0.5f);
    const float radius = b.getProperties().getWithDefault("round", false) ? r.getHeight() / 2.0f : 10.0f;
    paintField(g, r, radius, highlighted || down, glass);
}

void LookAndFeel::drawButtonText(juce::Graphics& g, juce::TextButton& b, bool highlighted, bool)
{
    const auto& p = palette(glass);
    g.setColour(highlighted ? p.text : p.textDim);
    g.setFont(getTextButtonFont(b, b.getHeight()));
    g.drawText(b.getButtonText(), b.getLocalBounds(), juce::Justification::centred, false);
}

//==============================================================================
Segmented::Segmented(juce::StringArray o) : options(std::move(o)) {}

void Segmented::setSelected(int index, bool notify)
{
    index = juce::jlimit(0, options.size() - 1, index);
    if (index == selected) return;
    selected = index;
    repaint();
    if (notify && onChange) onChange();
}

void Segmented::paint(juce::Graphics& g)
{
    const bool glass = glassFor(*this);
    const auto& p = palette(glass);
    const auto r = getLocalBounds().toFloat().reduced(0.5f);
    paintField(g, r, 10.0f, false, glass);
    const float w = (r.getWidth() - 6.0f) / static_cast<float>(options.size());
    for (int i = 0; i < options.size(); ++i)
    {
        const auto cell = juce::Rectangle<float>(r.getX() + 3.0f + w * static_cast<float>(i), r.getY() + 3.0f, w, r.getHeight() - 6.0f);
        if (i == selected)
        {
            g.setGradientFill(juce::ColourGradient(p.accent, cell.getX(), cell.getY(), p.accentEnd, cell.getRight(), cell.getBottom(), false));
            g.fillRoundedRectangle(cell, 8.0f);
            if (glass)
            {
                g.setColour(juce::Colours::white.withAlpha(0.45f));
                g.drawRoundedRectangle(cell.reduced(0.5f), 8.0f, 0.8f);
            }
        }
        else if (i == hover)
        {
            g.setColour(p.text.withAlpha(0.06f));
            g.fillRoundedRectangle(cell, 8.0f);
        }
        g.setColour(i == selected ? p.accentText : (i == hover ? p.text : p.textDim));
        g.setFont(font(14.0f, "Semibold"));
        g.drawText(options[i], cell, juce::Justification::centred, false);
    }
}

void Segmented::mouseDown(const juce::MouseEvent& e)
{
    setSelected(juce::jlimit(0, options.size() - 1, e.x * options.size() / juce::jmax(1, getWidth())), true);
}

void Segmented::mouseMove(const juce::MouseEvent& e)
{
    const int h = juce::jlimit(0, options.size() - 1, e.x * options.size() / juce::jmax(1, getWidth()));
    if (h != hover) { hover = h; repaint(); }
}

void Segmented::mouseExit(const juce::MouseEvent&) { hover = -1; repaint(); }

//==============================================================================
void Switch::paintButton(juce::Graphics& g, bool highlighted, bool)
{
    const bool glass = glassFor(*this);
    const auto& p = palette(glass);
    const bool on = getToggleState();
    auto r = getLocalBounds().toFloat();

    auto track = r.removeFromLeft(46.0f).withSizeKeepingCentre(46.0f, 26.0f);
    if (on)
    {
        g.setGradientFill(juce::ColourGradient(p.accent, track.getX(), track.getY(), p.accentEnd, track.getRight(), track.getBottom(), false));
        g.fillRoundedRectangle(track, 13.0f);
    }
    else paintField(g, track, 13.0f, highlighted, glass);
    const auto knob = juce::Rectangle<float>(20.0f, 20.0f).withCentre({ on ? track.getRight() - 13.0f : track.getX() + 13.0f, track.getCentreY() });
    juce::DropShadow(juce::Colours::black.withAlpha(0.35f), 6, { 0, 1 }).drawForRectangle(g, knob.toNearestInt());
    // white knob on the glass accent, dark knob on the white one
    g.setColour(on ? (glass ? juce::Colours::white : p.accentText) : p.textDim);
    g.fillEllipse(knob);

    r.removeFromLeft(12.0f);
    g.setColour(p.text);
    g.setFont(font(14.5f, "Semibold"));
    g.drawText("Audio pitch", r.removeFromTop(r.getHeight() / 2.0f + 2.0f), juce::Justification::bottomLeft, false);
    g.setColour(on ? p.accent : p.textFaint);
    g.setFont(font(12.0f));
    g.drawText(on ? "On: the audio is shifted by the result" : "Off: the audio passes unchanged", r, juce::Justification::topLeft, false);
}
}

//==============================================================================
// The "?" window: about the plug-in, and the theme.
class SemitoneCalculatorAudioProcessorEditor::InfoPopup : public juce::Component, private juce::Timer
{
public:
    InfoPopup(SemitoneCalculatorAudioProcessorEditor& e) : editor(e)
    {
        setWantsKeyboardFocus(true);
        closeButton.setButtonText(juce::String(juce::CharPointer_UTF8("\xc3\x97")));
        closeButton.getProperties().set("round", true);
        closeButton.onClick = [this] { close(); };
        addAndMakeVisible(closeButton);
        setAlpha(0.0f);
        startTimerHz(60);
    }

    void close() { closing = true; startTimerHz(60); }

    void resized() override
    {
        card = juce::Rectangle<float>(400.0f, 258.0f).withCentre(getLocalBounds().toFloat().getCentre());
        closeButton.setBounds(card.toNearestInt().removeFromTop(48).removeFromRight(48).reduced(12));
        // the same rows as paint(): title, version, SUPPORT, e-mail, THEME, the two theme cards
        auto r = card.reduced(22.0f);
        r.removeFromTop(26.0f + 20.0f + 10.0f + 16.0f);
        emailArea = r.removeFromTop(22.0f);
        r.removeFromTop(12.0f + 16.0f + 6.0f);
        auto row = r.removeFromTop(64.0f);
        themeCards[0] = row.removeFromLeft((row.getWidth() - 10.0f) / 2.0f);
        row.removeFromLeft(10.0f);
        themeCards[1] = row;
    }

    void paint(juce::Graphics& g) override
    {
        const bool glass = editor.useLiquidGlassTheme;
        const auto& p = sc::palette(glass);
        g.fillAll(juce::Colours::black.withAlpha(0.45f));

        const auto box = card.withSizeKeepingCentre(card.getWidth() * (0.96f + 0.04f * alpha), card.getHeight() * (0.96f + 0.04f * alpha));
        juce::DropShadow(juce::Colours::black.withAlpha(0.45f), 28, { 0, 10 }).drawForRectangle(g, box.toNearestInt());
        if (glass) editor.paintPanel(g, box, 18.0f);
        g.setColour(glass ? p.menu.withAlpha(0.55f) : p.menu);
        g.fillRoundedRectangle(box, 18.0f);
        g.setColour(glass ? juce::Colours::white.withAlpha(0.22f) : p.panelBorder);
        g.drawRoundedRectangle(box.reduced(0.5f), 18.0f, 1.0f);

        auto r = card.reduced(22.0f);
        g.setColour(p.text);
        g.setFont(sc::font(19.0f, "Semibold"));
        g.drawText("Semitone Calculator", r.removeFromTop(26.0f), juce::Justification::centredLeft, false);
        g.setColour(p.textDim);
        g.setFont(sc::font(12.5f));
        g.drawText("Version " + juce::String(JucePlugin_VersionString) + "  " + juce::String(juce::CharPointer_UTF8("\xc2\xb7")) + "  by "
                       + juce::String(kDeveloperName) + "  " + juce::String(juce::CharPointer_UTF8("\xc2\xb7")) + "  " + kBrand,
                   r.removeFromTop(20.0f), juce::Justification::centredLeft, false);
        r.removeFromTop(10.0f);

        caption(g, "SUPPORT", r.removeFromTop(16.0f));
        g.setColour(emailHover ? p.accent : p.text);
        g.setFont(sc::font(14.0f, emailHover ? "Semibold" : "Regular"));
        g.drawText(kSupportEmail, emailArea, juce::Justification::centredLeft, false);
        r.removeFromTop(22.0f + 12.0f);

        caption(g, "THEME", r.removeFromTop(16.0f));
        for (int i = 0; i < 2; ++i) paintThemeCard(g, themeCards[i], i == 1, glass == (i == 1));
        g.setColour(p.textDim);
        g.setFont(sc::font(11.5f));
        g.drawText("SoundTouch (c) Olli Parviainen - LGPL 2.1+ (Licenses/COPYING.TXT)",
                   card.reduced(16.0f, 10.0f).removeFromBottom(18.0f), juce::Justification::centred, true);
    }

    void mouseMove(const juce::MouseEvent& e) override
    {
        const bool h = emailArea.withWidth(260.0f).contains(e.position);
        if (h != emailHover) { emailHover = h; setMouseCursor(h ? juce::MouseCursor::PointingHandCursor : juce::MouseCursor::NormalCursor); repaint(); }
    }

    void mouseUp(const juce::MouseEvent& e) override
    {
        if (!card.contains(e.position)) { close(); return; }
        for (int i = 0; i < 2; ++i)
            if (themeCards[i].contains(e.position)) { editor.setTheme(i == 1); repaint(); return; }
        if (emailArea.withWidth(260.0f).contains(e.position))
            juce::URL("mailto:" + juce::String(kSupportEmail)).launchInDefaultBrowser();
    }

    bool keyPressed(const juce::KeyPress& key) override
    {
        if (key == juce::KeyPress::escapeKey) { close(); return true; }
        return false;
    }

private:
    void caption(juce::Graphics& g, const juce::String& text, juce::Rectangle<float> r) const
    {
        g.setColour(sc::palette(editor.useLiquidGlassTheme).textFaint);
        g.setFont(sc::font(11.0f, "Semibold").withExtraKerningFactor(0.08f));
        g.drawText(text, r, juce::Justification::centredLeft, false);
    }

    // A small picture of the theme with its name.
    void paintThemeCard(juce::Graphics& g, juce::Rectangle<float> r, bool glass, bool active) const
    {
        const auto& t = sc::palette(glass);
        const auto& p = sc::palette(editor.useLiquidGlassTheme);
        juce::Path shape;
        shape.addRoundedRectangle(r, 12.0f);
        g.saveState();
        g.reduceClipRegion(shape);
        g.setGradientFill(juce::ColourGradient(t.bgTop, r.getX(), r.getY(), t.bgBottom, r.getRight(), r.getBottom(), false));
        g.fillRect(r);
        if (glass)
        {
            for (auto [c, x, y] : { std::tuple<juce::Colour, float, float> { juce::Colour(0xff6b5cff), 0.2f, 0.1f },
                                    { juce::Colour(0xff35d4ff), 0.85f, 0.9f } })
            {
                const juce::Point<float> centre(r.getX() + r.getWidth() * x, r.getY() + r.getHeight() * y);
                g.setGradientFill(juce::ColourGradient(c.withAlpha(0.55f), centre, c.withAlpha(0.0f), centre.translated(r.getHeight() * 0.9f, 0.0f), true));
                g.fillRect(r);
            }
        }
        const auto chip = juce::Rectangle<float>(r.getX() + 12.0f, r.getCentreY() - 6.0f, 46.0f, 12.0f);
        g.setGradientFill(juce::ColourGradient(t.accent, chip.getX(), chip.getY(), t.accentEnd, chip.getRight(), chip.getBottom(), false));
        g.fillRoundedRectangle(chip, 6.0f);
        g.restoreState();

        g.setColour(active ? p.accent : p.fieldBorder);
        g.drawRoundedRectangle(r.reduced(active ? 1.0f : 0.5f), 12.0f, active ? 2.0f : 1.0f);
        g.setColour(t.text);
        g.setFont(sc::font(13.5f, "Semibold"));
        g.drawText(glass ? "Liquid Glass" : "Default", r.withTrimmedLeft(70.0f), juce::Justification::centredLeft, false);
    }

    void timerCallback() override
    {
        alpha = juce::jlimit(0.0f, 1.0f, alpha + (closing ? -1.0f : 1.0f) / (60.0f * 0.14f));
        setAlpha(alpha);
        repaint();
        if (closing && alpha <= 0.0f)
        {
            stopTimer();
            juce::MessageManager::callAsync([safe = juce::Component::SafePointer<SemitoneCalculatorAudioProcessorEditor>(&editor)]
                                            { if (safe != nullptr) safe->infoPopup.reset(); });
        }
        else if (!closing && alpha >= 1.0f) stopTimer();
    }

    SemitoneCalculatorAudioProcessorEditor& editor;
    juce::TextButton closeButton;
    juce::Rectangle<float> card, emailArea;
    juce::Rectangle<float> themeCards[2];
    float alpha = 0.0f;
    bool closing = false, emailHover = false;
};

//==============================================================================
SemitoneCalculatorAudioProcessorEditor::SemitoneCalculatorAudioProcessorEditor (SemitoneCalculatorAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    useLiquidGlassTheme = audioProcessor.getUseLiquidGlassTheme();
    lookAndFeel.setGlass(useLiquidGlassTheme);
    setLookAndFeel(&lookAndFeel);

    for (auto* box : { &fromNoteBox, &toNoteBox })
    {
        box->addItemList(noteNames, 1);
        box->onChange = [this] { updateCalculation(); };
        addAndMakeVisible(box);
    }
    for (auto* seg : { &fromModeBox, &toModeBox })
    {
        seg->onChange = [this] { updateCalculation(); };
        addAndMakeVisible(seg);
    }

    // Restore UI state from the processor (so DAWs/hosts don't reset everything).
    fromNoteBox.setSelectedId(audioProcessor.fromNoteId.load(), juce::dontSendNotification);
    toNoteBox.setSelectedId(audioProcessor.toNoteId.load(), juce::dontSendNotification);
    fromModeBox.setSelected(audioProcessor.fromModeId.load() - 1, false);
    toModeBox.setSelected(audioProcessor.toModeId.load() - 1, false);

    pitchToggleButton.setToggleState(audioProcessor.isPitchingActive.load(), juce::dontSendNotification);
    pitchToggleButton.onClick = [this] { audioProcessor.isPitchingActive.store(pitchToggleButton.getToggleState()); };
    addAndMakeVisible(pitchToggleButton);

    helpButton.setButtonText("?");
    helpButton.getProperties().set("round", true);
    helpButton.setTooltip("About and theme");
    helpButton.onClick = [this] { showAboutPopup(); };
    addAndMakeVisible(helpButton);

    setSize (640, 250);
    updateCalculation();
    pop = 1.0f;
}

SemitoneCalculatorAudioProcessorEditor::~SemitoneCalculatorAudioProcessorEditor()
{
    stopTimer();
    infoPopup.reset();
    setLookAndFeel(nullptr);
}

//==============================================================================
void SemitoneCalculatorAudioProcessorEditor::renderBackground()
{
    const int w = juce::jmax(1, getWidth()), h = juce::jmax(1, getHeight());
    const auto& p = sc::palette(useLiquidGlassTheme);
    background = juce::Image(juce::Image::ARGB, w, h, true);
    {
        juce::Graphics g(background);
        g.setGradientFill(juce::ColourGradient(p.bgTop, 0.0f, 0.0f, p.bgBottom, 0.0f, static_cast<float>(h), false));
        g.fillAll();
        if (useLiquidGlassTheme)
        {
            // Soft coloured light behind the glass: it is what the panels refract.
            struct Blob { juce::Colour colour; float x, y, radius, alpha; };
            const Blob blobs[] = {
                { juce::Colour(0xff6b5cff), 0.16f, 0.30f, 0.42f, 0.55f },
                { juce::Colour(0xff2fd2ff), 0.62f, 0.95f, 0.40f, 0.45f },
                { juce::Colour(0xffff5fa8), 0.92f, 0.12f, 0.30f, 0.30f },
                { juce::Colour(0xff35f0c0), 0.40f, -0.05f, 0.25f, 0.22f } };
            for (const auto& b : blobs)
            {
                const juce::Point<float> centre(w * b.x, h * b.y);
                const float radius = static_cast<float>(w) * b.radius;
                juce::ColourGradient glow(b.colour.withAlpha(b.alpha), centre, b.colour.withAlpha(0.0f), centre.translated(radius, 0.0f), true);
                glow.addColour(0.45, b.colour.withAlpha(b.alpha * 0.4f));
                g.setGradientFill(glow);
                g.fillAll();
            }
        }
        else
        {
            g.setGradientFill(juce::ColourGradient(juce::Colours::white.withAlpha(0.035f), w * 0.5f, 0.0f,
                                                   juce::Colours::white.withAlpha(0.0f), w * 0.5f, h * 0.7f, true));
            g.fillAll();
        }
    }

    // The frosted copy: shrink, blur, and it is drawn back stretched inside the panels.
    frosted = {};
    if (useLiquidGlassTheme)
    {
        auto small = background.rescaled(juce::jmax(1, w / 8), juce::jmax(1, h / 8), juce::Graphics::highResamplingQuality);
        juce::ImageConvolutionKernel kernel(5);
        kernel.createGaussianBlur(2.2f);
        auto blurred = small.createCopy();
        kernel.applyToImage(blurred, small, small.getBounds());
        frosted = blurred;
    }
}

void SemitoneCalculatorAudioProcessorEditor::paintPanel(juce::Graphics& g, juce::Rectangle<float> r, float radius) const
{
    const auto& p = sc::palette(useLiquidGlassTheme);
    if (!useLiquidGlassTheme)
    {
        juce::DropShadow(juce::Colours::black.withAlpha(0.35f), 18, { 0, 6 }).drawForRectangle(g, r.toNearestInt());
        g.setColour(p.panel);
        g.fillRoundedRectangle(r, radius);
        g.setColour(p.panelBorder);
        g.drawRoundedRectangle(r.reduced(0.5f), radius, 1.0f);
        return;
    }

    juce::Path shape;
    shape.addRoundedRectangle(r, radius);
    juce::DropShadow(juce::Colours::black.withAlpha(0.45f), 24, { 0, 10 }).drawForPath(g, shape);
    g.saveState();
    g.reduceClipRegion(shape);
    if (frosted.isValid())
    {
        g.setImageResamplingQuality(juce::Graphics::highResamplingQuality);
        g.setOpacity(1.0f);
        g.drawImage(frosted, getLocalBounds().toFloat(), juce::RectanglePlacement::stretchToFit);
    }
    g.setColour(p.panel);
    g.fillRect(r);
    // light from above, and a faint glow along the bottom edge
    g.setGradientFill(juce::ColourGradient(juce::Colours::white.withAlpha(0.13f), r.getX(), r.getY(),
                                           juce::Colours::white.withAlpha(0.0f), r.getX(), r.getY() + r.getHeight() * 0.45f, false));
    g.fillRect(r);
    g.setGradientFill(juce::ColourGradient(juce::Colours::white.withAlpha(0.0f), r.getX(), r.getBottom() - 30.0f,
                                           juce::Colours::white.withAlpha(0.05f), r.getX(), r.getBottom(), false));
    g.fillRect(r);
    g.restoreState();

    // The rim: bright at the top left, fading round the edge.
    g.setGradientFill(juce::ColourGradient(juce::Colours::white.withAlpha(0.42f), r.getX(), r.getY(),
                                           juce::Colours::white.withAlpha(0.06f), r.getRight(), r.getBottom(), false));
    g.drawRoundedRectangle(r.reduced(0.5f), radius, 1.0f);
    g.setColour(juce::Colours::white.withAlpha(0.10f));
    g.drawRoundedRectangle(r.reduced(2.0f), radius - 1.5f, 0.8f);
}

void SemitoneCalculatorAudioProcessorEditor::paint (juce::Graphics& g)
{
    const auto& p = sc::palette(useLiquidGlassTheme);
    if (!background.isValid() || background.getWidth() != getWidth() || background.getHeight() != getHeight()) renderBackground();
    g.drawImageAt(background, 0, 0);

    // Header: name and brand.
    g.setColour(p.text);
    g.setFont(sc::font(16.0f, "Semibold"));
    g.drawText("Semitone Calculator", 22, 16, 300, 20, juce::Justification::centredLeft, false);
    g.setColour(p.textFaint);
    g.setFont(sc::font(11.5f));
    g.drawText(kBrand, 22, 36, 300, 14, juce::Justification::centredLeft, false);

    paintPanel(g, leftPanel, 16.0f);
    paintPanel(g, resultPanel, 16.0f);

    // Row captions
    g.setColour(p.textFaint);
    g.setFont(sc::font(11.0f, "Semibold").withExtraKerningFactor(0.1f));
    g.drawText("FROM", juce::Rectangle<int>(leftPanel.toNearestInt().getX() + 18, fromNoteBox.getY(), 50, fromNoteBox.getHeight()), juce::Justification::centredLeft, false);
    g.drawText("TO", juce::Rectangle<int>(leftPanel.toNearestInt().getX() + 18, toNoteBox.getY(), 50, toNoteBox.getHeight()), juce::Justification::centredLeft, false);
    g.setColour(p.fieldBorder);
    g.fillRect(juce::Rectangle<float>(leftPanel.getX() + 18.0f, static_cast<float>(pitchToggleButton.getY()) - 12.0f, leftPanel.getWidth() - 36.0f, 1.0f));

    // Result
    auto r = resultPanel.reduced(16.0f, 14.0f);
    g.setColour(p.textFaint);
    g.drawText("SEMITONES", r.removeFromTop(16.0f), juce::Justification::centred, false);

    const auto number = semitones > 0 ? "+" + juce::String(semitones) : semitones < 0 ? minusSign() + juce::String(-semitones) : juce::String("0");
    auto numberArea = r.removeFromTop(78.0f);
    {
        juce::Graphics::ScopedSaveState state(g);
        const auto centre = numberArea.getCentre();
        g.addTransform(juce::AffineTransform::scale(0.86f + 0.14f * pop, 0.86f + 0.14f * pop, centre.x, centre.y));
        g.setFont(sc::font(64.0f, "Light"));
        if (semitones != 0)
            g.setGradientFill(juce::ColourGradient(p.accent, numberArea.getX() + 40.0f, numberArea.getY(), p.accentEnd, numberArea.getRight() - 40.0f, numberArea.getBottom(), false));
        else
            g.setColour(p.text);
        if (!useLiquidGlassTheme) g.setColour(p.text);
        g.setOpacity(0.35f + 0.65f * pop);
        g.drawText(number, numberArea, juce::Justification::centred, false);
    }

    const auto fromName = shortName(fromNoteBox.getText()) + (fromModeBox.getSelected() == 1 ? "m" : "");
    const auto toName = shortName(toNoteBox.getText()) + (toModeBox.getSelected() == 1 ? "m" : "");
    g.setColour(p.textDim);
    g.setFont(sc::font(14.0f, "Semibold"));
    g.drawText(fromName + "   " + juce::String(juce::CharPointer_UTF8("\xe2\x9f\xb6")) + "   " + toName, r.removeFromTop(20.0f), juce::Justification::centred, false);
    g.setColour(p.textFaint);
    g.setFont(sc::font(12.0f));
    const auto direction = semitones == 0 ? juce::String("same pitch")
                         : juce::String(semitones > 0 ? "up " : "down ") + juce::String(std::abs(semitones)) + (std::abs(semitones) == 1 ? " semitone" : " semitones");
    g.drawText(direction, r.removeFromTop(18.0f), juce::Justification::centred, false);
    g.drawText("# sharp  " + juce::String(juce::CharPointer_UTF8("\xc2\xb7")) + "  b flat", r.removeFromBottom(16.0f), juce::Justification::centred, false);
}

void SemitoneCalculatorAudioProcessorEditor::resized()
{
    leftPanel = { 18.0f, 62.0f, 372.0f, 170.0f };
    resultPanel = { 402.0f, 62.0f, 220.0f, 170.0f };

    const int x = static_cast<int>(leftPanel.getX()) + 66;
    fromNoteBox.setBounds(x, 78, 142, 36);
    fromModeBox.setBounds(x + 150, 78, 140, 36);
    toNoteBox.setBounds(x, 124, 142, 36);
    toModeBox.setBounds(x + 150, 124, 140, 36);
    pitchToggleButton.setBounds(static_cast<int>(leftPanel.getX()) + 18, 184, 330, 36);

    helpButton.setBounds(getWidth() - 50, 16, 30, 30);
    if (infoPopup != nullptr) infoPopup->setBounds(getLocalBounds());
    background = {};
}

void SemitoneCalculatorAudioProcessorEditor::updateCalculation()
{
    int fromIdx = fromNoteBox.getSelectedId() - 1;
    int toIdx = toNoteBox.getSelectedId() - 1;

    // A minor key is counted from its relative major.
    if (fromModeBox.getSelected() == 1) fromIdx = (fromIdx + 3) % 12;
    if (toModeBox.getSelected() == 1) toIdx = (toIdx + 3) % 12;

    int diff = toIdx - fromIdx;

    if (diff > 6) diff -= 12;
    if (diff < -6) diff += 12;

    if (diff != semitones) { semitones = diff; pop = 0.0f; startTimerHz(60); }
    audioProcessor.transposeAmount.store(diff);

    // Keep processor state in sync so getStateInformation works even if the editor
    // isn't open when the host requests the plugin state.
    audioProcessor.fromNoteId.store(fromNoteBox.getSelectedId());
    audioProcessor.toNoteId.store(toNoteBox.getSelectedId());
    audioProcessor.fromModeId.store(fromModeBox.getSelected() + 1);
    audioProcessor.toModeId.store(toModeBox.getSelected() + 1);
    repaint(resultPanel.toNearestInt().expanded(4));
}

void SemitoneCalculatorAudioProcessorEditor::timerCallback()
{
    pop = juce::jmin(1.0f, pop + 1.0f / (60.0f * 0.22f));
    repaint(resultPanel.toNearestInt().expanded(4));
    if (pop >= 1.0f) stopTimer();
}

void SemitoneCalculatorAudioProcessorEditor::applyTheme()
{
    lookAndFeel.setGlass(useLiquidGlassTheme);
    background = {};
    sendLookAndFeelChange();
    repaint();
}

void SemitoneCalculatorAudioProcessorEditor::setTheme(bool glass)
{
    if (glass == useLiquidGlassTheme) return;
    useLiquidGlassTheme = glass;
    audioProcessor.setUseLiquidGlassTheme(glass);
    applyTheme();
}

void SemitoneCalculatorAudioProcessorEditor::showAboutPopup()
{
    if (infoPopup != nullptr) return;
    infoPopup = std::make_unique<InfoPopup>(*this);
    addAndMakeVisible(*infoPopup);
    infoPopup->setBounds(getLocalBounds());
    infoPopup->grabKeyboardFocus();
}
