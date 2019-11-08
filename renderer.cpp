#include <algorithm>
#include <numeric>
#include <chrono>
#include <SDL_video.h>
#include "renderer.h"
#include "model.h"

namespace renderer {
Renderer::Renderer(const SDL_Window* window, uint16_t width, uint16_t height, Color clear_color) :
window(window), width(width), height(height), clear_color(clear_color) {
    zbuffer.resize(width * height, std::numeric_limits<float>::max());
}

void Renderer::clear_buffer(uint32_t* buffer) {
    std::fill(buffer, buffer + width * height, clear_color.bgra);
    std::fill(zbuffer.begin(), zbuffer.end(), std::numeric_limits<float>::max());
}

void Renderer::set_clear_color(Color color) {
    clear_color = color;
}

BarycentricPoint Renderer::get_barycentric_coords(const Pixel& p, const Vertex& v1, const Vertex& v2, const Vertex& v3) const {
    BarycentricPoint barycentric_point;
    float denom = (v2.x * width - v1.x * width) * (v3.y * height - v1.y * height) - (v3.x * width - v1.x * width) * (v2.y * height - v1.y * height);
    barycentric_point.a = ((v2.x * width - p.x) * (v3.y * height - p.y) - (v3.x * width - p.x) * (v2.y * height - p.y)) / denom;
    barycentric_point.b = ((v3.x * width - p.x) * (v1.y * height - p.y) - (v1.x * width - p.x) * (v3.y * height - p.y)) / denom;
    barycentric_point.c = 1.f - barycentric_point.a - barycentric_point.b;
    return barycentric_point;
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

void Renderer::draw_pixel(const Pixel& p, const Vertex& v1, const Vertex& v2, const Vertex& v3, uint32_t* buffer, const SDL_Surface* texture) {
    if (p.x >= 0 && p.x < width && p.y >= 0 && p.y < height) {
        size_t idx = width * p.y + p.x;

        BarycentricPoint barycentric_point = get_barycentric_coords(p, v1, v2, v3);
        const float z = v1.z * barycentric_point.a + v2.z * barycentric_point.b + v3.z * barycentric_point.c;

        if (z < zbuffer[idx]) {
            const float uf = v1.u * barycentric_point.a + v2.u * barycentric_point.b + v3.u * barycentric_point.c;
            const float vf = v1.v * barycentric_point.a + v2.v * barycentric_point.b + v3.v * barycentric_point.c;
            int64_t u = uf * texture->w;
            int64_t v = vf * texture->h;
            if (u < 0 || u >= texture->w || v < 0 || v >= texture->h) {
                return;
            }

            auto pixels = static_cast<uint8_t*>(texture->pixels);
            Color color;
            size_t pixel_idx = (v * texture->w + u) * 3;
            color.r = pixels[pixel_idx];
            color.g = pixels[pixel_idx + 1];
            color.b = pixels[pixel_idx + 2];
            color.a = 255;
            buffer[idx] = color.bgra;
            zbuffer[idx] = z;
        }
    }
}

math::Matrix<4, 4> Renderer::get_transform_matrix() const {
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

void Renderer::draw_line(const Vertex& v1, const Vertex& v2, const Vertex& v3, const Pixel &p1, const Pixel &p2, uint32_t* buffer, const SDL_Surface* texture) {
    int64_t x1 = p1.x;
    int64_t y1 = p1.y;
    int64_t x2 = p2.x;
    int64_t y2 = p2.y;

    int64_t dx = std::abs(x2 - x1);
    int64_t sx = x1 < x2 ? 1 : -1;
    int64_t dy = -std::abs(y2 - y1);
    int64_t sy = y1 < y2 ? 1 : -1;
    int64_t err = dx + dy;

    draw_pixel(Pixel {x2, y2}, v1, v2, v3, buffer, texture);
    while (x1 != x2 || y1 != y2) {
        draw_pixel(Pixel {x1, y1}, v1, v2, v3, buffer, texture);

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

Vertex Renderer::transform_vertex(const Vertex &vertex, const math::Matrix<4, 4>& matrix) const {
    const math::Vector<4> vector = math::mul(matrix, vertex.xyzw);
    Vertex vertex_out = vertex;
    vertex_out.x = (vector.data[0] / vector.data[3] + 1.f) / 2.f;
    vertex_out.y = (vector.data[1] / vector.data[3] + 1.f) / 2.f;
    vertex_out.z = vector.data[2] / vector.data[3];
    return vertex_out;
}

void Renderer::draw_triangle(const Vertex &v1, const Vertex &v2, const Vertex &v3, uint32_t* buffer, const SDL_Surface* texture) {
    int64_t x1 = v1.x * width;
    int64_t y1 = v1.y * height;
    int64_t x2 = v2.x * width;
    int64_t y2 = v2.y * height;
    int64_t x3 = v3.x * width;
    int64_t y3 = v3.y * height;

    int64_t x1_2 = y1 == y2 ? x2 : x1;
    int64_t y1_2 = y1 == y2 ? y2 : y1;

    int64_t dx = std::abs(x3 - x1);
    int64_t sx = x1 < x3 ? 1 : -1;
    int64_t dy = -std::abs(y3 - y1);
    int64_t sy = y1 < y3 ? 1 : -1;
    int64_t err = dx + dy;

    int64_t dx2 = std::abs(x2 - x1_2);
    int64_t sx2 = x1_2 < x2 ? 1 : -1;
    int64_t dy2 = -std::abs(y2 - y1_2);
    int64_t sy2 = y1_2 < y2 ? 1 : -1;
    int64_t err_2 = dx2 + dy2;

    draw_pixel(Pixel {x3, y3}, v1, v2, v3, buffer, texture);
    while (x1 != x3 || y1 != y3) {
        draw_line(v1, v2, v3, Pixel { x1, y1}, Pixel { x1_2, y1_2}, buffer, texture);

        int64_t err2 = err * 2;
        if (err2 >= dy) {
            err += dy;
            x1 += sx;
        }

        if (err2 <= dx) {
            err += dx;
            y1 += sy;

            if (y1_2 == y2) {
                draw_line(v1, v2, v3, Pixel{x1_2, y1_2}, Pixel{x2, y2}, buffer, texture);

                dx2 = std::abs(x3 - x2);
                sx2 = x2 < x3 ? 1 : -1;
                dy2 = -std::abs(y3 - y2);
                sy2 = y2 < y3 ? 1 : -1;
                err_2 = dx2 + dy2;

                x1_2 = x2;
                y1_2 = y2;

                x2 = x3;
                y2 = y3;
            }

            int64_t y1_2_tmp = y1_2;
            while (y1_2_tmp == y1_2) {
                int64_t err2_2 = err_2 * 2;
                if (err2_2 >= dy2) {
                    err_2 += dy2;
                    x1_2 += sx2;
                }

                if (err2_2 <= dx2) {
                    err_2 += dx2;
                    y1_2 += sy2;
                }
            }
        }
    }
}

void Renderer::resize_window(uint16_t width, uint16_t height) {
    this->width = width;
    this->height = height;
    zbuffer.resize(width * height, std::numeric_limits<float>::max());
}

}
