#pragma once
#include "core/math/Rgb.h"
#include "IconButtonView.h"

class IconButton2DView : public IconButtonView
{
    SHARED_ONLY(IconButton2DView)

public:
    class VramToken
    {
        u32 _vramOffset;
        u32 _pillVramOffset;
        u32 _borderedVramOffset;
    public:
        VramToken()
            : _vramOffset(0), _pillVramOffset(0), _borderedVramOffset(0) { }

        explicit VramToken(u32 offset, u32 pillOffset, u32 borderedOffset)
            : _vramOffset(offset), _pillVramOffset(pillOffset)
            , _borderedVramOffset(borderedOffset) { }

        constexpr u32 GetVramOffset() const { return _vramOffset; }
        constexpr u32 GetPillVramOffset() const { return _pillVramOffset; }
        constexpr u32 GetBorderedVramOffset() const { return _borderedVramOffset; }
    };

    void Draw(GraphicsContext& graphicsContext) override;

    void SetGraphics(const VramToken& vramToken)
    {
        _selectorVramOffset = vramToken.GetVramOffset();
        _pillVramOffset = vramToken.GetPillVramOffset();
        _borderedSelectorVramOffset = vramToken.GetBorderedVramOffset();
    }

    /// @brief Switches this button's focus/press highlight from the 32x32
    ///        circle to a 32x16 rounded rectangle. The circle is taller than
    ///        the bottom icon row's background strip, so on that row it would
    ///        spill over the groove at the strip's top edge and past the bottom
    ///        of the screen; the shorter shape sits wholly inside the strip.
    void SetUsePillSelector(bool usePillSelector) override
    {
        _usePillSelector = usePillSelector;
    }

    /// @brief Overrides the colour this button's icon assumes is behind it.
    ///        The icons are 4bpp with baked-in antialiasing: their edge
    ///        pixels are intermediate steps of a 16-entry gradient whose low
    ///        end is that backdrop, so it has to match whatever is actually
    ///        drawn underneath or the edges resolve to the wrong colour and
    ///        halo. With a circle behind (Type::Tonal/Filled) the circle's
    ///        own colour is used and this isn't needed; Type::Standard draws
    ///        nothing behind, so anything painting its own backdrop - like
    ///        the horizontal list layout's icon-row strip - needs to say so.
    void SetBackdropColor(const Rgb<8, 8, 8>& backdropColor) override
    {
        _backdropColor = backdropColor;
        _hasBackdropColor = true;
    }

    static VramToken UploadGraphics(IVramManager& vramManager);

private:
    u32 _selectorVramOffset;
    u32 _pillVramOffset = 0;
    u32 _borderedSelectorVramOffset = 0;
    bool _usePillSelector = false;
    Rgb<8, 8, 8> _backdropColor;
    bool _hasBackdropColor = false;

    IconButton2DView(Type type, State state,
        md::sys::color backgroundColor, const MaterialColorScheme* materialColorScheme)
        : IconButtonView(type, state, backgroundColor, materialColorScheme) { }
};
