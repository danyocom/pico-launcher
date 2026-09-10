#include "common.h"
#include <libtwl/dma/dmaNitro.h>
#include "gui/VramContext.h"
#include "gui/IVramManager.h"
#include "BottomSheetView.h"

BottomSheetView::BottomSheetView(const MaterialColorScheme* materialColorScheme)
{
}

void BottomSheetView::InitVram(const VramContext& vramContext)
{
    // Needs to keep cascading to children: without this override every
    // subclass's BottomSheetView::InitVram() call would resolve straight to
    // ViewContainer::InitVram() anyway, but keeping it explicit documents that
    // the cascade is required - labels and icons allocate their VRAM here.
    ViewContainer::InitVram(vramContext);
}

void BottomSheetView::Update()
{
    // Top-right corner, a small margin in from both edges - tracks
    // _position.y so it stays put relative to the sheet as it slides open
    // and closed.

    // Same reasoning as InitVram above: this override needs to keep
    // cascading to children too, or every label's marquee/ellipsis handling
    // (LabelView::Update) silently stops running in every dialog.
    ViewContainer::Update();
}

void BottomSheetView::HandlePenDown(const Point& touchPoint, FocusManager& focusManager)
{
    DialogView::HandlePenDown(touchPoint, focusManager);

    if (!GetBounds().Contains(touchPoint))
    {
        _oobPenDown = true;
    }
}

void BottomSheetView::HandlePenMove(const Point& touchPoint, FocusManager& focusManager)
{
    DialogView::HandlePenMove(touchPoint, focusManager);

    if (GetBounds().Contains(touchPoint))
    {
        _oobPenDown = false;
    }
}

void BottomSheetView::HandlePenUp(const Point& lastTouchPoint, FocusManager& focusManager)
{
    DialogView::HandlePenUp(lastTouchPoint, focusManager);

    if (_oobPenDown && !GetBounds().Contains(lastTouchPoint))
    {
        Close();
    }

    _oobPenDown = false;
}
