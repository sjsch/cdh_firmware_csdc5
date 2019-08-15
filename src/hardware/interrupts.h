#pragma once

#define INTERRUPT                                                              \
    extern "C" __attribute__((used, interrupt, externally_visible))

INTERRUPT void handle_systick();
INTERRUPT void handle_hardfault();
INTERRUPT void handle_usart1_irq();
INTERRUPT void handle_usart2_irq();
INTERRUPT void handle_usart3_irq();
INTERRUPT void handle_uart4_irq();
INTERRUPT void handle_uart5_irq();
INTERRUPT void handle_usart6_irq();
INTERRUPT void handle_uart7_irq();
INTERRUPT void handle_uart8_irq();
