#pragma once

#include "color.h"

#include <cstdint>
#include <vector>

class SDL_Window;

namespace renderer {
class Model;

struct Pixel {
    int64_t x;
    int64_t y;
};

struct BarycentricPoint {
    float a;
    float b;
    float c;
};

class Renderer {
public:
    Renderer(const SDL_Window* window, uint16_t width, uint16_t height, Color clear_color);
    void clear_buffer(uint32_t* buffer);
    void draw_model(const Model* model, uint32_t* buffer, const math::Matrix<4, 4>& rotation_mtx, const math::Matrix<4, 4>& translation_mtx, float fov);
    void resize_window(uint16_t width, uint16_t height);
    void set_clear_color(Color color);
private:
    void draw_line(const Vertex& v1, const Vertex& v2, const Vertex& v3, float barycentric_denom, int64_t x1, int64_t x2, int64_t y, uint32_t* buffer, const SDL_Surface* texture);
    void draw_pixel(const Pixel& p, const Vertex& v1, const Vertex& v2, const Vertex& v3, float barycentric_denom, uint32_t* buffer, const SDL_Surface* texture);
    void draw_triangle(const Vertex& v1, const Vertex& v2, const Vertex& v3, uint32_t* buffer, const SDL_Surface* texture);
    BarycentricPoint get_barycentric_coords(const Pixel& p, const Vertex& v1, const Vertex& v2, const Vertex& v3, float denom) const;
    math::Matrix<4, 4> get_transform_matrix(const math::Matrix<4, 4>& rotation_mtx, const math::Matrix<4, 4>& translation_mtx, float fov) const;
    Vertex transform_vertex(const Vertex& vertex, const math::Matrix<4, 4>& matrix) const;

    Color clear_color;
    uint16_t height;
    uint16_t width;
    const SDL_Window* window;
    std::vector<float> zbuffer;
};
}
