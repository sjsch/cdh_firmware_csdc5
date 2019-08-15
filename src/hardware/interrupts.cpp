#include <stm32h7xx_hal.h>

#include "FreeRTOS.h"
#include "hardware/interrupts.h"

extern "C" void xPortSysTickHandler();

/**
 * @brief Handle the Cortex's systick.
 *
 * The frequency of the systick is configured in @ref clock_init.  For
 * now, this is also going to increment the HAL tick, but this is a
 * temporary solution that should be replaced with a proper timer once
 * we get it working.
 *
 */
void handle_systick() {
    xPortSysTickHandler();
    HAL_IncTick();
}

void handle_hardfault() {
    while (1) {
        __asm__("nop;");
    }
}
