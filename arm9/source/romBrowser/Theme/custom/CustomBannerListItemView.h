#pragma once
#include "gui/views/LabelView.h"
#include "romBrowser/views/BannerListItemView.h"

class MaterialColorScheme;
class IFontRepository;
class CustomThemeInfo;

class CustomBannerListItemView : public BannerListItemView
{
    SHARED_ONLY(CustomBannerListItemView)

public:
    void Draw(GraphicsContext& graphicsContext) override;

    Rectangle GetBounds() const override
    {
        // See WideBannerListFileRecyclerAdapter::GetViewSize for why the
        // horizontal list layout's item is a little shorter than the
        // standard banner list's 44.
        return Rectangle(_position, _width, _wide ? 40 : 44);
    }

private:
    const CustomThemeInfo* _customThemeInfo;
    const MaterialColorScheme* _materialColorScheme;
    u32 _texVramOffset = 0;
    u32 _plttVramOffset = 0;
    u32 _selectedTexVramOffset = 0;
    u32 _selectedPlttVramOffset = 0;
    /// @brief True for the horizontal list layout's item (see
    ///        RomBrowserWideBannerListDisplayMode) - wider than the standard
    ///        banner list item. Unlike the Material theme's item (which
    ///        widens by repeating a tileable background segment), this
    ///        stretches the custom theme's fixed-size background texture to
    ///        the new width instead: see Draw() for why that's the safer
    ///        option here.
    bool _wide;
    u32 _width;

    CustomBannerListItemView(std::unique_ptr<IRomBrowserItemViewModel> viewModel, const CustomThemeInfo* customThemeInfo,
        const MaterialColorScheme* materialColorScheme, const IFontRepository* fontRepository,
        u32 texVramOffset, u32 plttVramOffset, u32 selectedTexVramOffset, u32 selectedPlttVramOffset,
        VBlankTextureLoader* vblankTextureLoader, bool wide);
};
