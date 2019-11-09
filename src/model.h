#pragma once

#include "vertex.h"

#include <functional>
#include <memory>
#include <string>
#include <vector>
#include <SDL_image.h>

namespace renderer {
class Model {
public:
    explicit Model(const std::string& path);
    const std::vector<Vertex>& get_vertex_buffer() const;
    const SDL_Surface* get_texture() const;
private:
    std::unique_ptr<SDL_Surface, void (*) (SDL_Surface*)> init_texture(const std::string& path) const;

    std::vector<Vertex> vertex_buffer;
    std::unique_ptr<SDL_Surface, void (*) (SDL_Surface*)> texture;
};
}
