#include "Hud.h"
#include "Chapters.h"
#include <algorithm>
#include <cstdio>
#include <string>
#include <vector>

// ---- layout ----------------------------------------------------------------------------------

constexpr float Margin = 24;     // gap between the bars and the window edge
constexpr float PadX   = 24;     // space inside a bar, left and right
constexpr float PadY   = 14;     // space inside a bar, top and bottom

// Font sizes. Display and Mono are pixel-exact at these sizes (multiples of 8 and 25 px).
constexpr float DisplaySize = 16, MonoSize = 25, BodySize = 22, SmallSize = 20;
constexpr float Line = 28;

static float BarWidth() { return GetScreenWidth() - 2 * Margin; }

// ---- top bar, header row: status | chapter title | counters ----------------------------------

static void DrawStatus(const Theme& theme, float x, float y, const DemoState& state)
{
    const char* label = state.paused ? "PAUSED" : (state.slowMotion ? "SLOW x0.2" : "RUNNING");
    const Color color = state.paused ? Theme::Paused : (state.slowMotion ? Theme::Slow : Theme::Running);

    const float width = theme.Measure(label, DisplaySize, FontStyle::Display).x + 24;
    theme.Tag(Rectangle{ x, y, width, 30 }, color);
    theme.Text(label, x + 12, y + 7, DisplaySize, WHITE, FontStyle::Display);

    char fps[16];
    std::snprintf(fps, sizeof(fps), "%d FPS", GetFPS());
    theme.Text(fps, x + width + 14, y + 1, MonoSize, Theme::TextMuted, FontStyle::Mono);
}

static void DrawCounters(const Theme& theme, float right, float y, const PhysicsEngine& engine)
{
    int dynamic = 0, awake = 0, points = 0;
    for (const auto& entry : engine.GetEntries())
        if (entry.Body->IsDynamic()) { ++dynamic; if (entry.Body->IsAwake()) ++awake; }
    for (const Contact& c : engine.GetContactPoints()) points += c.Data.Count;

    const int values[3]   = { dynamic, awake, points };
    const char* labels[3] = { "bodies", "awake", "contacts" };
    const Color colors[3] = { Theme::Ink, Theme::Readout, Theme::Accent };

    // drawn right to left, so the last counter sits against the bar's right edge
    float x = right;
    for (int i = 2; i >= 0; --i)
    {
        char value[16];
        std::snprintf(value, sizeof(value), "%d", values[i]);
        x -= theme.Measure(labels[i], SmallSize).x;
        theme.Text(labels[i], x, y + 5, SmallSize, Theme::TextMuted);
        x -= theme.Measure(value, MonoSize, FontStyle::Mono).x + 6;
        theme.Text(value, x, y + 1, MonoSize, colors[i], FontStyle::Mono);
        x -= 26;
    }
}

static void DrawTitle(const Theme& theme, float centerX, float y, const DemoState& state, int chapterCount)
{
    char number[16];
    std::snprintf(number, sizeof(number), "%d/%d", state.chapter + 1, chapterCount);
    const char* title = GetChapters()[state.chapter].Title;

    constexpr float Gap = 20;
    const float numberWidth = theme.Measure(number, DisplaySize, FontStyle::Display).x;
    const float total = numberWidth + Gap + theme.Measure(title, DisplaySize, FontStyle::Display).x;
    const float x = centerX - total * 0.5f;
    theme.Text(number, x, y + 7, DisplaySize, Theme::Accent, FontStyle::Display);
    theme.Text(title, x + numberWidth + Gap, y + 7, DisplaySize, Theme::Ink, FontStyle::Display);
}

// one square per chapter, sitting on the bar's bottom edge
static void DrawProgress(float centerX, float y, int count, int current)
{
    constexpr float Size = 10, Gap = 8;
    float x = centerX - (count * Size + (count - 1) * Gap) * 0.5f;
    for (int i = 0; i < count; ++i, x += Size + Gap)
    {
        const bool done = i <= current;
        DrawRectangle(static_cast<int>(x) - 2, static_cast<int>(y) - 2, static_cast<int>(Size) + 4, static_cast<int>(Size) + 4, Theme::Ink);
        DrawRectangle(static_cast<int>(x), static_cast<int>(y), static_cast<int>(Size), static_cast<int>(Size),
                      i == current ? Theme::Accent : (done ? Theme::TextMuted : Theme::PanelFill));
    }
}

// ---- top bar ---------------------------------------------------------------------------------

static void DrawChapterBar(const Theme& theme, const PhysicsEngine& engine, const DemoState& state)
{
    const auto& chapters = GetChapters();
    const Chapter& ch = chapters[state.chapter];
    std::vector<std::string> readout;
    if (ch.Readout) ch.Readout(engine, readout);

    constexpr float HeaderHeight = 30 + 2 * PadY;
    const size_t lines = std::max(ch.Caption.size(), readout.size());
    const float height = HeaderHeight + lines * Line + 2 * PadY;
    const Rectangle bar{ Margin, Margin, BarWidth(), height };
    const float cx = bar.x + bar.width * 0.5f;
    theme.Panel(bar);

    // header: status on the left, counters mirrored on the right, the title centred between them
    const float headerY = bar.y + PadY;
    DrawStatus(theme, bar.x + PadX, headerY, state);
    DrawCounters(theme, bar.x + bar.width - PadX, headerY, engine);
    DrawTitle(theme, cx, headerY, state, static_cast<int>(chapters.size()));
    DrawRectangle(static_cast<int>(bar.x + PadX), static_cast<int>(bar.y + HeaderHeight), static_cast<int>(bar.width - 2 * PadX), 2, Theme::Divider);

    // body: the captions, and beside them (when the chapter has one) the live readout
    float y = bar.y + HeaderHeight + PadY;
    if (readout.empty())
    {
        for (const char* text : ch.Caption) { theme.TextCentered(text, cx, y, BodySize, Theme::Ink); y += Line; }
    }
    else
    {
        const float half = (bar.width - 2 * PadX) * 0.5f;
        const float leftCenter = bar.x + PadX + half * 0.5f, rightCenter = cx + half * 0.5f;
        for (size_t i = 0; i < ch.Caption.size(); ++i)
            theme.TextCentered(ch.Caption[i], leftCenter, y + i * Line, BodySize, Theme::Ink);

        const float boxY = y + (lines - readout.size()) * Line * 0.5f;
        DrawRectangle(static_cast<int>(cx + 12), static_cast<int>(boxY - 6), static_cast<int>(half - 12),
                      static_cast<int>(readout.size() * Line + 10), Fade(Theme::Readout, 0.1f));
        for (size_t i = 0; i < readout.size(); ++i)
            theme.TextCentered(readout[i].c_str(), rightCenter + 6, boxY + i * Line - 2, MonoSize, Theme::Readout, FontStyle::Mono);
    }

    DrawProgress(cx, bar.y + height - 5, static_cast<int>(chapters.size()), state.chapter);
}

// ---- bottom bar: keys and colour legend ------------------------------------------------------

struct KeyHint { std::vector<const char*> keys; const char* label; bool pausedOnly = false; };
struct LegendItem { Color color; const char* label; };

static const std::vector<KeyHint> SimKeys = {
    {{"Space"}, "pause"}, {{"N"}, "step (hold)", true}, {{"T"}, "slow-mo"}, {{"<-", "->"}, "chapter"}, {{"R"}, "restart"},
    {{"G"}, "lit/wire"}, {{"C"}, "contacts"}, {{"F"}, "forces"}, {{"V"}, "velocity"}, {{"P"}, "point vel."},
    {{"X"}, "axes"}, {{"H"}, "hide help"},
};

static const std::vector<KeyHint> WorldKeys = {
    {{"LMB"}, "grab"}, {{"B"}, "throw ball"}, {{"K"}, "drop box"}, {{"RMB", "WASD", "QE"}, "camera"}, {{"Wheel"}, "zoom"},
};

static const std::vector<LegendItem> Legend = {
    { RED, "contact" }, { ORANGE, "normal" }, { GOLD, "support" }, { SKYBLUE, "friction" },
    { BLUE, "velocity" }, { MAGENTA, "spin" }, { GREEN, "point vel." },
};

constexpr float ChipGap = 4, LabelGap = 8, ItemGap = 22, RowHeight = 34, SectionGap = 48;

static float ChipWidth(const Theme& theme, const char* key) { return theme.Measure(key, MonoSize, FontStyle::Mono).x + 16; }

static float KeysWidth(const Theme& theme, const std::vector<KeyHint>& keys)
{
    float w = 0;
    for (const KeyHint& h : keys)
    {
        for (const char* k : h.keys) w += ChipWidth(theme, k) + ChipGap;
        w += LabelGap - ChipGap + theme.Measure(h.label, SmallSize).x + ItemGap;
    }
    return w - ItemGap;
}

static float LegendWidth(const Theme& theme)
{
    float w = 0;
    for (const LegendItem& l : Legend) w += 18 + theme.Measure(l.label, SmallSize).x + ItemGap;
    return w - ItemGap;
}

static float DrawKeys(const Theme& theme, const std::vector<KeyHint>& keys, float x, float y, const DemoState& state)
{
    for (const KeyHint& h : keys)
    {
        const bool enabled = !h.pausedOnly || state.paused;    // e.g. N only steps while paused
        for (const char* k : h.keys) x += theme.KeyChip(k, x, y, enabled) + ChipGap;
        x += LabelGap - ChipGap;
        theme.Text(h.label, x, y + 3, SmallSize, enabled ? Theme::TextMuted : Theme::Disabled);
        x += theme.Measure(h.label, SmallSize).x + ItemGap;
    }
    return x;
}

static void DrawLegend(const Theme& theme, float x, float y)
{
    for (const LegendItem& l : Legend)
    {
        DrawRectangle(static_cast<int>(x), static_cast<int>(y + 7), 12, 12, l.color);
        theme.Text(l.label, x + 18, y + 3, SmallSize, Theme::Ink);
        x += 18 + theme.Measure(l.label, SmallSize).x + ItemGap;
    }
}

static void DrawKeysBar(const Theme& theme, const DemoState& state)
{
    // row 1: simulation and overlay keys; row 2: world keys, then what each overlay colour means
    const float row2 = KeysWidth(theme, WorldKeys) + SectionGap + LegendWidth(theme);
    const float height = 2 * PadY + 2 * RowHeight - 8;
    const Rectangle bar{ Margin, GetScreenHeight() - Margin - height, BarWidth(), height };
    const float cx = bar.x + bar.width * 0.5f;
    theme.Panel(bar);

    float y = bar.y + PadY;
    DrawKeys(theme, SimKeys, cx - KeysWidth(theme, SimKeys) * 0.5f, y, state);

    y += RowHeight;
    const float legendX = DrawKeys(theme, WorldKeys, cx - row2 * 0.5f, y, state) - ItemGap + SectionGap;
    DrawRectangle(static_cast<int>(legendX - SectionGap * 0.5f), static_cast<int>(y + 2), 2, 22, Theme::Divider);
    DrawLegend(theme, legendX, y);
}

static void DrawHelpHint(const Theme& theme)
{
    const float width = ChipWidth(theme, "H") + LabelGap + theme.Measure("show help", SmallSize).x + 2 * 16;
    const Rectangle bar{ (GetScreenWidth() - width) * 0.5f, GetScreenHeight() - Margin - 46, width, 46 };
    theme.Panel(bar);
    const float x = bar.x + 16 + theme.KeyChip("H", bar.x + 16, bar.y + 10) + LabelGap;
    theme.Text("show help", x, bar.y + 13, SmallSize, Theme::TextMuted);
}

void DrawHud(const Theme& theme, const PhysicsEngine& engine, const DemoState& state)
{
    DrawChapterBar(theme, engine, state);
    if (state.showHelp) DrawKeysBar(theme, state);
    else                DrawHelpHint(theme);
}
