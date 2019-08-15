#include <hydrogen.h>

#include "os/os.h"
#include "hardware/uart.h"
#include "util/i2a.h"

#include <algorithm>
#include <cstdint>
#include <cstdlib>

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
    auto *section_32 = reinterpret_cast<uint32_t *>(section);
    auto *section_32_end = reinterpret_cast<uint32_t *>(section + len);
    for (auto *p = section_32; p < section_32_end; p++)
        *p = xorshift(state);
}

#if TRILLIUM==1
UART uart{UART::U3, 115200, GPIO::D, 8, 9};
#elif TRILLIUM==2
UART uart{UART::U5, 115200, GPIO::B, 13, 12};
#elif TRILLIUM==3
UART uart{UART::U5, 115200, GPIO::B, 13, 12};
#endif

#if TRILLIUM==1
UART uart_b{UART::U7, 115200, GPIO::E, 8, 7};
UART uart_c{UART::U6, 115200, GPIO::G, 14, 9};
#elif TRILLIUM==2
UART uart_out{UART::U7, 115200, GPIO::E, 8, 7};
#elif TRILLIUM==3
UART uart_out{UART::U6, 115200, GPIO::G, 14, 9};
#endif

void check_ram_constant(const uint8_t *section, uint32_t len, uint8_t fill) {
    for (auto *p = section; p < section + len; p++) {
        if (*p != fill) {
            uart.transmit("[").block();
            uart.transmit(to_hex(reinterpret_cast<uint32_t>(p))).block();
            uart.transmit("] error! Expected ").block();
            uart.transmit(to_hex(fill)).block();
            uart.transmit(", got ").block();
            uart.transmit(to_hex(*p)).block();
            uart.transmit("\r\n").block();
        }
    }
}

void check_ram_prng(const uint8_t *section, uint32_t len) {
    uint32_t state = seed;
    auto *section_32 = reinterpret_cast<const uint32_t *>(section);
    auto *section_32_end = reinterpret_cast<const uint32_t *>(section + len);

    for (auto *p = section_32; p < section_32_end; p++) {
        uint32_t expected = xorshift(state);
        if (*p != expected) {
            uart.transmit("[").block();
            uart.transmit(to_hex(reinterpret_cast<uint32_t>(p))).block();
            uart.transmit("] error! Expected ").block();
            uart.transmit(to_hex(expected)).block();
            uart.transmit(", got ").block();
            uart.transmit(to_hex(*p)).block();
            uart.transmit("\r\n").block();
        }
    }
}

void fill_section(uint8_t *section, uint32_t len, const char *name) {
    uart.transmit("[rad target] filling ").block();
    uart.transmit(name).block();
    uart.transmit(" target...\r\n").block();
    fill_ram_constant(section, len, 0);
    fill_ram_constant(section + len, len, 0xff);
    fill_ram_prng(section + len*2, len);
    uart.transmit("[rad target] done\r\n").block();
}

void check_section(uint8_t *section, uint32_t len, const char *name) {
    check_ram_constant(section, len, 0);
    check_ram_constant(section + len, len, 0xff);
    check_ram_prng(section + len*2, len);
}

#if TRILLIUM==1
GPIO led{GPIO::B, 7, GPIO::OutputPP, GPIO::None, 0};
#elif TRILLIUM==2
GPIO led{GPIO::B, 0, GPIO::OutputPP, GPIO::None, 0};
#elif TRILLIUM==3
GPIO led{GPIO::B, 14, GPIO::OutputPP, GPIO::None, 0};
#endif

const char * hydro_context = "CONTEXT";
constexpr int rounds = 10000;

void hash_main() {
    static uint8_t hash[hydro_hash_BYTES];
    static uint8_t buf[8];

#if TRILLIUM==2 || TRILLIUM==3
    uart_out.init();
#elif TRILLIUM==1
    uart_b.init();
    uart_c.init();
#endif

#if TRILLIUM==1
    uart.transmit("[rad compute] start\r\n").block();
#endif

    while (1) {
        uart.receive(buf, sizeof buf).block();
        hydro_hash_hash(hash, sizeof hash, buf, sizeof buf, hydro_context, nullptr);

        for (int i = 0; i < rounds; i++) {
            hydro_hash_hash(hash, sizeof hash, hash, sizeof hash, hydro_context, nullptr);
        }

        uint8_t ready = 1;
#if TRILLIUM==2 || TRILLIUM==3
        uart_out.receive(&ready, 1).block();
        uart_out.transmit(hash, sizeof hash).block();
#elif TRILLIUM==1
        static uint8_t hash_b[hydro_hash_BYTES];
        static uint8_t hash_c[hydro_hash_BYTES];

        uart.transmit("[rad compute] ready b\r\n").block();
        uart_b.transmit(&ready, 1).block();
        uart_b.receive(hash_b, sizeof hash_b).block();
        uart.transmit("[rad compute] ready c\r\n").block();
        uart_c.transmit(&ready, 1).block();
        uart_c.receive(hash_c, sizeof hash_c).block();
        uart.transmit("[rad compute] received\r\n").block();
        uart.transmit(hash, sizeof hash).block();
        uart.transmit(hash_b, sizeof hash_b).block();
        uart.transmit(hash_c, sizeof hash_c).block();
#endif
    }

#if TRILLIUM==1
    uart.transmit("[rad compute] stop\r\n").block();
#endif
}

void targ_main() {
    uart.transmit("[rad target] start\r\n").block();

    fill_section(dtcm_target[0], sizeof dtcm_target[0], "DTCM");
    fill_section(d1_target[0], sizeof d1_target[0], "D1");
    fill_section(d2_target[0], sizeof d2_target[0], "D2");
    fill_section(d3_target[0], sizeof d3_target[0], "D3");

    while (1) {
        check_section(dtcm_target[0], sizeof dtcm_target[0], "DTCM");
        check_section(d1_target[0], sizeof d1_target[0], "D1");
        check_section(d2_target[0], sizeof d2_target[0], "D2");
        check_section(d3_target[0], sizeof d3_target[0], "D3");
    }

    uart.transmit("[rad target] stop\r\n").block();
}

void rad_target_main() {
    uart.init();

    static uint8_t buf[4];
    uart.receive(buf, 4).block();

    if (std::equal(std::begin(buf), std::end(buf), "targ")) {
        led.init();
        led.write(true);

#if TRILLIUM == 1
        targ_main();
#endif

        while (1);
    } else if (std::equal(std::begin(buf), std::end(buf), "comp")) {
        led.init();
        led.write(true);

        hash_main();
    }

    led.write(false);
    uart.deinit();
}
