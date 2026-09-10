#pragma once
#include "ViewContainer.h"
#include "DialogType.h"

/// @brief View meant to be displayed as a dialog on top of other content.
class DialogView : public ViewContainer
{
public:
    /// @brief Gets the type of dialog.
    /// @return The type of dialog.
    virtual DialogType GetDialogType() const = 0;

    /// @brief Moves the focus to this dialog.
    /// @param focusManager The focus manager to use.
    virtual void Focus(FocusManager& focusManager) = 0;

    /// @brief Gets the area of the screen that will be fully covered by
    ///        this dialog for the purpose of culling views behind it.
    /// @return A rectangle that is fully covered by the dialog.
    virtual Rectangle GetFullyCoveredArea() const = 0;

    /// @brief Tells the dialog how much screen height, at the bottom edge,
    ///        it must leave uncovered - e.g. so the horizontal list layout's
    ///        bottom app bar stays visible while a dialog is open. Set by
    ///        DialogPresenter right before showing the dialog. No-op by
    ///        default since BottomSheetView is currently the only dialog
    ///        type that extends down to the bottom edge at all.
    virtual void SetBottomInset(int bottomInset) { }
};
