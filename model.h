#pragma once

#include <string>
#include <vector>
#include <memory>
#include <functional>

class SDL_Surface;

namespace renderer {
class Vertex;

class Model {
public:
    explicit Model(const std::string &path);
    [[nodiscard]] const std::vector<Vertex>& get_vertex_buffer() const;
    [[nodiscard]] const SDL_Surface* get_texture() const;
private:
    std::vector<Vertex> vertex_buffer;
    std::unique_ptr<SDL_Surface, std::function<void (SDL_Surface*)>> texture;
};
}
