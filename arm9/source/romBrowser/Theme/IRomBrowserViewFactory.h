#pragma once
#include <memory>
#include "core/SharedPtr.h"
#include "core/math/Point.h"
#include "../views/IconGridItemView.h"
#include "../views/BannerListItemView.h"
#include "../views/AppBarView.h"
#include "../views/BannerView.h"
#include "gui/views/RecyclerViewBase.h"

class VramContext;
class VBlankTextureLoader;
class RomBrowserViewModel;
class IThemeFileIconFactory;
class FileRecyclerAdapter;
class IRomBrowserItemViewModel;

// position is the top-left corner for the game count pill and the top-right
// corner for the launch info pill (that one grows leftward); hidden suppresses
// the pill, its text and its icons entirely
struct TopStripElementLayout
{
    Point position;
    bool hidden;
};

class IRomBrowserViewFactory
{
public:
    virtual ~IRomBrowserViewFactory() = 0;

    virtual SharedPtr<IconGridItemView> CreateIconGridItemView(std::unique_ptr<IRomBrowserItemViewModel> viewModel) const = 0;
    virtual IconGridItemView::VramToken UploadIconGridItemViewGraphics(
        const VramContext& vramContext) const { return IconGridItemView::VramToken(0); }

    virtual SharedPtr<BannerListItemView> CreateBannerListItemView(std::unique_ptr<IRomBrowserItemViewModel> viewModel,
        VBlankTextureLoader* vblankTextureLoader) const = 0;
    virtual BannerListItemView::VramToken UploadBannerListItemViewGraphics(
        const VramContext& vramContext) const { return BannerListItemView::VramToken(0); }

    /// @brief The wider item cell used by the horizontal list layout (see
    ///        RomBrowserWideBannerListDisplayMode). Draws with the same
    ///        uploaded graphics as CreateBannerListItemView/
    ///        UploadBannerListItemViewGraphics above - there is no separate
    ///        upload method for it.
    virtual SharedPtr<BannerListItemView> CreateWideBannerListItemView(std::unique_ptr<IRomBrowserItemViewModel> viewModel,
        VBlankTextureLoader* vblankTextureLoader) const = 0;

    /// @brief Width of a wide banner list item, and the vertical gap between
    ///        items. Theme-specific because the cell art differs between the
    ///        engines: the Material one is built from fixed 64px background
    ///        segments, so its width can only move in whole segments, and it
    ///        stands 48px tall against a 40px row, which needs more of a gap
    ///        than the custom engine's cell does.
    virtual int GetWideBannerListItemWidth() const { return 250; }
    virtual int GetWideBannerListItemSpacing() const { return 3; }

    virtual SharedPtr<AppBarView> CreateAppBarView(int x, int y, AppBarView::Orientation orientation,
        int startButtonCount, int endButtonCount) const = 0;

    virtual SharedPtr<BannerView> CreateFileInfoView() const = 0;

    virtual SharedPtr<RecyclerViewBase> CreateCoverFlowRecyclerView() const = 0;

    virtual SharedPtr<FileRecyclerAdapter> CreateCoverFlowRecyclerAdapter(
        RomBrowserViewModel* viewModel, const IThemeFileIconFactory* themeFileIconFactory,
        VBlankTextureLoader* vblankTextureLoader) const = 0;

    virtual Point GetTopCoverPosition() const = 0;

    virtual TopStripElementLayout GetTopGameCountLayout() const = 0;
    virtual TopStripElementLayout GetTopLaunchInfoLayout() const = 0;
};

inline IRomBrowserViewFactory::~IRomBrowserViewFactory() { }
