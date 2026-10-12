#include "Theme.h"
#include <cmath>
#include <string>

static std::string FontPath(const char* file)
{
    return std::string(GetApplicationDirectory()) + "assets/fonts/" + file;
}

static const char* FontFile(FontStyle style)
{
    switch (style)
    {
        case FontStyle::Display: return "PressStart2P-Regular.ttf";
        case FontStyle::Mono:    return "VT323-Regular.ttf";
        default:                 return "PixelifySans.ttf";
    }
}

// Loads `file` rasterised at `size` px. Smooth fonts get mipmaps so they scale; pixel fonts get
// nearest-pixel sampling so their square pixels stay square.
static Font LoadUiFont(const char* file, int size, bool pixelExact)
{
    const std::string path = FontPath(file);
    if (!FileExists(path.c_str()))
    {
        TraceLog(LOG_WARNING, "HUD: font %s not found, using raylib's default font", path.c_str());
        return GetFontDefault();
    }
    Font font = LoadFontEx(path.c_str(), size, nullptr, 0);
    if (pixelExact)
        SetTextureFilter(font.texture, TEXTURE_FILTER_POINT);
    else
    {
        GenTextureMipmaps(&font.texture);
        SetTextureFilter(font.texture, TEXTURE_FILTER_TRILINEAR);
    }
    return font;
}

static void UnloadUiFont(const Font& font)
{
    if (font.texture.id != GetFontDefault().texture.id) UnloadFont(font);   // the default font belongs to raylib
}

Theme::Theme()
{
    body = LoadUiFont(FontFile(FontStyle::Body), 64, false);   // Pixelify has soft edges, so it can scale
}

Theme::~Theme()
{
    UnloadUiFont(body);
    for (const auto& [key, font] : sized) UnloadUiFont(font);
}

const Font& Theme::Get(FontStyle style, float size) const
{
    if (style == FontStyle::Body) return body;

    const auto key = std::make_pair(style, static_cast<int>(std::lround(size)));
    auto it = sized.find(key);
    if (it == sized.end()) it = sized.emplace(key, LoadUiFont(FontFile(style), key.second, true)).first;
    return it->second;
}

// ---- text ------------------------------------------------------------------------------------

// Pixelify is drawn from one big atlas and its letters touch when shrunk, so it gets a little air.
static float Spacing(FontStyle style, float size) { return style == FontStyle::Body ? size * 0.06f : 0.0f; }

void Theme::Text(const char* text, float x, float y, float size, Color color, FontStyle style) const
{
    DrawTextEx(Get(style, size), text, Vector2{ std::round(x), std::round(y) }, size, Spacing(style, size), color);
}

void Theme::TextCentered(const char* text, float centerX, float y, float size, Color color, FontStyle style) const
{
    Text(text, centerX - Measure(text, size, style).x * 0.5f, y, size, color, style);
}

Vector2 Theme::Measure(const char* text, float size, FontStyle style) const
{
    return MeasureTextEx(Get(style, size), text, size, Spacing(style, size));
}

// ---- shapes ----------------------------------------------------------------------------------

static void Fill(float x, float y, float w, float h, Color color)
{
    DrawRectangle(static_cast<int>(std::round(x)), static_cast<int>(std::round(y)),
                  static_cast<int>(std::round(w)), static_cast<int>(std::round(h)), color);
}

void Theme::Panel(Rectangle r) const
{
    constexpr float Border = 2, ShadowOffset = 5;
    Fill(r.x + ShadowOffset, r.y + ShadowOffset, r.width, r.height, Shadow);
    Fill(r.x, r.y, r.width, r.height, PanelFill);
    // border with the corner pixels left out, for a slightly rounded pixel-art look
    Fill(r.x + Border, r.y,                       r.width - 2 * Border, Border, Ink);
    Fill(r.x + Border, r.y + r.height - Border,   r.width - 2 * Border, Border, Ink);
    Fill(r.x,                     r.y + Border,   Border, r.height - 2 * Border, Ink);
    Fill(r.x + r.width - Border,  r.y + Border,   Border, r.height - 2 * Border, Ink);
}

void Theme::Tag(Rectangle r, Color color) const
{
    Fill(r.x, r.y, r.width, r.height, color);
    Fill(r.x, r.y + r.height - 3, r.width, 3, ColorBrightness(color, -0.3f));
}

float Theme::KeyChip(const char* key, float x, float y, bool enabled) const
{
    constexpr float Size = 25, PadX = 8, Height = 26;   // VT323 is pixel-exact at multiples of 25 px
    const float width = Measure(key, Size, FontStyle::Mono).x + 2 * PadX;
    const Color ink = enabled ? Ink : Disabled;
    Fill(x, y, width, Height, ink);                               // outline
    Fill(x + 2, y + 2, width - 4, Height - 4, enabled ? Color{ 255, 255, 255, 255 } : Color{ 236, 236, 232, 255 });
    if (enabled) Fill(x + 2, y + Height - 6, width - 4, 4, Color{ 214, 217, 226, 255 });   // key cap lip; a greyed key lies flat
    Text(key, x + PadX, y + (Height - Size) * 0.5f - 2, Size, ink, FontStyle::Mono);
    return width;
}
