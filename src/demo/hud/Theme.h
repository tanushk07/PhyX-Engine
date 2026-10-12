#pragma once
#include <raylib.h>
#include <map>
#include <utility>

// Display: Press Start 2P, the chunky 8-bit look, for titles and labels
// Body:    Pixelify Sans, a readable pixel font for sentences
// Mono:    VT323, a terminal font for numbers and key names (digits all have the same width)
enum class FontStyle { Body, Display, Mono };

// Fonts, colours and the pixel-art panel style shared by all 2D text.
// Create it after the window exists (it loads fonts onto the GPU); it frees them when destroyed.
class Theme {  // NOLINT(cppcoreguidelines-special-member-functions)
public:
    Theme();
    ~Theme();
    Theme(const Theme&) = delete;
    Theme& operator=(const Theme&) = delete;

    // ---- colours: a light paper panel with dark ink, like an old game's dialogue box
    static constexpr Color PanelFill   { 250, 248, 241, 238 };
    static constexpr Color Ink         {  38,  43,  61, 255 };   // border + main text
    static constexpr Color Shadow      {  38,  43,  61,  70 };   // hard offset shadow, no blur
    static constexpr Color TextMuted   {  98, 106, 128, 255 };
    static constexpr Color Disabled    { 184, 188, 198, 255 };   // keys that do nothing right now
    static constexpr Color Divider     {  38,  43,  61,  40 };
    static constexpr Color Accent      { 232, 106,  51, 255 };   // orange
    static constexpr Color Readout     {  20, 140, 120, 255 };   // teal, for live numbers
    static constexpr Color Running     {  46, 160,  87, 255 };
    static constexpr Color Paused      { 214,  64,  69, 255 };
    static constexpr Color Slow        {  52, 110, 220, 255 };

    // ---- text (positions are snapped to whole pixels so the pixel fonts stay sharp)
    void    Text(const char* text, float x, float y, float size, Color color, FontStyle style = FontStyle::Body) const;
    void    TextCentered(const char* text, float centerX, float y, float size, Color color, FontStyle style = FontStyle::Body) const;
    Vector2 Measure(const char* text, float size, FontStyle style = FontStyle::Body) const;

    // ---- shapes
    void  Panel(Rectangle r) const;                             // light panel, 2 px ink border, hard shadow
    void  Tag(Rectangle r, Color color) const;                  // small solid block with a darker bottom edge
    float KeyChip(const char* key, float x, float y, bool enabled = true) const;   // [key] as a pixel key cap (greyed out
                                                                                // when it does nothing right now); returns its width

private:
    const Font& Get(FontStyle style, float size) const;

    Font body{};
    // Display and Mono are drawn with nearest-pixel sampling, so each size gets its own atlas,
    // rasterised at exactly that size. They are made on first use.
    mutable std::map<std::pair<FontStyle, int>, Font> sized;
};
