#pragma once

/// @brief Sets the DS Lite backlight level (0 = low .. 3 = max) through the
///        ARM7. Fire-and-forget: the ARM7 applies it on its main thread (the
///        PMIC shares the SPI bus with the touch screen) and only after
///        detecting a DS Lite — on the original DS the backlight register
///        mirrors the control register and must not be written. The level
///        persists into the launched game until the console powers off.
void backlight_setLevel(unsigned int level);

/// @brief Registers the handler that receives the ARM7's answer to
///        backlight_requestLevel. Call once, after the fifo system is up.
void backlight_init(void);

/// @brief Asks the ARM7 what level the backlight is currently at. The answer
///        arrives asynchronously - the ARM7 can only touch the power chip from
///        its main thread, since that chip shares the SPI bus with the touch
///        screen - so read it back later with backlight_getReportedLevel.
void backlight_requestLevel(void);

/// @brief The level the ARM7 last reported, or -1 if no answer has arrived yet
///        or the hardware can't report one (an original DS rather than a Lite).
int backlight_getReportedLevel(void);
