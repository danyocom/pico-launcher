#pragma once
#include <memory>
#include "gui/views/ViewContainer.h"
#include "IconButtonView.h"

class MaterialColorScheme;

class AppBarView : public ViewContainer
{
public:
    enum class Orientation
    {
        Horizontal,
        Vertical
    };

    void SetButtonIcon(int button, u32 vramOffset)
    {
        _buttons[button]->SetIconVramOffset(vramOffset);
    }

    void SetButtonAction(int button, IconButtonView::button_action_t action, void* arg)
    {
        _buttons[button]->SetAction(action, arg);
    }

    void SetButtonLongAction(int button, IconButtonView::button_action_t longAction)
    {
        _buttons[button]->SetLongAction(longAction);
    }

    void SetButtonEnabled(int button, bool enabled)
    {
        _buttons[button]->SetEnabled(enabled);
    }

    void SetButtonIconColorOverride(int button, const Rgb<8, 8, 8>& color)
    {
        _buttons[button]->SetIconColorOverride(color);
    }

    void ClearButtonIconColorOverride(int button)
    {
        _buttons[button]->ClearIconColorOverride();
    }

    Rectangle GetBounds() const override;
    void Update() override;
    void InitVram(const VramContext& vramContext) override;
    void Draw(GraphicsContext& graphicsContext) override;
    SharedPtr<View> MoveFocus(const SharedPtr<View>& currentFocus, FocusMoveDirection direction, View* source) override;
    void Focus(FocusManager& focusManager, int button);

    /// @brief Focuses whichever button was last focused, rather than always
    ///        starting from the first one. Keeps the highlight where the user
    ///        left it when focus returns here - after toggling a filter, or
    ///        after stepping away into the content and back.
    void Focus(FocusManager& focusManager);

    /// @brief Index of the button currently holding focus, or -1 when focus is
    ///        elsewhere. Read before this view is torn down so the replacement
    ///        can pick focus back up on the same button.
    int GetFocusedButtonIndex() const;

    constexpr Orientation GetOrientation() const { return _orientation; }

    /// @brief Sets the gradient the bottom icon row's background strip is
    ///        filled with, top colour to bottom. Only has any effect for a
    ///        horizontal bar positioned away from the top of the screen; every
    ///        other layout draws no strip at all. Subclasses call this with
    ///        whatever their theme provides - see CustomAppBarView (theme.json
    ///        "iconRow") and MaterialAppBarView (the colour scheme).
    void SetIconRowColors(const Rgb<8, 8, 8>& topColor, const Rgb<8, 8, 8>& bottomColor);

protected:
    Orientation _orientation;
    std::unique_ptr<SharedPtr<IconButtonView>[]> _buttons;
    int _startButtonCount;
    int _endButtonCount;

    /// @brief True when this bar sits at the bottom of the screen rather than
    ///        the top or down a side - i.e. when it gets a background strip.
    bool IsAtBottom() const;

    /// @brief Index of the button focus should return to. Tracked from every
    ///        route into a button - d-pad, touch or programmatic - by polling
    ///        in Update, since focus is granted by the focus manager without
    ///        going through this class.
    int _lastFocusedButtonIndex = 0;

private:
    const MaterialColorScheme* _materialColorScheme;
    Rgb<8, 8, 8> _iconRowTopColor;
    Rgb<8, 8, 8> _iconRowBottomColor;
    u32 _iconRowStripVramOffset = 0;

    void DrawIconRowStrip(GraphicsContext& graphicsContext);

protected:

    void UpdateButtonPositionsHorizontal();
    void UpdateButtonPositionsVertical();
    int FindButtonIndex(const View* view);

    AppBarView(int x, int y, Orientation orientation,
        int startButtonCount, int endButtonCount, const MaterialColorScheme* materialColorScheme);
};
