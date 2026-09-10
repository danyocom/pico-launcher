#pragma once
#include <algorithm>
#include "core/math/Rgb.h"
#include "core/math/ColorConverter.h"
#include "IPalette.h"

/// @brief Palette for the bottom icon row's background strip, packing both the
///        strip's vertical gradient and its top-edge groove into a single
///        16-entry row so the whole row can be drawn from one tile.
///
/// Index 0 is left unused - it's the transparent entry for 4bpp sprites, so a
/// tile row assigned to it shows the wallpaper through the strip rather than a
/// colour. Indices 1-13 are the gradient, top colour to bottom; 14 and 15 are
/// the groove's shadow and highlight lines. Keeping them in one row is what
/// lets the strip cost four sprites instead of twelve - drawn separately, the
/// gradient and the 2px groove needed a tile, a palette row and four sprites
/// each, on a screen whose OAM table only holds 128 entries in total.
class IconRowPalette : public IPalette
{
public:
    static constexpr u32 GRADIENT_MIN_INDEX = 1;
    static constexpr u32 GRADIENT_MAX_INDEX = 13;
    static constexpr u32 GROOVE_SHADOW_INDEX = 14;
    static constexpr u32 GROOVE_HIGHLIGHT_INDEX = 15;

    IconRowPalette(const Rgb<8, 8, 8>& topColor, const Rgb<8, 8, 8>& bottomColor)
        : _topColor(topColor), _bottomColor(bottomColor) { }

    void GetColors(u16* dst) const override
    {
        // Whatever is written here is never sampled, but leave it black rather
        // than uninitialised.
        dst[0] = 0;

        for (u32 i = GRADIENT_MIN_INDEX; i <= GRADIENT_MAX_INDEX; i++)
        {
            dst[i] = ToEntry(GetColorAtGradientIndex(i));
        }

        // A recessed groove: shadow directly above highlight, both shaded from
        // the strip's own top colour so the pair always sits in the same family
        // as the surface it's cut into, and stays legible whether that surface
        // is light or dark (against a dark strip the highlight carries the
        // effect, against a light one the shadow does).
        dst[GROOVE_SHADOW_INDEX] = ToEntry(ShadowColor());
        dst[GROOVE_HIGHLIGHT_INDEX] = ToEntry(HighlightColor());
    }

    u32 GetHashCode() const override;

    /// @brief The strip's colour at one of its gradient entries - used for the
    ///        backdrop the buttons' icons blend their antialiasing against.
    Rgb<8, 8, 8> GetColorAtGradientIndex(u32 index) const
    {
        int step = (int)std::clamp(index, GRADIENT_MIN_INDEX, GRADIENT_MAX_INDEX)
            - (int)GRADIENT_MIN_INDEX;
        int span = (int)(GRADIENT_MAX_INDEX - GRADIENT_MIN_INDEX);
        return Rgb<8, 8, 8>(
            _topColor.r + (((int)_bottomColor.r - (int)_topColor.r) * step / span),
            _topColor.g + (((int)_bottomColor.g - (int)_topColor.g) * step / span),
            _topColor.b + (((int)_bottomColor.b - (int)_topColor.b) * step / span));
    }

private:
    Rgb<8, 8, 8> _topColor;
    Rgb<8, 8, 8> _bottomColor;

    static u16 ToEntry(const Rgb<8, 8, 8>& color)
    {
        return ColorConverter::ToXBGR555(Rgb<5, 5, 5>(color));
    }

    Rgb<8, 8, 8> ShadowColor() const
    {
        return Rgb<8, 8, 8>(_topColor.r * 55 / 100, _topColor.g * 55 / 100, _topColor.b * 55 / 100);
    }

    Rgb<8, 8, 8> HighlightColor() const
    {
        return Rgb<8, 8, 8>(
            _topColor.r + ((255 - _topColor.r) * 40 / 100),
            _topColor.g + ((255 - _topColor.g) * 40 / 100),
            _topColor.b + ((255 - _topColor.b) * 40 / 100));
    }
};
