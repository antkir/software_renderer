#pragma once

#include <string>
#include <vector>

namespace renderer {
class Vertex;

class Model {
public:
    explicit Model(const std::string &path);
    [[nodiscard]] const std::vector<Vertex>& get_vertex_buffer() const;
private:
    std::vector<Vertex> vertex_buffer;
};
}
