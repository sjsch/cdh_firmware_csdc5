#pragma once

#include <limits>
#include <array>

char *i2a(int i);

template <typename T>
class hex_digits {
public:
    static constexpr unsigned n = std::numeric_limits<T>::digits / 4;
    using chars = std::array<uint8_t, n>;
};

template <typename T>
auto to_hex(T n) -> typename hex_digits<T>::chars {
    static_assert(std::numeric_limits<T>::radix == 2, "radix != 2");
    typename hex_digits<T>::chars cs;
    auto ds = hex_digits<T>::n;
    for (unsigned i = 0; i < ds; i++) {
        uint8_t nibble = (n >> ((ds - i - 1) * 4)) & 0xf;
        if (nibble < 0xa)
            cs[i] = nibble + '0';
        else
            cs[i] = nibble - 0xa + 'a';
    }
    return cs;
}
