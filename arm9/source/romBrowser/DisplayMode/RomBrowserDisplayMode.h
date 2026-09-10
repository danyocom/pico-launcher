#pragma once
#include <memory>
#include "../views/AppBarView.h"
#include "gui/views/RecyclerViewBase.h"
#include "../FileRecyclerAdapter.h"
#include "../viewModels/RomBrowserViewModel.h"
#include "../Theme/IRomBrowserViewFactory.h"

class VBlankTextureLoader;
class ICoverRepository;

class RomBrowserDisplayMode
{
public:
    virtual bool IsVertical() const = 0;
    /// @brief Whether the app bar sits at the "end" of its axis - the right
    ///        edge for a vertical bar, or the bottom edge for a horizontal
    ///        one - rather than the "start" (left/top). Every built-in
    ///        layout keeps its bar at the start, so this only needs
    ///        overriding by a layout (like the horizontal list) that puts it
    ///        at the opposite edge; the focus-navigation direction between
    ///        the bar and the content flips accordingly.
    virtual bool IsAppBarAtEnd() const { return false; }
    /// @brief Screen height, at the bottom edge, that this layout's app bar
    ///        (plus its usual 10px gap) reserves and that popup dialogs
    ///        should leave uncovered - so the horizontal list layout's nav/
    ///        settings/favorite/etc icons stay visible and reachable while a
    ///        dialog (Settings, Cheats, ...) is open. 0 for every layout
    ///        whose app bar isn't down there.
    virtual int GetReservedBottomSpace() const { return 0; }

    /// @brief Y coordinate of the top edge of this layout's bottom icon row -
    ///        i.e. where the app bar's buttons actually start, which is
    ///        distinct from (and lower than) 192 - GetReservedBottomSpace(),
    ///        since that figure also includes the gap above the bar. The one
    ///        place this boundary is defined, so the pieces that have to line
    ///        up with it (the separator line, the opaque row behind the
    ///        icons, the list's clip area) all follow the bar automatically
    ///        instead of repeating the arithmetic. 192 - i.e. no icon row,
    ///        nothing to clip against - for every layout without one.
    virtual int GetIconRowTopY() const { return 192; }
    virtual bool ShowCoverOnTopScreen() const { return true; }
    virtual SharedPtr<AppBarView> CreateAppBarView(const IRomBrowserViewFactory* romBrowserViewFactory,
        int startButtonCount, int endButtonCount) const = 0;
    virtual SharedPtr<RecyclerViewBase> CreateRecyclerView(const IRomBrowserViewFactory* romBrowserViewFactory) const = 0;
    virtual SharedPtr<FileRecyclerAdapter> CreateRecyclerAdapter(
        RomBrowserViewModel* viewModel, const IThemeFileIconFactory* themeFileIconFactory,
        const IRomBrowserViewFactory* romBrowserViewFactory, VBlankTextureLoader* vblankTextureLoader) const = 0;
};
