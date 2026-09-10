#include "common.h"
#include "../Theme/IRomBrowserViewFactory.h"
#include "romBrowser/viewModels/RomBrowserItemViewModel.h"
#include "WideBannerListFileRecyclerAdapter.h"

void WideBannerListFileRecyclerAdapter::GetViewSize(int& width, int& height) const
{
    // Width: 45px wider than the standard banner list item
    // (BannerListFileRecyclerAdapter's 203) - reclaims most of the width the
    // vertical icon bar used to occupy at the left of the screen, now that
    // this layout's icons run along the bottom instead. Not the full 42px
    // freed up: the recycler now also keeps a small 4px margin on each side
    // (see RomBrowserWideBannerListDisplayMode::CreateRecyclerView) so a
    // sliver of the theme's background shows around the list.
    //
    // Height: a little shorter than the standard item's 44 - the row's
    // background is fixed-size sprite/texture art that can't be resized
    // arbitrarily (see MaterialBannerListItemView::Draw), but its 3 text
    // lines still fit down to 40, and packing rows tighter helps more of the
    // list fit above the reserved bottom bar space.
    // Asked of the theme rather than fixed here: the Material engine builds its
    // cell from whole 64px background segments, so it can only be certain
    // widths, and three segments put its right edge off the side of the screen.
    width = _romBrowserViewFactory->GetWideBannerListItemWidth();
    height = 40;
}

SharedPtr<View> WideBannerListFileRecyclerAdapter::CreateView() const
{
    return _romBrowserViewFactory->CreateWideBannerListItemView(
        std::make_unique<RomBrowserItemViewModel>(_romBrowserController), _vblankTextureLoader);
}
