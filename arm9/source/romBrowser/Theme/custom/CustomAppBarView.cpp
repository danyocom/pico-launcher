#include "common.h"
#include <libtwl/dma/dmaNitro.h>
#include "gui/Gx.h"
#include "core/math/Rgb.h"
#include "gui/VramContext.h"
#include "gui/IVramManager.h"
#include "gui/OamBuilder.h"
#include "gui/GraphicsContext.h"
#include "gui/palette/GradientPalette.h"
#include "iconRowStrip.h"
#include "romBrowser/views/IconButton2DView.h"
#include "romBrowser/views/IconButton3DView.h"
#include "CustomAppBarView.h"

CustomAppBarView::CustomAppBarView(int x, int y, Orientation orientation,
    int startButtonCount, int endButtonCount, const CustomThemeInfo* customThemeInfo,
    const MaterialColorScheme* materialColorScheme,
    u32 scrimTexVramOffset, u32 scrimPlttVramOffset)
    : AppBarView(x, y, orientation, startButtonCount, endButtonCount, materialColorScheme)
    , _scrimTexVramOffset(scrimTexVramOffset), _scrimPlttVramOffset(scrimPlttVramOffset)
{
    // Sprite buttons ONLY for the bottom icon row, 3D buttons everywhere else.
    //
    // The bottom row has to use sprites: its background strip is a sprite (the
    // list's game icons behind it are sprites too, and nothing on a BG or 3D
    // layer can cover a sprite), and a sprite strip would hide a 3D focus
    // circle no matter how the two were ordered.
    //
    // But that trade is only worth making where the strip exists. A 3D circle
    // blends per-pixel against whatever is actually behind it, so on the
    // vertical and top bars - where the background is this theme's translucent
    // scrim over an arbitrary wallpaper - it stays clean. A sprite circle
    // instead carries baked-in antialiasing computed against one assumed
    // backdrop colour, which nothing behind those bars matches, leaving haloed
    // edges on the icons and their selectors.
    bool atBottom = orientation == Orientation::Horizontal && y != 0;
    for (int i = 0; i < _startButtonCount + _endButtonCount; i++)
    {
        if (atBottom)
        {
            // Type::Standard: nothing is drawn behind an icon at rest, so only
            // the focused button gets a selector on top of the strip.
            _buttons[i] = IconButton2DView::CreateShared(
                IconButtonView::Type::Standard,
                IconButtonView::State::ToggleUnselected,
                md::sys::color::inverseOnSurface,
                materialColorScheme);
        }
        else
        {
            _buttons[i] = IconButton3DView::CreateShared(
                IconButtonView::Type::Tonal,
                // the unselected tone at rest, so the focus circle (which uses the
                // selected tone, matching the display settings sheet) stands out
                IconButtonView::State::ToggleUnselected,
                md::sys::color::inverseOnSurface,
                materialColorScheme);
        }
        AddChildTail(_buttons[i].GetPointer());
    }

    // Only override AppBarView's colour-scheme derived default when this theme
    // actually asked for something; themes predating the "iconRow" key keep
    // colours generated from their own palette.
    if (customThemeInfo->iconRowInfo.IsSpecified())
    {
        SetIconRowColors(customThemeInfo->iconRowInfo.GetTopColor(),
            customThemeInfo->iconRowInfo.GetBottomColor());
    }
}

void CustomAppBarView::InitVram(const VramContext& vramContext)
{
    // Allocates the icon row strip's tile and cascades to the children.
    AppBarView::InitVram(vramContext);

    const auto objVramManager = vramContext.GetObjVramManager();
    if (!objVramManager)
    {
        return;
    }

    // Matches the constructor: only the bottom row's buttons are sprite-based.
    if (IsAtBottom())
    {
        auto iconButtonVramToken = IconButton2DView::UploadGraphics(*objVramManager);
        for (int i = 0; i < _startButtonCount + _endButtonCount; i++)
        {
            static_cast<IconButton2DView*>(_buttons[i].GetPointer())->SetGraphics(iconButtonVramToken);
        }
    }
}

void CustomAppBarView::Draw(GraphicsContext& graphicsContext)
{
    Gx::MtxIdentity();

    // The horizontal list layout (RomBrowserWideBannerListDisplayMode) is
    // the only one that puts this bar against the bottom of the screen;
    // every other layout using this view keeps it at the top or down the
    // side, and those keep the theme's own translucent scrim texture below.
    bool atBottom = GetOrientation() == Orientation::Horizontal && GetPosition().y != 0;

    // A bottom bar's background is the icon row strip, which AppBarView draws
    // for every theme (see AppBarView::DrawIconRowStrip) - the theme's own
    // translucent scrim is for the top/side positions only.
    if (!atBottom)
    {
        Gx::PolygonAttr(GX_LIGHTMASK_NONE, GX_POLYGON_MODE_MODULATE, GX_DISPLAY_MODE_FRONT,
            false, false, false, GX_DEPTH_FUNC_LESS, false, 31, 63);
        Gx::Color(0);
        Gx::TexImageParam(_scrimTexVramOffset >> 3, true, false, false, false, GX_TEXSIZE_8,
            GX_TEXSIZE_64, GX_TEXFMT_A5I3, false, GX_TEXGEN_NONE);
        Gx::TexPlttBase(_scrimPlttVramOffset >> 4);
        Gx::Begin(GX_PRIMITIVE_QUAD);
        if (GetOrientation() == Orientation::Horizontal)
        {
            Gx::TexCoord(0, 0);
            Gx::Vtx16(0, 0, -1.0 / 64);
            Gx::TexCoord(0, 42);
            Gx::Vtx16(0, 42.0 / 512.0, -1.0 / 64);
            Gx::TexCoord(256, 42);
            Gx::Vtx16(256.0 / 64, 42.0 / 512.0, -1.0 / 64);
            Gx::TexCoord(256, 0);
            Gx::Vtx16(256.0 / 64, 0, -1.0 / 64);
        }
        else
        {
            Gx::TexCoord(0, 0);
            Gx::Vtx16(0, 192.0 / 512, -1.0 / 64);
            Gx::TexCoord(0, 42);
            Gx::Vtx16(42.0 / 64, 192.0 / 512, -1.0 / 64);
            Gx::TexCoord(192, 42);
            Gx::Vtx16(42.0 / 64, 0, -1.0 / 64);
            Gx::TexCoord(192, 0);
            Gx::Vtx16(0, 0, -1.0 / 64);
            Gx::End();
        }
    }
    AppBarView::Draw(graphicsContext);
}
