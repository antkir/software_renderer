#pragma once

#include <cstdint>

namespace renderer {
struct Color {
    union {
        struct {
            uint8_t b;
            uint8_t g;
            uint8_t r;
            uint8_t a;
        };

        uint32_t bgra;
    };
};
}
