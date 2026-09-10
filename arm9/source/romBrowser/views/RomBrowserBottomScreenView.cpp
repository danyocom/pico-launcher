#include "common.h"
#include <libtwl/dma/dmaNitro.h>
#include "../viewModels/RomBrowserViewModel.h"
#include "../views/IconGridItemView.h"
#include "gui/GraphicsContext.h"
#include "gui/VramContext.h"
#include "gui/OamBuilder.h"
#include "gui/palette/GradientPalette.h"
#include "core/math/Rgb.h"
#include "backIcon.h"
#include "settingsIcon.h"
#include "heartIcon.h"
#include "recentIcon.h"
#include "listIcon.h"
#include "gui/IVramManager.h"
#include "gui/input/InputProvider.h"
#include "RomBrowserBottomScreenView.h"

RomBrowserBottomScreenView::RomBrowserBottomScreenView(
    RomBrowserBottomScreenViewModel* viewModel,
    const RomBrowserDisplayMode* displayMode,
    const IThemeFileIconFactory* themeFileIconFactory,
    const IRomBrowserViewFactory* romBrowserViewFactory,
    VBlankTextureLoader* vblankTextureLoader)
    : _viewModel(viewModel)
    , _romBrowserViewFactory(romBrowserViewFactory)
    , _romBrowserDisplayMode(displayMode)
    , _themeFileIconFactory(themeFileIconFactory)
    , _romBrowserAppBarView(RomBrowserAppBarView::CreateShared(_viewModel->GetRomBrowserAppBarViewModel(),
        *displayMode, romBrowserViewFactory))
    , _vblankTextureLoader(vblankTextureLoader)
{
    _romBrowserAppBarView->SetParent(this);
}

void RomBrowserBottomScreenView::InitVram(const VramContext& vramContext)
{
    _romBrowserAppBarView->InitVram(vramContext);
}

void RomBrowserBottomScreenView::Update()
{
    _romBrowserAppBarView->Update();
    if (_romBrowserView && _viewModel->IsRomBrowserVisible())
    {
        _romBrowserView->Update();
    }
}

void RomBrowserBottomScreenView::Draw(GraphicsContext& graphicsContext)
{
    if (_romBrowserDisplayMode->GetReservedBottomSpace() > 0)
    {
        // Content first, icon row second - the opposite of the other
        // layouts below, and the whole point of this branch.
        //
        // The custom theme's app bar already draws a genuinely
        // semi-transparent scrim across the reserved strip: a 3D quad with
        // an A5I3 (per-texel alpha) gradient texture, see
        // CustomAppBarView::Draw. That's real translucency via the 3D
        // engine's own alpha, which is completely independent of the
        // main engine's BLDCNT/BLDALPHA - the shared registers
        // DialogPresenter drives every frame, and the reason an OBJ
        // translucent sprite can't do this job.
        //
        // Translucent 3D polygons blend in submission order (SwapBuffers
        // uses GX_XLU_SORT_MANUAL - see App.cpp), so whatever is submitted
        // later paints over what came before. Drawing the app bar first
        // meant list rows overhanging into the reserved strip were
        // submitted afterwards and landed on top of that scrim instead of
        // being muted by it. Submitting the list first lets the scrim
        // blend over the overhang the way it was always meant to, and -
        // because the buttons are drawn after their own scrim inside
        // AppBarView::Draw - leaves every button's focus circle
        // (IconButton3DView::DrawSelector) fully visible on top.
        if (_romBrowserView && _viewModel->IsRomBrowserVisible())
        {
            // Deliberately NOT setting a clip area of our own here. App::Draw
            // calls DialogPresenter::ApplyClipArea before this, which installs
            // an INVERSE clip over the area an open dialog fully covers, so
            // every row hidden behind that dialog is culled instead of
            // allocating OAM entries nothing will ever see. Overwriting it
            // with a normal clip (as this did while the icons still clipped
            // themselves) silently disabled that culling, which is enough to
            // exhaust the 128-entry OAM table on sprite-heavy themes like
            // Material - OamManager::AllocOams doesn't bounds check, so the
            // overflow just shows up as dialog content failing to render.
            _romBrowserView->Draw(graphicsContext);
        }

        // A dialog's background (BG1, always priority 1 - see
        // DialogPresenter::InitVram) always extends down past the bottom of
        // the screen once open, regardless of GetReservedBottomSpace(); it
        // isn't shrunk to make room, only scrolled (DialogPresenter::Update).
        // A layout that reserves bottom space wants its app bar to stay
        // visible and usable through that anyway, so draw it at a priority
        // that wins the tie against BG1 instead - sprites beat a same-
        // priority BG layer on this hardware.
        u32 oldPriority = graphicsContext.SetPriority(1);
        // The icon row's own background strip and groove line are drawn by the
        // app bar itself (AppBarView::Draw), so they follow whatever colours
        // the active theme supplies rather than being fixed here.
        _romBrowserAppBarView->Draw(graphicsContext);
        graphicsContext.SetPriority(oldPriority);
    }
    else
    {
        _romBrowserAppBarView->Draw(graphicsContext);
        if (_romBrowserView && _viewModel->IsRomBrowserVisible())
        {
            _romBrowserView->Draw(graphicsContext);
        }
    }
}

void RomBrowserBottomScreenView::VBlank()
{
    _romBrowserAppBarView->VBlank();
    if (_romBrowserView && _viewModel->IsRomBrowserVisible())
    {
        _romBrowserView->VBlank();
    }
}

SharedPtr<View> RomBrowserBottomScreenView::MoveFocus(const SharedPtr<View>& currentFocus, FocusMoveDirection direction, View* source)
{
    if (!currentFocus)
    {
        return nullptr;
    }
    if (source == _romBrowserAppBarView.GetPointer())
    {
        if (_romBrowserDisplayMode->IsVertical())
        {
            if (direction == FocusMoveDirection::Right)
            {
                return _romBrowserView->MoveFocus(currentFocus, direction, this);
            }
        }
        else
        {
            // a horizontal bar at the bottom (the horizontal list layout) is
            // reached from the content by pressing Down, not Up
            FocusMoveDirection towardContent = _romBrowserDisplayMode->IsAppBarAtEnd()
                ? FocusMoveDirection::Up : FocusMoveDirection::Down;
            // Reserved-space layouts (currently just the horizontal list)
            // treat Up and Down as interchangeable ways back to the list -
            // the icon row is a single horizontal strip with no meaningful
            // "up vs down" of its own, and Left/Right own all in-bar
            // navigation there instead (see the matching change below), so
            // leaving it vertically in either direction should return to
            // content. Other horizontal-app-bar layouts (icon grid, cover
            // flow) keep the original single-direction behavior.
            bool reservedSpaceLayout = _romBrowserDisplayMode->GetReservedBottomSpace() > 0;
            bool verticalExit = reservedSpaceLayout
                ? (direction == FocusMoveDirection::Up || direction == FocusMoveDirection::Down)
                : (direction == towardContent);
            if (verticalExit)
            {
                return _romBrowserView->MoveFocus(currentFocus, direction, this);
            }
        }
        return nullptr;
    }
    else if (source == _romBrowserView.GetPointer())
    {
        if (_romBrowserDisplayMode->IsVertical())
        {
            if (direction == FocusMoveDirection::Left)
            {
                return _romBrowserAppBarView->MoveFocus(currentFocus, direction, this);
            }
        }
        else
        {
            FocusMoveDirection towardAppBar = _romBrowserDisplayMode->IsAppBarAtEnd()
                ? FocusMoveDirection::Down : FocusMoveDirection::Up;
            bool reservedSpaceLayout = _romBrowserDisplayMode->GetReservedBottomSpace() > 0;
            // Reserved-space layouts route Left/Right to the icon row
            // exclusively, not the natural scroll-to-the-end towardAppBar
            // direction - Up/Down always stays within the list and never
            // leaves it (matches the change above: it's the only way back
            // in, so it needs to also be the only way out). Other
            // horizontal-app-bar layouts keep the original single-direction
            // behavior, scoped the same way appBarSideEntry used to be.
            bool appBarEntry = reservedSpaceLayout
                ? (direction == FocusMoveDirection::Left || direction == FocusMoveDirection::Right)
                : (direction == towardAppBar);
            if (appBarEntry)
            {
                return _romBrowserAppBarView->MoveFocus(currentFocus, direction, this);
            }
        }
        return nullptr;
    }
    return nullptr;
}

bool RomBrowserBottomScreenView::HandleInput(const InputProvider& inputProvider, FocusManager& focusManager)
{
    if (inputProvider.Triggered(InputKey::B))
    {
        _viewModel->NavigateUp();
        return true;
    }
    return View::HandleInput(inputProvider, focusManager);
}

void RomBrowserBottomScreenView::HandlePenDown(const Point& touchPoint, FocusManager& focusManager)
{
    _romBrowserAppBarView->HandlePenDown(touchPoint, focusManager);
    if (_romBrowserView && _viewModel->IsRomBrowserVisible())
    {
        _romBrowserView->HandlePenDown(touchPoint, focusManager);
    }
}

void RomBrowserBottomScreenView::HandlePenMove(const Point& touchPoint, FocusManager& focusManager)
{
    _romBrowserAppBarView->HandlePenMove(touchPoint, focusManager);
    if (_romBrowserView && _viewModel->IsRomBrowserVisible())
    {
        _romBrowserView->HandlePenMove(touchPoint, focusManager);
    }
}

void RomBrowserBottomScreenView::HandlePenUp(const Point& lastTouchPoint, FocusManager& focusManager)
{
    _romBrowserAppBarView->HandlePenUp(lastTouchPoint, focusManager);
    if (_romBrowserView && _viewModel->IsRomBrowserVisible())
    {
        _romBrowserView->HandlePenUp(lastTouchPoint, focusManager);
    }
}

void RomBrowserBottomScreenView::RomBrowserViewModelInvalidated(const VramContext& vramContext)
{
    if (_viewModel->GetRomBrowserViewModel().IsValid())
    {
        _romBrowserView = RomBrowserView::CreateShared(
            _viewModel->GetRomBrowserViewModel(), *_romBrowserDisplayMode,
            _themeFileIconFactory, _romBrowserViewFactory, _vblankTextureLoader);
        _romBrowserView->SetParent(this);
        _romBrowserView->InitVram(vramContext);
    }
    else
    {
        _romBrowserView.Reset();
    }
}
