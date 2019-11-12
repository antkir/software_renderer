#include "model.h"
#include "vertex.h"

#include <iostream>
#include <tiny_obj_loader_impl.h>

namespace renderer {

Model::Model(const std::string &path) : texture(init_texture(path)) {
    if (texture == nullptr) {
        throw std::runtime_error("Failed to load a texture: " + std::string(SDL_GetError()));
    }

    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;

    std::string warn;
    std::string err;

    std::string file_obj = path + ".obj";
    bool has_loaded = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, file_obj.c_str());
    if (!warn.empty()) {
        std::cout << warn << std::endl;
    }
    if (!has_loaded) {
        throw std::runtime_error("Failed to load a model: " + err);
    }

    for (const tinyobj::shape_t& shape : shapes) {
        size_t index_offset = 0;

        vertex_buffer.resize(shape.mesh.num_face_vertices.size() * 3);

        for (size_t face = 0; face < shape.mesh.num_face_vertices.size(); face++) {
            // num_face_vertices must be 3 for every face
            assert(shape.mesh.num_face_vertices[face] == 3);

            for (size_t vertex = 0; vertex < 3; vertex++) {
                tinyobj::index_t idx = shape.mesh.indices[index_offset + vertex];
                tinyobj::real_t x = attrib.vertices[3 * idx.vertex_index + 0];
                tinyobj::real_t y = attrib.vertices[3 * idx.vertex_index + 1];
                tinyobj::real_t z = attrib.vertices[3 * idx.vertex_index + 2];
                tinyobj::real_t u = attrib.texcoords[2 * idx.texcoord_index + 0];
                tinyobj::real_t v = 1.f - attrib.texcoords[2 * idx.texcoord_index + 1];

                vertex_buffer[face * 3 + vertex] = renderer::Vertex {x, y, z, 1.f, u, v};
            }
            index_offset += 3;
        }
    }
}

std::unique_ptr<SDL_Surface, void (*) (SDL_Surface*)> Model::init_texture(const std::string &path) const {
    std::string file_texture = path + ".bmp";
    auto deleter = [](SDL_Surface* surface) { SDL_FreeSurface(surface); };
    return std::unique_ptr<SDL_Surface, void (*) (SDL_Surface*)>(SDL_LoadBMP(file_texture.c_str()), deleter);
}

const std::vector<Vertex>& Model::get_vertex_buffer() const {
    return vertex_buffer;
}

const SDL_Surface* Model::get_texture() const {
    return texture.get();
}
}
