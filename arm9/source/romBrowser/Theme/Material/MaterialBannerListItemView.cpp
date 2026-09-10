#include "common.h"
#include <libtwl/gfx/gfx.h>
#include "gui/PaletteManager.h"
#include "gui/OamManager.h"
#include "gui/OamBuilder.h"
#include "gui/IVramManager.h"
#include "gui/VramContext.h"
#include "gui/GraphicsContext.h"
#include "core/math/ColorConverter.h"
#include "core/math/RgbMixer.h"
#include "themes/material/MaterialColorScheme.h"
#include "themes/IFontRepository.h"
#include "bannerListItemBg0.h"
#include "bannerListItemBg1.h"
#include "bannerListItemBg2.h"
#include "gui/palette/GradientPalette.h"
#include "gui/palette/DirectPalette.h"
#include "gui/views/Label2DView.h"
#include "MaterialBannerListItemView.h"

MaterialBannerListItemView::MaterialBannerListItemView(std::unique_ptr<IRomBrowserItemViewModel> viewModel,
    const MaterialColorScheme* materialColorScheme, const IFontRepository* fontRepository, bool wide)
    : BannerListItemView(std::move(viewModel),
        // 160 rather than 197 for the wide cell: it now spans two background
        // segments instead of three, and a label wider than the cell would
        // overrun it. Narrower labels also cost fewer sprites - Label2DView
        // allocates one OAM entry per 32px of width, and with three labels per
        // row that adds up fast on a screen with a 128-entry table.
        Label2DView::CreateShared(wide ? 160 : 152, 16, 128, fontRepository->GetFont(FontType::Medium10)),
        Label2DView::CreateShared(wide ? 160 : 152, 16, 128, fontRepository->GetFont(FontType::Regular10)),
        Label2DView::CreateShared(wide ? 160 : 152, 16, 128, fontRepository->GetFont(FontType::Regular10)))
    , _materialColorScheme(materialColorScheme)
    , _wide(wide)
    // Visible width for the wide cell: 248 leaves a matching 4px margin either
    // side once the recycler's 7px padding and this view's -3 art offset are
    // applied. The narrow cell keeps its original end-to-end sprite span of
    // 224 and its original positioning, untouched.
    , _width(wide ? 248 : 224) { }

void MaterialBannerListItemView::Draw(GraphicsContext& graphicsContext)
{
    const int artX = _position.x + (_wide ? -3 : -2);
    const int artY = _position.y - 2;
    if (!graphicsContext.IsVisible(Rectangle(artX, artY, _width, 48)))
        return;

    auto backColor = _materialColorScheme->inverseOnSurface;
    auto frontColor = _isFocused
        ? _materialColorScheme->mainIconBg
        : _materialColorScheme->surfaceBright;
    u16 bgPltt[16];
    for (int i = 0; i < 16; i++)
    {
        auto blendFactors = ColorConverter::FromXBGR555(bannerListItemBg0Pal[i]);
        auto palColor = Rgb<8, 8, 8>(
            (backColor.r * blendFactors.r + frontColor.r * blendFactors.g + 16) / 31,
            (backColor.g * blendFactors.r + frontColor.g * blendFactors.g + 16) / 31,
            (backColor.b * blendFactors.r + frontColor.b * blendFactors.g + 16) / 31);
        palColor = palColor.Clamped();
        bgPltt[i] = ColorConverter::ToGBGR565(palColor);
    }
    u32 bgPaletteRow = graphicsContext.GetPaletteManager().AllocRow(
        DirectPalette(bgPltt), _position.y - 2, _position.y - 2 + 48);

    // Art is a 64px left cap, tileable 64px middle sections and a 32px right
    // cap. Every middle MUST be drawn as a full 64x64 sprite: OBJ tiles are
    // mapped one-dimensionally, so a sprite's width decides how many tiles make
    // up each of its rows. Drawing this 64-wide art as a 32-wide sprite makes
    // the hardware read four tiles per row instead of eight and reassemble the
    // section out of the wrong pieces - which is what turned the right-hand end
    // of these cells into garbage.
    //
    // Laid strictly end to end, 64px steps only build 224, 256 or 288 - none of
    // which leaves equal margins on a 256px screen. So the middles are allowed
    // to overlap each other instead, which is invisible: the section is uniform
    // across its width apart from two dithered rows, and those repeat every
    // 16px, so any offset that is a multiple of 16 keeps their pattern in
    // phase. 64 / 128 / 169 covers 64 through 236 continuously, stopping just
    // short of where the right cap's art begins - the middles must not run
    // underneath it, or they would show through the transparency around its
    // rounded corners and square them off.
    //
    // 169 is 9px out of that 16px phase. Equal margins and 16px alignment turn
    // out to be mutually exclusive here - the arithmetic only lands on both for
    // a half-pixel offset - and a phase break in two dithered rows at the very
    // top and bottom edge is the less visible of the two compromises.
    static constexpr int WIDE_MIDDLE_OFFSETS[] = { 64, 128, 169 };
    static constexpr int NARROW_MIDDLE_OFFSETS[] = { 64, 128 };
    const int* middleOffsets = _wide ? WIDE_MIDDLE_OFFSETS : NARROW_MIDDLE_OFFSETS;
    u32 middleTileCount = _wide ? 3 : 2;

    gfx_oam_entry_t* oam = graphicsContext.GetOamManager().AllocOams(2 + middleTileCount);
    u32 middleVramOffset = (_bgVramOffset + bannerListItemBg0TilesLen) >> 7;
    u32 rightCapVramOffset =
        (_bgVramOffset + bannerListItemBg0TilesLen + bannerListItemBg1TilesLen) >> 7;

    OamBuilder::OamWithSize<64, 64>(artX, artY, _bgVramOffset >> 7)
        .WithPalette16(bgPaletteRow)
        .WithPriority(graphicsContext.GetPriority())
        .Build(oam[0]);

    for (u32 i = 0; i < middleTileCount; i++)
    {
        OamBuilder::OamWithSize<64, 64>(artX + middleOffsets[i], artY, middleVramOffset)
            .WithPalette16(bgPaletteRow)
            .WithPriority(graphicsContext.GetPriority())
            .Build(oam[1 + i]);
    }

    // The right cap's sprite is 32 wide but only its first 15 columns carry
    // art - the other 17 are transparent padding. Positioning it by its sprite
    // width therefore left the cell's visible right edge 17px short, which is
    // why its margin never matched the left one however the width was set.
    // Placed by its ART width instead, so _width really is the visible width.
    OamBuilder::OamWithSize<32, 64>(artX + _width - (_wide ? 15 : 32), artY, rightCapVramOffset)
        .WithPalette16(bgPaletteRow)
        .WithPriority(graphicsContext.GetPriority())
        .Build(oam[1 + middleTileCount]);

    if (_isFocused)
    {
        _firstLine->SetBackgroundColor(frontColor);
        _firstLine->SetForegroundColor(_materialColorScheme->onSecondaryContainer);
        _secondLine->SetBackgroundColor(frontColor);
        _secondLine->SetForegroundColor(_materialColorScheme->onSecondaryContainer);
        _thirdLine->SetBackgroundColor(frontColor);
        _thirdLine->SetForegroundColor(_materialColorScheme->onSecondaryContainer);
    }
    else
    {
        _firstLine->SetForegroundColor(_materialColorScheme->onSurface);
        _firstLine->SetBackgroundColor(_materialColorScheme->surfaceBright);
        _secondLine->SetForegroundColor(_materialColorScheme->onSurfaceVariant);
        _secondLine->SetBackgroundColor(_materialColorScheme->surfaceBright);
        _thirdLine->SetForegroundColor(_materialColorScheme->onSurfaceVariant);
        _thirdLine->SetBackgroundColor(_materialColorScheme->surfaceBright);
    }

    if (_lines == 1)
    {
        _firstLine->SetPosition(_position.x + 6 + 32 + 6, _position.y + 14);
    }
    else if (_lines == 2)
    {
        _firstLine->SetPosition(_position.x + 6 + 32 + 6, _position.y + 8);
        _secondLine->SetPosition(_position.x + 6 + 32 + 6, _position.y + 20);
    }
    else
    {
        _firstLine->SetPosition(_position.x + 6 + 32 + 6, _position.y + 2);
        _secondLine->SetPosition(_position.x + 6 + 32 + 6, _position.y + 14);
        _thirdLine->SetPosition(_position.x + 6 + 32 + 6, _position.y + 26);
    }

    if (_lines >= 1)
        _firstLine->Draw(graphicsContext);
    if (_lines >= 2)
        _secondLine->Draw(graphicsContext);
    if (_lines >= 3)
        _thirdLine->Draw(graphicsContext);

    if (_icon)
    {
        _icon->SetPosition(6 + _position.x, 6 + _position.y);
        _icon->Draw(graphicsContext, frontColor);
    }
}

BannerListItemView::VramToken MaterialBannerListItemView::UploadGraphics(const VramContext& vramContext)
{
    const auto objVramManager = vramContext.GetObjVramManager();
    u32 vramOffset = 0;
    if (objVramManager)
    {
        vramOffset = objVramManager->Alloc(
            bannerListItemBg0TilesLen + bannerListItemBg1TilesLen + bannerListItemBg2TilesLen);
        dma_ntrCopy32(3, bannerListItemBg0Tiles,
            objVramManager->GetVramAddress(vramOffset),
            bannerListItemBg0TilesLen);
        dma_ntrCopy32(3, bannerListItemBg1Tiles,
            objVramManager->GetVramAddress(vramOffset + bannerListItemBg0TilesLen),
            bannerListItemBg1TilesLen);
        dma_ntrCopy32(3, bannerListItemBg2Tiles,
            objVramManager->GetVramAddress(vramOffset + bannerListItemBg0TilesLen + bannerListItemBg1TilesLen),
            bannerListItemBg2TilesLen);
    }
    return BannerListItemView::VramToken(vramOffset);
}