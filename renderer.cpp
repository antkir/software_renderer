#include <memory>
#include "renderer.h"

namespace renderer {

Renderer::Renderer(Color clear_color, uint16_t width, uint16_t height) {
    this->clear_color = clear_color;
    this->width = width;
    this->height = height;
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

}