// Copyright 2019 Sic Studios. All rights reserved.
// Use of this source code is governed by our license that can be
// found in the LICENSE file.

#define TINYOBJLOADER_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION

#include <demo/model.hpp>
#include <vk_wrappers/image_utils.hpp>
#include "tiny_obj_loader.h"
#include "stb_image.h"

namespace christalz {

Model::Model(const gfx::LogicalDevicePtr& device,
             const std::string& model_path,
             const std::string& texture_path)
: device_(device) {

    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, model_path.c_str())) {
        throw std::runtime_error(warn + err);
    }

    std::unordered_map<Vertex, uint32_t> uniqueVertices;

    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

    for (const auto& shape : shapes) {
        for (const auto& index : shape.mesh.indices) {
            Vertex vertex{};

            vertex.pos = {attrib.vertices[3 * index.vertex_index + 0],
                          attrib.vertices[3 * index.vertex_index + 1],
                          attrib.vertices[3 * index.vertex_index + 2], 1.0};

            vertex.uvs = {attrib.texcoords[2 * index.texcoord_index + 0],
                          1.0f - attrib.texcoords[2 * index.texcoord_index + 1]};

            vertex.col = {1.0f, 1.0f, 1.0f, 1.0f};

            if (uniqueVertices.count(vertex) == 0) {
                uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
                vertices.push_back(vertex);
            }

            indices.push_back(uniqueVertices[vertex]);
        }
    }

    num_indices_ = indices.size();

    vertices_ = gfx::ComputeBuffer::createFromVector(
        device, vertices, vk::BufferUsageFlagBits::eVertexBuffer);
    CXL_DCHECK(vertices_);

    indices_ = gfx::ComputeBuffer::createFromVector(device, indices,
                                                             vk::BufferUsageFlagBits::eIndexBuffer);
    CXL_DCHECK(indices_);


    int texWidth, texHeight, texChannels;
    stbi_uc* pixels =
        stbi_load(texture_path.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    CXL_DCHECK(pixels);
    texture_ = gfx::ImageUtils::create8BitUnormImage(device, texWidth, texHeight, 4,
                                                         vk::SampleCountFlagBits::e1, pixels);
    CXL_DCHECK(texture_);
}


Model::~Model() {
  texture_.reset();
  vertices_.reset();
  indices_.reset();
}

} // christalz