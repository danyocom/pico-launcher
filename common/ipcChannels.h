#pragma once

#define IPC_CHANNEL_DSI_SD  16
#define IPC_CHANNEL_DLDI    17
#define IPC_CHANNEL_LOADER  18
#define IPC_CHANNEL_SOUND   19
#define IPC_CHANNEL_RTC     20
#define IPC_CHANNEL_PMIC    21

// Messages on IPC_CHANNEL_PMIC. ARM9 -> ARM7 is either a level to apply (a
// plain 0..3, as it always was) or a query; ARM7 -> ARM9 is the answer to a
// query. Only the ARM7 can reach the power management chip, so the level the
// console booted with is otherwise unknowable to the UI.
#define IPC_PMIC_MSG_QUERY_LEVEL    0x100
#define IPC_PMIC_RESP_LEVEL         0x200   // | level, when the level was read
#define IPC_PMIC_RESP_UNKNOWN       0x300   // not a DS Lite, nothing to report
