#pragma once

#include <cmath>
#include <cstdint>

namespace math {
constexpr float PI = 3.14159265358979323846;

template <size_t N, size_t M>
struct Matrix {
    float data[N * M];
};

template <size_t N>
using Vector = Matrix<1, N>;

Matrix<4, 4> create_projection_matrix(uint16_t width, uint16_t height, float near, float far, float fov);
Matrix<4, 4> create_rotation_matrix(float x, float y, float z, float angle);
Matrix<4, 4> create_translation_matrix(float x, float y, float z);

template <size_t N1, size_t M1, size_t N2, size_t M2>
Matrix<N2, M1> mul(const Matrix<N1, M1>& l, const Matrix<N2, M2>& r) {
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