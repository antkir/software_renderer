#include <algorithm>
#include "renderer.h"
#include "model.h"

namespace renderer {
Renderer::Renderer(Color clear_color, uint16_t width, uint16_t height) :
clear_color(clear_color), width(width), height(height) {
}

void Renderer::clear_buffer(uint32_t* buffer) {
    for (uint32_t h = 0; h < height; h++) {
        for (uint32_t w = 0; w < width; w++) {
            buffer[width * h + w] = clear_color.bgra;
        }
    }
}

void Renderer::set_clear_color(Color color) {
    clear_color = color;
}

void Renderer::draw_model(const Model& model, uint32_t* buffer) {
    math::Matrix mtx = get_transform_matrix();

    const std::vector<Vertex>& vertex_buffer = model.get_vertex_buffer();
    for (size_t i = 0; i < vertex_buffer.size(); i += 3) {
        Pixel vertex1 = transform_vertex(vertex_buffer[i], mtx);
        Pixel vertex2 = transform_vertex(vertex_buffer[i + 1], mtx);
        Pixel vertex3 = transform_vertex(vertex_buffer[i + 2], mtx);

        draw_line(vertex1, vertex2, buffer);
        draw_line(vertex2, vertex3, buffer);
        draw_line(vertex1, vertex3, buffer);
    }
}

void Renderer::draw_pixel(uint64_t x, uint64_t y, uint32_t* buffer) {
    if (x >= 0 && x < width && y >= 0 && y < height) {
        buffer[width * y + x] = Color{0, 255, 0, 255}.bgra;
    }
}

math::Matrix<4, 4> Renderer::get_transform_matrix() {
    static float ang = 0.f;
    math::Matrix<4, 4> mtx_rot1 = math::create_rotation_matrix(1.f, 0.f, 0.f, 1.6f);
    math::Matrix<4, 4> mtx_rot2 = math::create_rotation_matrix(0.f, 0.f, 1.f, ang);
    math::Matrix<4, 4> mtx_rot = mul(mtx_rot1, mtx_rot2);
    math::Matrix<4, 4> mtx_trans = math::create_translation_matrix(0.f, 15.f, 50.f);
    math::Matrix<4, 4> mtx = mul(mtx_trans, mtx_rot);
    math::Matrix proj = math::create_projection_matrix(width, height, 0.01f, 100.f, 60.f);
    mtx = mul(proj, mtx);
    ang += 0.01f;
    return mtx;
}

Pixel Renderer::transform_vertex(const Vertex& vertex_info, const math::Matrix<4, 4>& matrix) {
    const math::Vector vector = math::mul(matrix, vertex_info.xyzw);
    float xf = (vector.data[0] / vector.data[3] + 1.f) / 2.f;
    float yf = (vector.data[1] / vector.data[3] + 1.f) / 2.f;
    return { std::lround(xf * width), std::lround(yf * height) };
}

void Renderer::draw_line(const Pixel &pixel1, const Pixel &pixel2, uint32_t* buffer) {
    int64_t x1 = pixel1.x;
    int64_t x2 = pixel2.x;
    int64_t y1 = pixel1.y;
    int64_t y2 = pixel2.y;

    int64_t dx = std::abs(x2 - x1);
    int64_t sx = x1 < x2 ? 1 : -1;
    int64_t dy = -std::abs(y2 - y1);
    int64_t sy = y1 < y2 ? 1 : -1;
    int64_t err = dx + dy;

    while (x1 != x2 || y1 != y2) {
        draw_pixel(x1, y1, buffer);

        int64_t err2 = err * 2;
        if (err2 >= dy) {
            err += dy;
            x1 += sx;
        }

        if (err2 <= dx) {
            err += dx;
            y1 += sy;
        }
    }
}
}
