#include "common.h"
#include "gui/OamManager.h"
#include "gui/OamBuilder.h"
#include "gui/IVramManager.h"
#include "gui/GraphicsContext.h"
#include "gui/input/InputProvider.h"
#include "iconButtonSelector.h"
#include "iconButtonSelectorBordered.h"
#include "iconButtonPill.h"
#include "core/math/ColorConverter.h"
#include "gui/palette/GradientPalette.h"
#include "gui/palette/IconButtonSelectorPalette.h"
#include "themes/material/MaterialColorScheme.h"
#include "IconButton2DView.h"

void IconButton2DView::Draw(GraphicsContext& graphicsContext)
{
    if (!graphicsContext.IsVisible(GetBounds()))
        return;

    u32 iconPaletteRow;
    if (_isFocused || _penDown)
    {
        // Same reasoning as the rest-state ramp further down: the selector
        // circle's edge pixels are intermediate steps of this gradient, so its
        // low end has to be whatever is really behind the button. On a sheet
        // that is the scheme's background, but the horizontal list layout's
        // icon row paints its own strip and says so via SetBackdropColor.
        const Rgb<8, 8, 8>& bgColor = _hasBackdropColor
            ? _backdropColor
            : _materialColorScheme->GetColor(_backgroundColor);
        const auto& selectorColor = _materialColorScheme->GetColor(GetFocusFillColor());
        gfx_oam_entry_t* selectorOam = graphicsContext.GetOamManager().AllocOams(1);
        if (_usePillSelector)
        {
            // The pill's artwork sits inset within its tile (28x20, centred
            // horizontally, and placed so that drawing the tile at
            // _position.y + 3 lands it on _position.y + 6 through + 25). A
            // 32x32 sprite rather than a shorter one because the shape is 20
            // rows tall and OBJ heights only come in 8, 16 and 32 - the
            // surrounding rows are transparent. That keeps it clear of the
            // groove along the strip's top edge and of the bottom of the
            // screen.
            //
            // The same tile also carries a 1px border, drawn from a separate
            // span of the palette so it can take its own colour rather than the
            // fill's - see IconButtonSelectorPalette. Costs no extra sprite.
            //
            // The border matters here in a way it wouldn't on a dialog: the
            // focus fill is chosen to contrast with the theme's surfaces, but
            // this row paints its own strip behind the buttons, and a theme is
            // free to set that to anything - including something close to the
            // fill, which would leave the highlight barely visible. An outline
            // in the scheme's dedicated border role always reads against it.
            u32 selectorPlttRow = graphicsContext.GetPaletteManager().AllocRow(
                IconButtonSelectorPalette(bgColor, selectorColor,
                    _materialColorScheme->GetColor(md::sys::color::outline)),
                _position.y + 6, _position.y + 26);
            OamBuilder::OamWithSize<32, 32>(
                    _position.x,
                    _position.y + 3, _pillVramOffset >> 7)
                .WithPalette16(selectorPlttRow)
                .WithPriority(graphicsContext.GetPriority())
                .Build(selectorOam[0]);
        }
        else
        {
            // Same border treatment as the pill above, and for the same
            // reason. A Tonal button rests on surfaceContainerHighest and
            // focuses to secondaryContainer - neutral and secondary at the same
            // tone, so they differ in chroma but hardly at all in lightness,
            // which left focus nearly invisible when moving through a sheet by
            // d-pad. The fills are unchanged (so selected still reads
            // differently from unselected); the outline is what makes the
            // focused one obvious.
            u32 selectorPlttRow = graphicsContext.GetPaletteManager().AllocRow(
                IconButtonSelectorPalette(bgColor, selectorColor,
                    _materialColorScheme->GetColor(md::sys::color::outline)),
                _position.y, _position.y + 32);
            OamBuilder::OamWithSize<32, 32>(
                    _position.x + 3,
                    _position.y + 3, _borderedSelectorVramOffset >> 7)
                .WithPalette16(selectorPlttRow)
                .WithPriority(graphicsContext.GetPriority())
                .Build(selectorOam[0]);
        }

        iconPaletteRow = graphicsContext.GetPaletteManager().AllocRow(
            GradientPalette(selectorColor, GetFocusIconColor()), _position.y + 8, _position.y + 24);
    }
    else
    {
        if (IsCircleBackgroundVisible())
        {
            auto circleBgColor = GetCircleBackgroundColor();
            u32 circlePaletteRow = graphicsContext.GetPaletteManager().AllocRow(
                GradientPalette(
                    _materialColorScheme->GetColor(_backgroundColor),
                    _materialColorScheme->GetColor(circleBgColor)),
                _position.y, _position.y + 32);
            gfx_oam_entry_t* selectorOam = graphicsContext.GetOamManager().AllocOams(1);
            OamBuilder::OamWithSize<32, 32>(
                    _position.x + 3,
                    _position.y + 3, _selectorVramOffset >> 7)
                .WithPalette16(circlePaletteRow)
                .WithPriority(graphicsContext.GetPriority())
                .Build(selectorOam[0]);
            iconPaletteRow = graphicsContext.GetPaletteManager().AllocRow(
                GradientPalette(
                    _materialColorScheme->GetColor(circleBgColor),
                    GetIconColor()),
                _position.y + 8, _position.y + 24);
        }
        else
        {
            iconPaletteRow = graphicsContext.GetPaletteManager().AllocRow(
                GradientPalette(
                    _hasBackdropColor
                        ? _backdropColor
                        : _materialColorScheme->GetColor(_backgroundColor),
                    GetIconColor()),
                _position.y + 8, _position.y + 24);
        }
    }
    gfx_oam_entry_t* iconOam = graphicsContext.GetOamManager().AllocOams(1);
    OamBuilder::OamWithSize<16, 16>(
            _position.x + 8,
            _position.y + 8, _iconVramOffset >> 7)
        .WithPalette16(iconPaletteRow)
        .WithPriority(graphicsContext.GetPriority())
        .Build(iconOam[0]);
}

IconButton2DView::VramToken IconButton2DView::UploadGraphics(IVramManager& vramManager)
{
    u32 vramOffset = vramManager.Alloc(iconButtonSelectorTilesLen);
    dma_ntrCopy32(3, iconButtonSelectorTiles, vramManager.GetVramAddress(vramOffset), iconButtonSelectorTilesLen);
    // Both shapes are uploaded together so a caller doesn't have to know which
    // one its buttons will end up using. Each is a 32x32 4bpp tile - 512 bytes,
    // already a multiple of the 128-byte granularity OAM tile indices are
    // counted in, so the second allocation stays aligned.
    u32 pillVramOffset = vramManager.Alloc(iconButtonPillTilesLen);
    dma_ntrCopy32(3, iconButtonPillTiles, vramManager.GetVramAddress(pillVramOffset), iconButtonPillTilesLen);
    u32 borderedVramOffset = vramManager.Alloc(iconButtonSelectorBorderedTilesLen);
    dma_ntrCopy32(3, iconButtonSelectorBorderedTiles,
        vramManager.GetVramAddress(borderedVramOffset), iconButtonSelectorBorderedTilesLen);
    return IconButton2DView::VramToken(vramOffset, pillVramOffset, borderedVramOffset);
}
