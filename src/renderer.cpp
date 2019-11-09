#include "matrix.h"
#include "model.h"
#include "renderer.h"
#include "vertex.h"

#include <algorithm>
#include <SDL_video.h>
#include <cassert>

namespace renderer {
Renderer::Renderer(const SDL_Window* window, uint16_t width, uint16_t height, Color clear_color)
        : window(window), width(width), height(height), clear_color(clear_color) {
    zbuffer.resize(width * height, std::numeric_limits<float>::max());
}

void Renderer::clear_buffer(uint32_t* buffer) {
    std::fill(buffer, buffer + width * height, clear_color.bgra);
    std::fill(zbuffer.begin(), zbuffer.end(), std::numeric_limits<float>::max());
}

void Renderer::draw_model(const Model& model, uint32_t* buffer) {
    math::Matrix mtx = get_transform_matrix();
    const std::vector<Vertex>& vertex_buffer = model.get_vertex_buffer();
    for (size_t i = 0; i < vertex_buffer.size(); i += 3) {
        Vertex vertex1 = transform_vertex(vertex_buffer[i], mtx);
        Vertex vertex2 = transform_vertex(vertex_buffer[i + 1], mtx);
        Vertex vertex3 = transform_vertex(vertex_buffer[i + 2], mtx);

        if (vertex3.y < vertex1.y) {
            std::swap(vertex3, vertex1);
        }
        if (vertex2.y < vertex1.y) {
            std::swap(vertex2, vertex1);
        }
        if (vertex3.y < vertex2.y) {
            std::swap(vertex3, vertex2);
        }

        draw_triangle(vertex1, vertex2, vertex3, buffer, model.get_texture());
    }
}

math::Matrix<4, 4> Renderer::get_transform_matrix() const {
    //TODO redo this
    static float ang;
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

Vertex Renderer::transform_vertex(const Vertex &vertex, const math::Matrix<4, 4>& matrix) const {
    const math::Vector<4> vector = math::mul(matrix, vertex.xyzw);
    assert(vector.data[3] != 0.f);
    Vertex vertex_out = vertex;
    vertex_out.x = (vector.data[0] / vector.data[3] + 1.f) / 2.f;
    vertex_out.y = (vector.data[1] / vector.data[3] + 1.f) / 2.f;
    vertex_out.z = vector.data[2] / vector.data[3];
    return vertex_out;
}

void Renderer::draw_triangle(const Vertex &v1, const Vertex &v2, const Vertex &v3, uint32_t* buffer, const SDL_Surface* texture) {
    float barycentic_denom = (v2.x * width - v1.x * width) * (v3.y * height - v1.y * height) - (v3.x * width - v1.x * width) * (v2.y * height - v1.y * height);
    if (barycentic_denom == 0.f) {
        barycentic_denom = std::numeric_limits<float>::max();
    }

    int64_t x1 = v1.x * width;
    int64_t y1 = v1.y * height;
    int64_t x2 = v2.x * width;
    int64_t y2 = v2.y * height;
    int64_t x3 = v3.x * width;
    int64_t y3 = v3.y * height + 1;

    int64_t dx_long = std::abs(x3 - x1);
    int64_t dy_long = -std::abs(y3 - y1);
    int64_t err_long = dx_long + dy_long;
    int64_t sx_long = x1 < x3 ? 1 : -1;

    int64_t x1_short = x1;

    int64_t dx_short = std::abs(x2 - x1_short);
    int64_t dy_short = -std::abs(y2 - y1);
    int64_t err_short = dx_short + dy_short;
    int64_t sx_short = x1_short < x2 ? 1 : -1;

    bool line_drawn = false;

    int64_t y_end = std::min(y3, static_cast<int64_t>(height));
    while (x1 != x3 || y1 != y_end) {
        if (!line_drawn && y1 >= 0) {
            draw_line(v1, v2, v3, barycentic_denom, x1, x1_short, y1, buffer, texture);
            line_drawn = true;
        }

        int64_t err2_long = err_long * 2;
        if (err2_long >= dy_long) {
            err_long += dy_long;
            x1 += sx_long;
        }

        if (err2_long <= dx_long) {
            err_long += dx_long;

            if (y1 == y2) {
                dx_short = std::abs(x3 - x2);
                dy_short = -std::abs(y3 - y2);
                err_short = dx_short + dy_short;
                sx_short = x2 < x3 ? 1 : -1;

                x1_short = x2;

                x2 = x3;
                y2 = y3;
            }

            int64_t y_next = y1 + 1;
            while (y1 < y_next) {
                int64_t err2_short = err_short * 2;
                if (err2_short >= dy_short) {
                    err_short += dy_short;
                    x1_short += sx_short;
                }

                if (err2_short <= dx_short) {
                    err_short += dx_short;
                    y1++;
                }
            }

            line_drawn = false;
        }
    }
}

void Renderer::draw_line(const Vertex& v1, const Vertex& v2, const Vertex& v3, float barycentic_denom, int64_t x1, int64_t x2, int64_t y, uint32_t* buffer, const SDL_Surface* texture) {
    if (x2 < x1) {
        std::swap(x1, x2);
    }
    x1 = std::max(x1, 0l);
    x2 = std::min(x2, static_cast<int64_t>(width - 1));
    while (x1 <= x2) {
        draw_pixel(Pixel {x1, y}, v1, v2, v3, barycentic_denom, buffer, texture);
        x1++;
    }
}

void Renderer::draw_pixel(const Pixel& p, const Vertex& v1, const Vertex& v2, const Vertex& v3, float barycentic_denom, uint32_t* buffer, const SDL_Surface* texture) {
    assert(p.x >= 0 && p.x < width && p.y >= 0 && p.y < height);
    size_t idx = width * p.y + p.x;

    BarycentricPoint barycentric_point = get_barycentric_coords(p, v1, v2, v3, barycentic_denom);
    const float z = v1.z * barycentric_point.a + v2.z * barycentric_point.b + v3.z * barycentric_point.c;

    if (z < zbuffer[idx]) {
        const float uf = v1.u * barycentric_point.a + v2.u * barycentric_point.b + v3.u * barycentric_point.c;
        const float vf = v1.v * barycentric_point.a + v2.v * barycentric_point.b + v3.v * barycentric_point.c;
        auto ub = static_cast<uint64_t>(uf * texture->w);
        auto vb = static_cast<uint64_t>(vf * texture->h);
        uint32_t u = ub >= texture->w ? ub % texture->w : ub;
        uint32_t v = vb >= texture->h ? vb % texture->h : vb;

        Color color;
        auto pixels = static_cast<uint8_t*>(texture->pixels);
        const size_t pixel_idx = (v * texture->w + u) * 3;
        color.r = pixels[pixel_idx];
        color.g = pixels[pixel_idx + 1];
        color.b = pixels[pixel_idx + 2];
//        color.r = 255;
//        color.g = 0;
//        color.b = 0;
        color.a = 255;
        buffer[idx] = color.bgra;
        zbuffer[idx] = z;
    }
}

BarycentricPoint Renderer::get_barycentric_coords(const Pixel& p, const Vertex& v1, const Vertex& v2, const Vertex& v3, float denom) const {
    BarycentricPoint barycentric_point = {};
    barycentric_point.a = ((v2.x * width - p.x) * (v3.y * height - p.y) - (v3.x * width - p.x) * (v2.y * height - p.y)) / denom;
    barycentric_point.b = ((v3.x * width - p.x) * (v1.y * height - p.y) - (v1.x * width - p.x) * (v3.y * height - p.y)) / denom;
    barycentric_point.c = 1.f - barycentric_point.a - barycentric_point.b;
    return barycentric_point;
}

void Renderer::resize_window(uint16_t width, uint16_t height) {
    this->width = width;
    this->height = height;
    zbuffer.resize(width * height, std::numeric_limits<float>::max());
}

void Renderer::set_clear_color(Color color) {
    clear_color = color;
}

}
