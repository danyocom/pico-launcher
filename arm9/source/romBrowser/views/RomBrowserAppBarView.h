#pragma once
#include "gui/views/ViewContainer.h"
#include "AppBarView.h"
#include "../viewModels/RomBrowserAppBarViewModel.h"

class RomBrowserDisplayMode;
class IRomBrowserViewFactory;

class RomBrowserAppBarView : public ViewContainer
{
    SHARED_ONLY(RomBrowserAppBarView)

public:
    void InitVram(const VramContext& vramContext) override;

private:
    /// @brief The favorites button swaps between an outline and a filled heart
    ///        rather than only changing tint, so its on state is legible
    ///        without relying on colour alone. Both are uploaded once; toggling
    ///        just points the button at the other one, so it costs no extra
    ///        sprite - only the second tile's 128 bytes of OBJ VRAM.
    u32 _heartIconVramOffset = 0;
    u32 _heartFilledIconVramOffset = 0;

public:
    void Update() override;

    Rectangle GetBounds() const override
    {
        return Rectangle(0, 0, 256, 192);
    }

    SharedPtr<View> MoveFocus(const SharedPtr<View>& currentFocus, FocusMoveDirection direction, View* source) override;

    void Focus(FocusManager& focusManager)
    {
        // Not button 0: returning here after a filter toggle rebuilt the list
        // would otherwise dump focus on the back button every time, instead of
        // leaving it on the filter the user just pressed so they can toggle it
        // straight back off.
        _appBarView->Focus(focusManager);
    }

    void Focus(FocusManager& focusManager, int button)
    {
        _appBarView->Focus(focusManager, button);
    }

    int GetFocusedButtonIndex() const { return _appBarView->GetFocusedButtonIndex(); }

private:
    enum AppBarButton
    {
        APP_BAR_BUTTON_BACK = 0,

        APP_BAR_BUTTON_RECENT,
        APP_BAR_BUTTON_FAVORITE,
        APP_BAR_BUTTON_COMPLETED,
        APP_BAR_BUTTON_DELETE,
        APP_BAR_BUTTON_DISPLAY_SETTINGS,
        // APP_BAR_BUTTON_SETTINGS
    };

    RomBrowserAppBarViewModel* _viewModel;
    SharedPtr<AppBarView> _appBarView;

    RomBrowserAppBarView(
        RomBrowserAppBarViewModel* viewModel, const RomBrowserDisplayMode& displayMode,
        const IRomBrowserViewFactory* romBrowserViewFactory);
};
