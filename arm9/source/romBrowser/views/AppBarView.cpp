#include "common.h"
#include <algorithm>
#include <libtwl/dma/dmaNitro.h>
#include "gui/GraphicsContext.h"
#include "gui/VramContext.h"
#include "gui/IVramManager.h"
#include "gui/OamBuilder.h"
#include "gui/palette/IconRowPalette.h"
#include "iconRowStrip.h"
#include "gui/materialDesign.h"
#include "themes/material/MaterialColorScheme.h"
#include "AppBarView.h"

#define BUTTON_SIZE     32

// Nudges the strip down relative to the buttons, so the icons sit a little
// lower within it rather than flush against its top edge.
#define ICON_ROW_STRIP_Y_OFFSET     3

// The strip's height, and how iconRowStrip.png's tile is laid out: its first
// ICON_ROW_GROOVE_ROWS rows carry the groove (palette indices 14 and 15) and
// the rest ramp across the gradient's indices, reaching the last of them at the
// strip's bottom. The tile bakes these numbers in, so regenerating it and
// changing them has to happen together - the ramp is built as
// index = GRADIENT_MIN_INDEX + round((row - ICON_ROW_GROOVE_ROWS) /
// (ICON_ROW_STRIP_HEIGHT - ICON_ROW_GROOVE_ROWS - 1) *
// (GRADIENT_MAX_INDEX - GRADIENT_MIN_INDEX)). The ramp must never reach index
// 0: that entry is transparent for 4bpp sprites, so a row using it shows the
// wallpaper straight through the strip.
// Must equal 192 - RomBrowserDisplayMode::GetIconRowTopY() -
// ICON_ROW_STRIP_Y_OFFSET for the layout this is used by, AND match the height
// iconRowStrip.png's ramp was generated for. Kept as a constant rather than
// computed from GetPosition() precisely because the asset can't follow a
// runtime value: if the bar moves, the tile has to be regenerated to suit.
#define ICON_ROW_STRIP_HEIGHT       24
#define ICON_ROW_GROOVE_ROWS        2

// Where IconButton2DView::Draw puts a button's 16x16 icon within its 32px cell.
#define ICON_BUTTON_ICON_Y_OFFSET   8
#define ICON_BUTTON_ICON_SIZE       16

AppBarView::AppBarView(int x, int y, Orientation orientation,
    int startButtonCount, int endButtonCount, const MaterialColorScheme* materialColorScheme)
    : _orientation(orientation)
    , _buttons(std::make_unique<SharedPtr<IconButtonView>[]>(startButtonCount + endButtonCount))
    , _startButtonCount(startButtonCount), _endButtonCount(endButtonCount)
    , _materialColorScheme(materialColorScheme)
{
    // Default the icon row strip to the theme's own container tones rather than
    // any fixed pair. Every colour scheme is generated from the theme's
    // primaryColor and darkTheme flag (see MaterialColorSchemeFactory), and the
    // buttons' own icons are drawn in onSurfaceVariant from that same scheme -
    // so these are guaranteed to contrast with the icons and to follow a theme
    // from light to dark, which no hardcoded colour can do. Themes wanting
    // something specific override it via theme.json's "iconRow".
    SetIconRowColors(materialColorScheme->GetColor(md::sys::color::surfaceContainerHighest),
        materialColorScheme->GetColor(md::sys::color::surfaceContainerLow));
    SetPosition(x, y);
}

Rectangle AppBarView::GetBounds() const
{
    if (_orientation == Orientation::Horizontal)
        return Rectangle(0, _position.y, 256, BUTTON_SIZE);
    else
        return Rectangle(_position.x, 0, BUTTON_SIZE, 192);
}

void AppBarView::Update()
{
    // Focus is granted by the focus manager directly, so polling is the only
    // way to catch every route into a button - including a touch, which never
    // passes through MoveFocus. Nothing is recorded while focus is elsewhere,
    // so the last button used here survives a trip into the content.
    for (int i = 0; i < _startButtonCount + _endButtonCount; i++)
    {
        if (_buttons[i] && _buttons[i]->IsFocused())
        {
            _lastFocusedButtonIndex = i;
            break;
        }
    }

    if (_orientation == Orientation::Horizontal)
        UpdateButtonPositionsHorizontal();
    else
        UpdateButtonPositionsVertical();

    ViewContainer::Update();
}

SharedPtr<View> AppBarView::MoveFocus(const SharedPtr<View>& currentFocus, FocusMoveDirection direction, View* source)
{
    int idx = FindButtonIndex(currentFocus.GetPointer());
    if (idx >= 0)
    {
        if (_orientation == Orientation::Horizontal)
        {
            if (direction == FocusMoveDirection::Left)
                idx--;
            else if (direction == FocusMoveDirection::Right)
                idx++;
            else
                return View::MoveFocus(currentFocus, direction, this);
        }
        else
        {
            if (direction == FocusMoveDirection::Up)
                idx--;
            else if (direction == FocusMoveDirection::Down)
                idx++;
            else
                return View::MoveFocus(currentFocus, direction, this);
        }

        if (idx < 0 || idx >= _startButtonCount + _endButtonCount)
            return nullptr;

        return _buttons[idx];
    }
    else if ((_orientation == Orientation::Horizontal && (direction == FocusMoveDirection::Up || direction == FocusMoveDirection::Down
                  || direction == FocusMoveDirection::Left || direction == FocusMoveDirection::Right)) ||
             (_orientation == Orientation::Vertical && (direction == FocusMoveDirection::Left || direction == FocusMoveDirection::Right)))
    {
        // Return to wherever the user last was in this bar rather than to
        // whichever button happens to be geometrically nearest the thing they
        // came from. Nearest-button made entry depend on where the content's
        // focus sat, so coming back from the list always landed on the same
        // middle button no matter which one had been used before.
        if (_lastFocusedButtonIndex >= 0
            && _lastFocusedButtonIndex < _startButtonCount + _endButtonCount)
        {
            return _buttons[_lastFocusedButtonIndex];
        }
        return _buttons[0];
    }
    else
        return View::MoveFocus(currentFocus, direction, this);
}

void AppBarView::Focus(FocusManager& focusManager, int button)
{
    _lastFocusedButtonIndex = button;
    focusManager.Focus(_buttons[button]);
}

int AppBarView::GetFocusedButtonIndex() const
{
    for (int i = 0; i < _startButtonCount + _endButtonCount; i++)
    {
        if (_buttons[i] && _buttons[i]->IsFocused())
        {
            return i;
        }
    }
    return -1;
}

void AppBarView::Focus(FocusManager& focusManager)
{
    int button = (_lastFocusedButtonIndex >= 0
        && _lastFocusedButtonIndex < _startButtonCount + _endButtonCount)
        ? _lastFocusedButtonIndex : 0;
    focusManager.Focus(_buttons[button]);
}

void AppBarView::UpdateButtonPositionsHorizontal()
{
    for (int i = 0; i < _startButtonCount; i++)
        _buttons[i]->SetPosition(i * BUTTON_SIZE, _position.y);

    int x = 256;
    for (int i = _startButtonCount + _endButtonCount - 1; i >= _startButtonCount; i--)
    {
        x -= BUTTON_SIZE;
        _buttons[i]->SetPosition(x, _position.y);
    }
}

void AppBarView::UpdateButtonPositionsVertical()
{
    for (int i = 0; i < _startButtonCount; i++)
        _buttons[i]->SetPosition(_position.x, i * BUTTON_SIZE);

    int y = 192;
    for (int i = _startButtonCount + _endButtonCount - 1; i >= _startButtonCount; i--)
    {
        y -= BUTTON_SIZE;
        _buttons[i]->SetPosition(_position.x, y);
    }
}

int AppBarView::FindButtonIndex(const View* view)
{
    for (int i = 0; i < _startButtonCount + _endButtonCount; i++)
    {
        if (_buttons[i].GetPointer() == view)
        {
            return i;
        }
    }

    return -1;
}

bool AppBarView::IsAtBottom() const
{
    // A horizontal bar anywhere other than flush with the top of the screen is
    // the bottom icon row; every other layout puts it at y 0 or runs it
    // vertically down a side.
    return _orientation == Orientation::Horizontal && GetPosition().y != 0;
}

void AppBarView::SetIconRowColors(const Rgb<8, 8, 8>& topColor, const Rgb<8, 8, 8>& bottomColor)
{
    _iconRowTopColor = topColor;
    _iconRowBottomColor = bottomColor;
}

void AppBarView::InitVram(const VramContext& vramContext)
{
    ViewContainer::InitVram(vramContext);

    if (!IsAtBottom())
    {
        return;
    }

    const auto objVramManager = vramContext.GetObjVramManager();
    if (!objVramManager)
    {
        return;
    }

    // Rounded up to a 128-byte multiple - OAM tile indices are computed as
    // vramOffset >> 7, so every allocation has to both start at and be sized in
    // multiples of 128 bytes to keep later allocations aligned.
    u32 stripAllocLen = (iconRowStripTilesLen + 127) & ~127u;
    _iconRowStripVramOffset = objVramManager->Alloc(stripAllocLen);
    dma_ntrCopy32(3, iconRowStripTiles,
        objVramManager->GetVramAddress(_iconRowStripVramOffset), iconRowStripTilesLen);

    // The button icons carry baked-in antialiasing: their edge pixels are
    // intermediate steps of a gradient whose low end is whatever is behind
    // them, so with a strip painted back there they have to be told about it or
    // their edges resolve to the scheme's background colour and halo. Done here
    // rather than in SetIconRowColors because the subclass creates the buttons,
    // so they don't exist yet while this class's constructor runs.
    //
    // Sampled from the strip's own palette at the gradient index the icons'
    // vertical midpoint lands on, so it follows the strip automatically instead
    // of needing a hand-tuned fraction kept in sync with the offsets.
    IconRowPalette palette(_iconRowTopColor, _iconRowBottomColor);
    int iconMidRow = ((ICON_BUTTON_ICON_Y_OFFSET + ICON_BUTTON_ICON_Y_OFFSET + ICON_BUTTON_ICON_SIZE) / 2)
        - ICON_ROW_STRIP_Y_OFFSET;
    int rampRows = ICON_ROW_STRIP_HEIGHT - ICON_ROW_GROOVE_ROWS;
    int gradientSpan = (int)(IconRowPalette::GRADIENT_MAX_INDEX - IconRowPalette::GRADIENT_MIN_INDEX);
    int gradientIndex = std::clamp(
        (int)IconRowPalette::GRADIENT_MIN_INDEX
            + ((iconMidRow - ICON_ROW_GROOVE_ROWS) * gradientSpan / std::max(1, rampRows - 1)),
        (int)IconRowPalette::GRADIENT_MIN_INDEX, (int)IconRowPalette::GRADIENT_MAX_INDEX);
    Rgb<8, 8, 8> backdropColor = palette.GetColorAtGradientIndex((u32)gradientIndex);
    for (int i = 0; i < _startButtonCount + _endButtonCount; i++)
    {
        if (_buttons[i])
        {
            _buttons[i]->SetBackdropColor(backdropColor);
            _buttons[i]->SetUsePillSelector(true);
        }
    }
}

void AppBarView::DrawIconRowStrip(GraphicsContext& graphicsContext)
{
    // Sprites rather than a 3D quad, because on sprite-based themes (Material)
    // the list rows behind this are sprites too, and nothing on a BG or 3D
    // layer can cover a sprite at any priority. Drawn before the buttons so
    // they take lower OAM indices and land in front of it (OamManager hands
    // slots out from a descending stack, lower index winning the tie).
    //
    // Four sprites because 64px is the widest an OBJ can be, and 256 / 64 = 4.
    // They carry the groove as well as the gradient - see IconRowPalette - so
    // that's the whole icon row background for four OAM entries and one palette
    // row, on a screen where the OAM table only holds 128 entries in total.
    const int stripTopY = GetPosition().y + ICON_ROW_STRIP_Y_OFFSET;
    u32 stripPaletteRow = graphicsContext.GetPaletteManager().AllocRow(
        IconRowPalette(_iconRowTopColor, _iconRowBottomColor), stripTopY, 192);
    auto stripOam = graphicsContext.GetOamManager().AllocOams(4);
    for (int i = 0; i < 4; i++)
    {
        OamBuilder::OamWithSize<64, 32>(i * 64, stripTopY, _iconRowStripVramOffset >> 7)
            .WithPalette16(stripPaletteRow)
            .WithPriority(graphicsContext.GetPriority())
            .Build(stripOam[i]);
    }
}

void AppBarView::Draw(GraphicsContext& graphicsContext)
{
    if (IsAtBottom())
    {
        DrawIconRowStrip(graphicsContext);
    }
    ViewContainer::Draw(graphicsContext);
}
