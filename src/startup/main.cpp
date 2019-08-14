#include "hardware/clock.h"
#include "hardware/init.h"
#include "hardware/uart.h"
#include "os/os.h"
#include "startup/rad_testing.h"

UART uart{UART::U3, 9600, GPIO::D, 8, 9};
GPIO led{GPIO::B, 7, GPIO::OutputPP, GPIO::None, 0};

void init_func();
StaticTask<128> init_task{"INIT", 0, init_func};

void iwdg_func();
StaticTask<128> iwdg_task{"IWDG", 1, iwdg_func};

/**
 * @brief Main entry point for Trillium's firmware.
 *
 * This is the main entry point of the firmware, where execution starts.  Note
 * that the scheduler isn't running when this function starts, so that all the
 * low-level initializatoin of the clock, hardware, and other things may happen
 * undisturbed.
 *
 * Before this function is called, a few things happen in @file
 * startup/startup.s:
 * - The C runtime is initialized:
 *  - Zero-initialized globals are zeroed out in RAM.
 *  - Initialized globals' starting values are copied out of FLASH.
 * - Global C++ constructors are called (important).
 *
 * @return int This function never returns, because the scheduler starts.
 */
int main() {
    clock_init();
    hardware_init();

    init_task.start();

#if IWDG_ENABLE
    iwdg_task.start();
#endif

    // Does not return.
    os_init();
}

IWDG_HandleTypeDef iwdg;

void iwdg_func() {
    WakeupTimer wakeup{400};

    // need to refresh at least every 500ms
    __HAL_DBGMCU_FREEZE_IWDG1();
    iwdg.Instance = IWDG1;
    iwdg.Init = {IWDG_PRESCALER_64, 0xFF, 0x0fff};
    HAL_IWDG_Init(&iwdg);

    while (1) {
        HAL_IWDG_Refresh(&iwdg);
        wakeup.sleep();
    }
}

/**
 * The task that runs immediately after the processor starts.  Unlike
 * @ref main, it is safe to use asynchronous or blocking calls here.
 */
void init_func() {
    rad_target_main();

    led.init();
    led.write(true);

    init_task.stop();
}
