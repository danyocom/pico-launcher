#pragma once
#include "gui/views/LabelView.h"
#include "../../views/BannerListItemView.h"

class MaterialColorScheme;
class IFontRepository;

class MaterialBannerListItemView : public BannerListItemView
{
    SHARED_ONLY(MaterialBannerListItemView)

public:
    void Draw(GraphicsContext& graphicsContext) override;

    Rectangle GetBounds() const override
    {
        // The horizontal list layout's item is a little shorter than the
        // standard banner list's 44 - see WideBannerListFileRecyclerAdapter::
        // GetViewSize for why that's safe.
        return Rectangle(_position, _width, _wide ? 40 : 44);
    }

    void SetGraphics(const VramToken& vramToken) override
    {
        _bgVramOffset = vramToken.GetVramOffset();
    }

    static VramToken UploadGraphics(const VramContext& vramContext);

private:
    const MaterialColorScheme* _materialColorScheme;
    u32 _bgVramOffset;
    /// @brief True for the horizontal list layout's item (see
    ///        RomBrowserWideBannerListDisplayMode) - wider than the standard
    ///        banner list item, which Draw() achieves by repeating the
    ///        tileable middle background segment one extra time.
    bool _wide;
    u32 _width;

    MaterialBannerListItemView(std::unique_ptr<IRomBrowserItemViewModel> viewModel,
        const MaterialColorScheme* materialColorScheme, const IFontRepository* fontRepository, bool wide);
};
