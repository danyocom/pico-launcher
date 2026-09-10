#pragma once
#include <algorithm>
#include "core/math/Rgb.h"
#include "core/math/ColorConverter.h"
#include "IPalette.h"

/// @brief Palette for a focus highlight that carries both its fill and a 1px
///        border in a single tile - used for the icon row's pill and for the
///        circle the bottom sheets use.
///
/// The two need different colours, so the 16 entries are split rather than
/// being one ramp. Index 0 is the transparent entry for 4bpp sprites and is
/// left unused. Indices 1 to FILL_MAX_INDEX fade from whatever is behind the
/// button up to the pill's fill, giving its rounded corners something to
/// antialias against; BORDER_MIN_INDEX to BORDER_MAX_INDEX then fade from that
/// fill up to the border colour, so the border antialiases against the pill it
/// encloses rather than against the background.
///
/// The border is what keeps the highlight legible when a theme's focus fill
/// lands close in lightness to whatever sits behind it. That happens in two
/// places for the same reason: the icon row paints its own strip behind the
/// buttons, and a Tonal button's resting circle is surfaceContainerHighest
/// while its focused fill is secondaryContainer - neutral and secondary at the
/// SAME tone (90 light, and near it dark), so they differ in chroma but barely
/// in lightness. An outline in a contrasting role always reads.
class IconButtonSelectorPalette : public IPalette
{
public:
    static constexpr u32 FILL_MAX_INDEX = 8;
    static constexpr u32 BORDER_MIN_INDEX = 9;
    static constexpr u32 BORDER_MAX_INDEX = 15;

    IconButtonSelectorPalette(const Rgb<8, 8, 8>& backdropColor, const Rgb<8, 8, 8>& fillColor,
        const Rgb<8, 8, 8>& borderColor)
        : _backdropColor(backdropColor), _fillColor(fillColor), _borderColor(borderColor) { }

    void GetColors(u16* dst) const override
    {
        dst[0] = 0;
        for (u32 i = 1; i <= FILL_MAX_INDEX; i++)
        {
            dst[i] = ToEntry(Mix(_backdropColor, _fillColor, (int)i, (int)FILL_MAX_INDEX));
        }
        for (u32 i = BORDER_MIN_INDEX; i <= BORDER_MAX_INDEX; i++)
        {
            dst[i] = ToEntry(Mix(_fillColor, _borderColor,
                (int)(i - BORDER_MIN_INDEX) + 1, (int)(BORDER_MAX_INDEX - BORDER_MIN_INDEX) + 1));
        }
    }

    u32 GetHashCode() const override;

private:
    Rgb<8, 8, 8> _backdropColor;
    Rgb<8, 8, 8> _fillColor;
    Rgb<8, 8, 8> _borderColor;

    static Rgb<8, 8, 8> Mix(const Rgb<8, 8, 8>& from, const Rgb<8, 8, 8>& to, int step, int span)
    {
        return Rgb<8, 8, 8>(
            from.r + (((int)to.r - (int)from.r) * step / span),
            from.g + (((int)to.g - (int)from.g) * step / span),
            from.b + (((int)to.b - (int)from.b) * step / span));
    }

    static u16 ToEntry(const Rgb<8, 8, 8>& color)
    {
        return ColorConverter::ToXBGR555(Rgb<5, 5, 5>(color));
    }
};
