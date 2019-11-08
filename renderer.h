#pragma once

#include <cstdint>
#include <vector>
#include "matrix.h"

class SDL_Window;

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

        math::Vector<4> xyzw;
    };

    float u;
    float v;
};

struct Pixel {
    int64_t x;
    int64_t y;
};

struct BarycentricPoint {
    float a;
    float b;
    float c;
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
    Renderer(const SDL_Window* window, uint16_t width, uint16_t height, Color clear_color);
    void clear_buffer(uint32_t* buffer);
    void draw_model(const Model& model, uint32_t* buffer);
    void resize_window(uint16_t width, uint16_t height);
    void set_clear_color(Color color);
private:
    void draw_line(const Vertex& v1, const Vertex& v2, const Vertex& v3, const Pixel &p1, const Pixel &p2, uint32_t* buffer, const SDL_Surface* texture);
    void draw_pixel(const Pixel& p, const Vertex& v1, const Vertex& v2, const Vertex& v3, uint32_t* buffer, const SDL_Surface* texture);
    void draw_triangle(const Vertex& v1, const Vertex& v2, const Vertex& v3, uint32_t* buffer, const SDL_Surface* texture);
    [[nodiscard]] BarycentricPoint get_barycentric_coords(const Pixel& p, const Vertex& v1, const Vertex& v2, const Vertex& v3) const;
    [[nodiscard]] math::Matrix<4, 4> get_transform_matrix() const;
    [[nodiscard]] Vertex transform_vertex(const Vertex& vertex, const math::Matrix<4, 4>& matrix) const;

    Color clear_color = { 0, 0, 0, 255 };
    uint16_t height;
    uint16_t width;
    const SDL_Window* window;
    std::vector<float> zbuffer;
};
}
