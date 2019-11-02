#include <iostream>
#include "model.h"
#include "renderer.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

namespace renderer {

Model::Model(const std::string &path) {
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;

    std::string warn;
    std::string err;

    tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.c_str());

    if (!warn.empty()) {
        std::cout << warn << std::endl;
    }
    if (!err.empty()) {
        std::cerr << err << std::endl;
    }

    for (tinyobj::shape_t shape : shapes) {
        size_t index_offset = 0;

        // num_face_vertices is assumed to be 3 for every face
        vertex_buffer.resize(shape.mesh.num_face_vertices.size() * 3);

        for (size_t face = 0; face < shape.mesh.num_face_vertices.size(); face++) {
            for (size_t vertex = 0; vertex < 3; vertex++) {
                tinyobj::index_t idx = shape.mesh.indices[index_offset + vertex];
                tinyobj::real_t x = attrib.vertices[3 * idx.vertex_index + 0];
                tinyobj::real_t y = attrib.vertices[3 * idx.vertex_index + 1];
                tinyobj::real_t z = attrib.vertices[3 * idx.vertex_index + 2];
                tinyobj::real_t u = attrib.texcoords[2 * idx.texcoord_index + 0];
                tinyobj::real_t v = attrib.texcoords[2 * idx.texcoord_index + 1];

                vertex_buffer[face * 3 + vertex] = {x, y, z, 1.f, u, v};
            }
            index_offset += 3;
        }
    }
}

const std::vector<Vertex>& Model::get_vertex_buffer() const {
    return vertex_buffer;
}
}
