#pragma once
#include "core/SharedPtr.h"
#include "../viewModels/RomBrowserViewModel.h"
#include "gui/views/RecyclerViewBase.h"
#include "../FileRecyclerAdapter.h"
#include "gui/views/View.h"
#include "../DisplayMode/RomBrowserDisplayMode.h"
#include "../Theme/IThemeFileIconFactory.h"

class IRomBrowserViewFactory;

class RomBrowserView : public ViewContainer
{
    SHARED_ONLY(RomBrowserView)

public:
    void InitVram(const VramContext& vramContext) override;
    void Update() override;

    Rectangle GetBounds() const override
    {
        return Rectangle(0, 0, 256, 192);
    }

    bool Focus(FocusManager& focusManager)
    {
        if (!_fileRecyclerAdapter || _fileRecyclerAdapter->GetItemCount() == 0)
            return false;

        _fileGridView->Focus(focusManager);
        return true;
    }

    SharedPtr<View> MoveFocus(
        const SharedPtr<View>& currentFocus, FocusMoveDirection direction, View* source) override;

private:
    SharedPtr<RomBrowserViewModel> _viewModel;
    SharedPtr<RecyclerViewBase> _fileGridView;
    SharedPtr<FileRecyclerAdapter> _fileRecyclerAdapter;
    bool _isVertical;
    bool _isAppBarAtEnd;
    /// @brief Whether this layout reserves screen space for a bottom icon row.
    ///        Kept alongside the two flags above because the display mode isn't
    ///        held past construction.
    bool _hasReservedBottomSpace;

    RomBrowserView(
        SharedPtr<RomBrowserViewModel> viewModel,
        const RomBrowserDisplayMode& displayMode,
        const IThemeFileIconFactory* themeFileIconFactory,
        const IRomBrowserViewFactory* romBrowserViewFactory,
        VBlankTextureLoader* vblankTextureLoader);
};
