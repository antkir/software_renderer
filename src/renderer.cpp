#include "matrix.h"
#include "model.h"
#include "renderer.h"
#include "vertex.h"

#include <algorithm>
#include <cassert>
#include <SDL2/SDL_video.h>

namespace renderer {
Renderer::Renderer(const SDL_Window* window, uint16_t width, uint16_t height, Color clear_color)
        : window(window), width(width), height(height), clear_color(clear_color) {
    zbuffer.resize(width * height, std::numeric_limits<float>::max());
}

void Renderer::clear_buffer(uint32_t* buffer) {
    std::fill(buffer, buffer + width * height, clear_color.bgra);
    std::fill(zbuffer.begin(), zbuffer.end(), std::numeric_limits<float>::max());
}

void Renderer::draw_model(const Model* model, uint32_t* buffer, const math::Matrix<4, 4>& rotation_mtx, const math::Matrix<4, 4>& translation_mtx, float fov) {
    const math::Matrix mtx = get_transform_matrix(rotation_mtx, translation_mtx, fov);
    const std::vector<Vertex>& vertex_buffer = model->get_vertex_buffer();
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

        draw_triangle(vertex1, vertex2, vertex3, buffer, model->get_texture());
    }
}

inline math::Matrix<4, 4> Renderer::get_transform_matrix(const math::Matrix<4, 4>& rotation_mtx, const math::Matrix<4, 4>& translation_mtx, float fov) const {
    math::Matrix<4, 4> mtx = math::mul(translation_mtx, rotation_mtx);
    math::Matrix proj = math::create_projection_matrix(width, height, 0.01f, 100.f, fov);
    mtx = math::mul(proj, mtx);
    return mtx;
}

inline Vertex Renderer::transform_vertex(const Vertex &vertex, const math::Matrix<4, 4>& matrix) const {
    const math::Vector<4> vector = math::mul(matrix, vertex.xyzw);
    assert(vector.data[3] != 0.f);
    Vertex vertex_out = vertex;
    vertex_out.x = (vector.data[0] / vector.data[3] + 1.f) / 2.f;
    vertex_out.y = (vector.data[1] / vector.data[3] + 1.f) / 2.f;
    vertex_out.z = vector.data[2] / vector.data[3];
    return vertex_out;
}

void Renderer::draw_triangle(const Vertex &v1, const Vertex &v2, const Vertex &v3, uint32_t* buffer, const SDL_Surface* texture) {
    assert(v1.y <= v2.y);
    assert(v2.y <= v3.y);

    float barycentric_denom = (v2.x * width - v1.x * width) * (v3.y * height - v1.y * height) - (v3.x * width - v1.x * width) * (v2.y * height - v1.y * height);
    if (barycentric_denom == 0.f) {
        barycentric_denom = std::numeric_limits<float>::max();
    }

    float x1 = v1.x * width;
    int64_t y1 = v1.y * height;
    float x2 = v2.x * width;
    int64_t y2 = v2.y * height;
    float x3 = v3.x * width;
    int64_t y3 = v3.y * height;

    if (y1 >= height || y3 < 0) {
        return;
    }

    const uint64_t dy_ab = y2 - y1;
    const uint64_t dy_bc = y3 - y2;
    const uint64_t dy_ac = y3 - y1;

    if (dy_ab > 0) {
        const float dx_ab = (x2 - x1) / dy_ab;
        const float dx_ac = (x3 - x1) / dy_ac;

        int64_t i = 0;
        do {
            int64_t y = y1 + i;
            if (y < 0) {
                continue;
            }
            if (y >= height) {
                break;
            }

            int64_t x_start = (x1 + dx_ab * i);
            int64_t x_end = (x1 + dx_ac * i);
            draw_line(v1, v2, v3, barycentric_denom, x_start, x_end, y, buffer, texture);
        } while (++i < dy_ab);
    }

    const float mx = x1 + dy_ab * (x3 - x1) / dy_ac;
    const float dx_bc = (x3 - x2) / dy_bc;
    const float dx_ec = (x3 - mx) / dy_bc;

    int64_t i = 0;
    do {
        int64_t y = y2 + i;
        if (y < 0) {
            continue;
        }
        if (y >= height) {
            break;
        }

        int64_t x_start = (x2 + dx_bc * i);
        int64_t x_end = (mx + dx_ec * i);
        draw_line(v1, v2, v3, barycentric_denom, x_start, x_end, y, buffer, texture);
    } while (++i <= dy_bc);
}

inline void Renderer::draw_line(const Vertex& v1, const Vertex& v2, const Vertex& v3, float barycentric_denom, int64_t x1, int64_t x2, int64_t y, uint32_t* buffer, const SDL_Surface* texture) {
    if (x2 < x1) {
        std::swap(x1, x2);
    }
    x1 = std::max<int64_t>(x1, 0);
    x2 = std::min(x2, static_cast<int64_t>(width - 1));
    while (x1 <= x2) {
        draw_pixel(Pixel {x1, y}, v1, v2, v3, barycentric_denom, buffer, texture);
        x1++;
    }
}

void Renderer::draw_pixel(const Pixel& p, const Vertex& v1, const Vertex& v2, const Vertex& v3, float barycentric_denom, uint32_t* buffer, const SDL_Surface* texture) {
    assert(p.x >= 0 && p.x < width && p.y >= 0 && p.y < height);
    assert(buffer != nullptr);
    assert(texture != nullptr);
    size_t idx = width * p.y + p.x;

    BarycentricPoint barycentric_point = get_barycentric_coords(p, v1, v2, v3, barycentric_denom);
    const float z = v1.z * barycentric_point.a + v2.z * barycentric_point.b + v3.z * barycentric_point.c;

    if (z < zbuffer[idx]) {
        const float uf = v1.u * barycentric_point.a + v2.u * barycentric_point.b + v3.u * barycentric_point.c;
        const float vf = v1.v * barycentric_point.a + v2.v * barycentric_point.b + v3.v * barycentric_point.c;
        auto ub = static_cast<uint32_t>(uf * texture->w);
        auto vb = static_cast<uint32_t>(vf * texture->h);
        uint32_t u = ub >= texture->w ? ub % texture->w : ub;
        uint32_t v = vb >= texture->h ? vb % texture->h : vb;

        Color color;
        auto pixels = static_cast<uint8_t*>(texture->pixels);
        const size_t pixel_idx = (v * texture->w + u) * texture->format->BytesPerPixel;
        color.b = pixels[pixel_idx];
        color.g = pixels[pixel_idx + 1];
        color.r = pixels[pixel_idx + 2];
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
