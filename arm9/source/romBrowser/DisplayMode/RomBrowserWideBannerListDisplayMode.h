#pragma once
#include "gui/views/RecyclerView.h"
#include "RomBrowserDisplayMode.h"
#include "WideBannerListFileRecyclerAdapter.h"

// The banner list, rearranged for the app bar (nav/settings/favorite/etc)
// to run along the bottom of the screen instead of down the left side. See
// RomBrowserBannerListDisplayMode for the standard layout this is based on.
class RomBrowserWideBannerListDisplayMode : public RomBrowserDisplayMode
{
public:
    static const RomBrowserWideBannerListDisplayMode sInstance;

    bool IsVertical() const override { return false; }
    bool IsAppBarAtEnd() const override { return true; }
    int GetReservedBottomSpace() const override { return 42; }

    // The bar sits flush against the bottom edge (192 - 32 puts a 32px-tall
    // bar there), nudged down 5px. Everything that has to line up with the
    // icon row reads this rather than repeating the arithmetic.
    int GetIconRowTopY() const override { return 192 - 32 + 5; }

    SharedPtr<AppBarView> CreateAppBarView(const IRomBrowserViewFactory* romBrowserViewFactory,
        int startButtonCount, int endButtonCount) const override
    {
        // Horizontal orientation's bounds/button row follow _position.y
        // directly (see AppBarView::GetBounds/UpdateButtonPositionsHorizontal) -
        // 192 - 32 puts the 32px-tall bar flush against the bottom edge
        // instead of the top.
        return romBrowserViewFactory->CreateAppBarView(0, GetIconRowTopY(),
            AppBarView::Orientation::Horizontal, startButtonCount, endButtonCount);
    }

    SharedPtr<RecyclerViewBase> CreateRecyclerView(const IRomBrowserViewFactory* romBrowserViewFactory) const override
    {
        // Deliberately generous rather than sized to guarantee zero overlap
        // with the icon row: RecyclerView supports free touch-drag
        // scrolling to arbitrary (non-row-aligned) offsets, so a fully
        // overlap-proof height would need to assume a worst case of a row's
        // full 62px sprite overhang landing right at this boundary, leaving
        // very little of the screen for the list itself. Showing a
        // partially-visible next row (even if its sprite overhangs into the
        // reserved space sometimes) is more useful than hiding it entirely
        // to guarantee that never happens - it's the only cue that there's
        // more below.
        // Tall enough to reach the icon row, NOT stopping at the old 130. The
        // recycler only binds and draws rows whose logical bounds fall inside
        // its own height, but a row's artwork stands taller than its row and
        // stays visible for a good many pixels below it. Cutting the view off
        // at 130 meant the bottom row was dropped while its art still had
        // somewhere to show, so it vanished mid-scroll instead of sliding
        // under the icon row. Rows that reach the icon row are covered by its
        // opaque strip, which is what makes running the view right up to it
        // safe.
        auto recyclerView = RecyclerView::CreateShared(0, 0, 256, GetIconRowTopY(),
            RecyclerView::Mode::VerticalList);
        // A small margin on each side (rather than 0) so a sliver of the
        // theme's background shows around the list, and item width below is
        // sized to match. Tighter vertical padding/spacing than the standard
        // banner list's SetPadding(0, 3)/SetItemSpacing(0, 3) - the viewport
        // above lost height to the bottom bar, so rows are packed closer to
        // keep a similar number of them on screen at once.
        //recyclerView->SetPadding(4, 1);
        //recyclerView->SetItemSpacing(0, 1);
        recyclerView->SetPadding(7, 3);
        // Vertical gap comes from the theme: the Material cell's art stands
        // taller than the row it belongs to, so it needs more of a gap than the
        // custom engine's does.
        recyclerView->SetItemSpacing(0, romBrowserViewFactory->GetWideBannerListItemSpacing());
        return recyclerView;
    }

    SharedPtr<FileRecyclerAdapter> CreateRecyclerAdapter(
        RomBrowserViewModel* viewModel, const IThemeFileIconFactory* themeFileIconFactory,
        const IRomBrowserViewFactory* romBrowserViewFactory, VBlankTextureLoader* vblankTextureLoader) const override
    {
        return SharedPtr<WideBannerListFileRecyclerAdapter>::MakeShared(viewModel->GetRomBrowserController(),
            &viewModel->GetFileInfoManager(), viewModel->GetIoTaskQueue(), themeFileIconFactory,
            romBrowserViewFactory, vblankTextureLoader);
    }

private:
    constexpr RomBrowserWideBannerListDisplayMode() { }
};
