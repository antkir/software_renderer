#pragma once

#include "matrix.h"

namespace renderer {
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
}
