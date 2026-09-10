#pragma once
#include "BannerListFileRecyclerAdapter.h"

// Same item/graphics pipeline as BannerListFileRecyclerAdapter (InitVram and
// BindView are inherited as-is - the wider item view reuses the same
// uploaded background tiles, just with one extra repeated middle segment),
// just wider - used by RomBrowserWideBannerListDisplayMode, which frees up
// the width the vertical icon bar used to occupy by moving those icons into
// a bar along the bottom instead.
class WideBannerListFileRecyclerAdapter : public BannerListFileRecyclerAdapter
{
public:
    using BannerListFileRecyclerAdapter::BannerListFileRecyclerAdapter;

    void GetViewSize(int& width, int& height) const override;
    SharedPtr<View> CreateView() const override;
};
