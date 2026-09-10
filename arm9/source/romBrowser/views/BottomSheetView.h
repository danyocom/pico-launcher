#pragma once
#include <algorithm>
#include "animation/Animator.h"
#include "gui/views/DialogView.h"

class MaterialColorScheme;

class BottomSheetView : public DialogView
{
public:
    BottomSheetView(const MaterialColorScheme* materialColorScheme);

    void InitVram(const VramContext& vramContext) override;
    void Update() override;
    void HandlePenDown(const Point& touchPoint, FocusManager& focusManager) override;
    void HandlePenMove(const Point& touchPoint, FocusManager& focusManager) override;
    void HandlePenUp(const Point& lastTouchPoint, FocusManager& focusManager) override;

    Rectangle GetBounds() const override
    {
        // Clamped to 0: while opening/closing, _position.y sweeps between 32
        // (or 0 - see DialogPresenter::Update) and 192 every time, the same
        // range every layout uses regardless of bottomInset (see
        // DialogPresenter::Update's BottomSheetClosing case for why). For
        // part of that sweep, on a layout with a nonzero bottomInset,
        // _position.y is greater than (192 - _bottomInset) - without the
        // clamp this briefly goes negative, which is undefined for
        // everything downstream that reads this rectangle (clip areas,
        // visibility checks) and was corrupting label rendering.
        return Rectangle(_position.x, _position.y, 256 - _position.x,
            std::max(0, (192 - _bottomInset) - _position.y));
    }

    Rectangle GetFullyCoveredArea() const override
    {
        return Rectangle(_position.x, _position.y + 12, 256 - _position.x, 148 - _bottomInset);
    }

    constexpr DialogType GetDialogType() const override { return DialogType::BottomSheet; }

    void SetBottomInset(int bottomInset) override
    {
        _bottomInset = bottomInset;
    }

protected:
    virtual void Close() = 0;

private:
    bool _oobPenDown = false;
    int _bottomInset = 0;
};
