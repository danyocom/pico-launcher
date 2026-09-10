#pragma once
#include "core/math/Rgb.h"

/// @brief Theme-supplied appearance of the bottom icon row's background strip,
///        used by layouts that reserve space for one (currently the horizontal
///        list layout - see RomBrowserWideBannerListDisplayMode). The strip is
///        drawn as a vertical gradient from topColor down to bottomColor;
///        setting both to the same value gives a flat fill.
class CustomIconRowInfo
{
public:
    CustomIconRowInfo()
        : _topColor(Rgb8(0, 0, 0)), _bottomColor(Rgb8(0, 0, 0)), _isSpecified(false) { }

    CustomIconRowInfo(const Rgb8& topColor, const Rgb8& bottomColor)
        : _topColor(topColor), _bottomColor(bottomColor), _isSpecified(true) { }

    const Rgb8& GetTopColor() const { return _topColor; }
    const Rgb8& GetBottomColor() const { return _bottomColor; }

    /// @brief False when the theme's json has no "iconRow" entry, in which case
    ///        the colours above mean nothing and AppBarView's colour-scheme
    ///        derived default should stand instead.
    bool IsSpecified() const { return _isSpecified; }

private:
    Rgb8 _topColor;
    Rgb8 _bottomColor;
    bool _isSpecified;
};
