#pragma once

enum class RomBrowserLayout
{
    HorizontalIconGrid,
    VerticalIconGrid,
    BannerList,
    FileList,
    CoverFlow,
    /// @brief The banner list, rearranged for the app bar icons (nav/
    ///        settings/favorite/etc) to run along the bottom of the screen
    ///        instead of down the left side. Frees up that left column, so
    ///        each row's info cell is drawn wider to use the extra space.
    WideBannerList
};