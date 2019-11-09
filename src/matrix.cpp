#include <cassert>
#include "matrix.h"

namespace math {
Matrix<4, 4> create_translation_matrix(float x, float y, float z) {
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

Matrix<4, 4> create_rotation_matrix(float x, float y, float z, float angle) {
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

Matrix<4, 4> create_projection_matrix(uint16_t width, uint16_t height, float near, float far, float fov) {
    const float aspect = static_cast<float>(width) / height;
    assert(aspect != 0.f);
    const float range = near - far;
    assert(range != 0.f);
    const float tan_half_fov = tanf((fov * PI / 180.f) / 2.f);
    assert(tan_half_fov != 0.f);

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
}
