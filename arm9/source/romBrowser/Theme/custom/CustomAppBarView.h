#pragma once
#include "themes/custom/CustomThemeInfo.h"
#include "../../views/AppBarView.h"

class CustomAppBarView : public AppBarView
{
    SHARED_ONLY(CustomAppBarView)

public:
    void Draw(GraphicsContext& graphicsContext) override;
    void InitVram(const VramContext& vramContext) override;

private:
    u32 _scrimTexVramOffset = 0;
    u32 _scrimPlttVramOffset = 0;

    CustomAppBarView(int x, int y, Orientation orientation,
        int startButtonCount, int endButtonCount, const CustomThemeInfo* customThemeInfo,
        const MaterialColorScheme* materialColorScheme,
        u32 scrimTexVramOffset, u32 scrimPlttVramOffset);
};
