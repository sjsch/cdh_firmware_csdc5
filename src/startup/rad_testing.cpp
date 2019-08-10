#include "hardware/uart.h"

#include <cstdint>
#include <cstdlib>

// #include <

/*
 * Memory layout we're interested in testing:
 *
 * - Tightly-Coupled Memory (TCM)
 *   - DTCM - can contain data, but not executable code (128K)
 * - Regular SRAM banks (x3)
 *   - Bank 1 (512K)
 *   - Bank 2 (288K)
 *   - Bank 3 (64K)
 * - Flash memory (2048K)
 */

/*
 * All memory targets are tested in three modes: all ones, all zeroes,
 * and psuedorandom.
 */

constexpr unsigned long long operator"" _k(unsigned long long n) {
    return n * 1024;
}

uint8_t __attribute__((section(".dtcm_target"))) dtcm_target[3][32_k];
uint8_t __attribute__((section(".d1_target"))) d1_target[3][128_k];
uint8_t __attribute__((section(".d2_target"))) d2_target[3][96_k];
uint8_t __attribute__((section(".d3_target"))) d3_target[3][16_k];
// uint8_t __attribute__((section(".flash_target"))) flash_target[3][512_k];

/**
 * @brief Fill the specified section with the constant byte `fill`.
 */
void fill_ram_constant(uint8_t *section, uint32_t len, uint8_t fill) {
    for (uint8_t *p = section; p < section + len; p++)
        *p = fill;
}

/**
 * @brief Decent, but extremely fast PRNG. (xorshift32)
 */
uint32_t xorshift(uint32_t &state) {
    state ^= state << 13;
    state ^= state << 17;
    state ^= state << 5;
    return state;
}

/**
 * @brief Constant seed for reproducibility, chosen at random.
 */
constexpr uint32_t seed = 0x40405aa6;

void fill_ram_prng(uint8_t *section, uint32_t len) {
    uint32_t state = seed;
    uint32_t *section_32 = reinterpret_cast<uint32_t *>(section);
    uint32_t *section_32_end = reinterpret_cast<uint32_t *>(section + len);
    for (uint32_t *p = section_32; p < section_32_end; p++)
        *p = xorshift(state);
}

UART uart{UART::U3, 115200, GPIO_D, 8, 9};

void check_ram_constant(uint8_t *section, uint32_t len, uint8_t fill) {
    for (uint8_t *p = section; p < section + len; p++) {
        if (*p != fill) {
        }
    }
}

template <size_t N>
void fill_section(uint8_t (&section)[3][N], const char *name) {
    uart.transmit("[rad target] filling ").block();
    uart.transmit(name).block();
    uart.transmit(" target...\r\n").block();
    fill_ram_constant(section[0], sizeof section[0], 0);
    fill_ram_constant(section[1], sizeof section[1], 1);
    fill_ram_prng(section[2], sizeof section[2]);
    uart.transmit("[rad target] done\r\n").block();
}

void rad_target_main() {
    uart.init();
    uart.transmit("[rad target] start\r\n").block();

    fill_section(dtcm_target, "DTCM");
    fill_section(d1_target, "D1");
    fill_section(d2_target, "D2");
    fill_section(d3_target, "D3");

    uart.transmit("[rad target] stop\r\n").block();
    uart.deinit();
}
