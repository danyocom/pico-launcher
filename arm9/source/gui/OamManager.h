#pragma once
#include <libtwl/gfx/gfxOam.h>
#include <libtwl/dma/dmaNitro.h>
#include <string.h>

/// @brief Class for managing oams and affine matrices.
class alignas(32) OamManager
{
  public:
    OamManager()
    {
        Clear();
    }

    gfx_oam_entry_t* AllocOams(u32 count)
    {
        // Hardware has 128 entries and no more. Without this check the pointer
        // simply walks off the front of the table and callers write over
        // whatever memory precedes it - which is how a busy list ended up
        // making the bottom icon bar flicker: the bar is drawn after the list,
        // so it is the allocation that overflows, and its entries landed
        // outside the table instead of on screen.
        //
        // Overflowing draws are sent to a scratch entry instead. They still
        // don't appear - there is no room for them - but they can no longer
        // corrupt anything, and the failure stays confined to the sprites that
        // didn't fit.
        if (_oamPtr - count < &_oamTable.objs[0])
        {
            return &_overflowOam;
        }
        _oamPtr -= count;
        return _oamPtr;
    }

    /// @brief How many OAM entries are still free this frame. Lets a caller
    ///        that would rather draw nothing than draw half of something check
    ///        before it starts.
    u32 GetFreeOamCount() const
    {
        return (u32)(_oamPtr - &_oamTable.objs[0]);
    }

    gfx_oam_mtx_t* AllocMatrices(u32 count, u32& mtxId)
    {
        mtxId = _mtxIdx;
        gfx_oam_mtx_t* result = &_oamTable.mtxs[_mtxIdx];
        _mtxIdx += count;
        return result;
    }

    void Apply(vu16* dst);

    void Clear();

private:
    gfx_oam_table_t _oamTable alignas(32);
    gfx_oam_entry_t* _oamPtr;
    u32 _mtxIdx;
    /// @brief Somewhere harmless for allocations that don't fit to be written.
    ///        Deliberately not part of _oamTable, so nothing here reaches the
    ///        hardware.
    gfx_oam_entry_t _overflowOam;
};
