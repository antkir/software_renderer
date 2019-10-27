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

class Renderer {
public:
    Renderer(Color clear_color, uint16_t width, uint16_t height);
    void clear_buffer(uint32_t* buffer);
    void set_clear_color(Color color);
private:
    Color clear_color;
    uint16_t width;
    uint16_t height;
};
}
