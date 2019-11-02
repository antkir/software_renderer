#pragma once

#include <cstdint>
#include <vector>
#include "matrix.h"

namespace renderer {
class Model;

struct Vertex {
    union {
        struct {
            float x;
            float y;
            float z;
            float w;
        };

        math::Vector xyzw;
    };

    float u;
    float v;
};

struct Pixel {
    int64_t x;
    int64_t y;
};

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
    void draw_model(const Model& model, uint32_t* buffer);
    void set_clear_color(Color color);
    Pixel transform_vertex(const Vertex& vertex_info, const math::Matrix<4, 4>& matrix);
private:
    void draw_line(const Pixel& pixel1, const Pixel& pixel2, uint32_t* buffer);
    void draw_pixel(uint64_t x, uint64_t y, uint32_t* buffer);
    math::Matrix<4, 4> get_transform_matrix();

    Color clear_color = { 0, 0, 0, 255 };
    uint16_t width;
    uint16_t height;
};
}
