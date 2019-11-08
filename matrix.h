#pragma once

#include <cmath>

namespace math {
constexpr float PI = 3.14159265358979323846;

template <size_t N, size_t M>
struct Matrix {
    float data[N * M];
};

template <size_t N>
using Vector = Matrix<1, N>;

static Matrix<4, 4> create_translation_matrix(float x, float y, float z) {
    Matrix<4, 4> mtx;

    mtx.data[0] = 1.f;
    mtx.data[1] = 0.f;
    mtx.data[2] = 0.f;
    mtx.data[3] = x;

    mtx.data[4] = 0.f;
    mtx.data[5] = 1.f;
    mtx.data[6] = 0.f;
    mtx.data[7] = y;

    mtx.data[8] = 0.f;
    mtx.data[9] = 0.f;
    mtx.data[10] = 1.f;
    mtx.data[11] = z;

    mtx.data[12] = 0.f;
    mtx.data[13] = 0.f;
    mtx.data[14] = 0.f;
    mtx.data[15] = 1.f;

    return mtx;
}

static Matrix<4, 4> create_rotation_matrix(float x, float y, float z, float angle) {
    const float cos_angle = cosf(angle);
    const float sin_angle = sinf(angle);

    Matrix<4, 4> mtx;

    mtx.data[0] = cos_angle + x * x * (1.f - cos_angle);
    mtx.data[1] = x * y * (1.f - cos_angle) - z * sin_angle;
    mtx.data[2] = x * z * (1.f - cos_angle) + y * sin_angle;
    mtx.data[3] = 0.f;

    mtx.data[4] = y * x * (1.f - cos_angle) + z * sin_angle;
    mtx.data[5] = cos_angle + y * y * (1.f - cos_angle);
    mtx.data[6] = y * z * (1.f - cos_angle) - x * sin_angle;
    mtx.data[7] = 0.f;

    mtx.data[8] = z * x * (1.f - cos_angle) - y * sin_angle;
    mtx.data[9] = z * y * (1.f - cos_angle) + x * sin_angle;
    mtx.data[10] = cos_angle + z * z * (1.f - cos_angle);
    mtx.data[11] = 0.f;

    mtx.data[12] = 0.f;
    mtx.data[13] = 0.f;
    mtx.data[14] = 0.f;
    mtx.data[15] = 1.f;

    return mtx;
}

static Matrix<4, 4> create_projection_matrix(uint16_t width, uint16_t height, float near, float far, float fov) {
    const float aspect = static_cast<float>(width) / static_cast<float>(height);
    const float range = near - far;
    const float tan_half_fov = tanf((fov * PI / 180.f) / 2.f);

    Matrix<4, 4> mtx;

    mtx.data[0] = 1.f / (tan_half_fov * aspect);
    mtx.data[1] = 0.f;
    mtx.data[2] = 0.f;
    mtx.data[3] = 0.f;

    mtx.data[4] = 0.f;
    mtx.data[5] = 1.f / tan_half_fov;
    mtx.data[6] = 0.f;
    mtx.data[7] = 0.f;

    mtx.data[8] = 0.f;
    mtx.data[9] = 0.f;
    mtx.data[10] = (-near - far) / range;
    mtx.data[11] = 2.f * far * near / range;

    mtx.data[12] = 0.f;
    mtx.data[13] = 0.f;
    mtx.data[14] = 1.f;
    mtx.data[15] = 0.f;

    return mtx;
}

template <size_t N1, size_t M1, size_t N2, size_t M2>
static Matrix<N2, M1> mul(const Matrix<N1, M1>& l, const Matrix<N2, M2>& r) {
    static_assert(M2 == N1);
    Matrix<N2, M1> mtx { 0 };

    for (size_t i = 0; i < M1; i++) {
        for (size_t j = 0; j < N2; j++) {
            for (size_t k = 0; k < N1; k++) {
                mtx.data[i * N2 + j] += l.data[i * N1 + k] * r.data[j + k * N2];
            }
        }
    }

    return mtx;
}
}