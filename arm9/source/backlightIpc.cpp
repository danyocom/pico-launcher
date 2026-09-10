#include "common.h"
#include <libtwl/ipc/ipcFifoSystem.h>
#include "ipcChannels.h"
#include "backlightIpc.h"

/// @brief -1 until the ARM7 answers a query, and left at -1 forever on hardware
///        that can't report its level.
static volatile int sReportedLevel = -1;
/// @brief Set once this app has driven the level itself, after which the ARM7's
///        answer describes how things were BEFORE that and must not overwrite
///        it. The query is sent early in boot and answered within a frame or
///        two, so it normally lands long before anything sets a level - this
///        just makes the ordering not matter.
static volatile bool sLevelSetByApp = false;

static void ipcMessageHandler(u32 channel, u32 data, void* arg)
{
    if (sLevelSetByApp)
    {
        return;
    }
    if ((data & 0xF00) == IPC_PMIC_RESP_LEVEL)
    {
        sReportedLevel = (int)(data & 3);
    }
    else if ((data & 0xF00) == IPC_PMIC_RESP_UNKNOWN)
    {
        sReportedLevel = -1;
    }
}

void backlight_init(void)
{
    ipc_setChannelHandler(IPC_CHANNEL_PMIC, ipcMessageHandler, nullptr);
}

void backlight_requestLevel(void)
{
    ipc_sendFifoMessage(IPC_CHANNEL_PMIC, IPC_PMIC_MSG_QUERY_LEVEL);
}

int backlight_getReportedLevel(void)
{
    return sReportedLevel;
}

void backlight_setLevel(unsigned int level)
{
    ipc_sendFifoMessage(IPC_CHANNEL_PMIC, level & 3);
    // keep the cached value in step, so a later read doesn't report a stale
    // level from before the user changed it
    sReportedLevel = (int)(level & 3);
    sLevelSetByApp = true;
}
