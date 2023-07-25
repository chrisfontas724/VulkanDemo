// Copyright 2020 Sic Studios. All rights reserved.
// Use of this source code is governed by our license that can be
// found in the LICENSE file.

#include <compute_buffer.hpp>
#include <functional>
#include <glm/gtx/hash.hpp>
#include <glm/vec2.hpp>


struct Vertex {
    glm::vec4 pos;
    glm::vec4 col;
    glm::vec2 uvs;
    bool operator==(const Vertex& other) const {
        return pos == other.pos && col == other.col && uvs == other.uvs;
    }
};

namespace std {
template <>
struct hash<Vertex> {
    size_t operator()(Vertex const& vertex) const {
        return ((hash<glm::vec3>()(vertex.pos) ^ (hash<glm::vec3>()(vertex.col) << 1)) >> 1) ^
               (hash<glm::vec2>()(vertex.uvs) << 1);
    }
};
}  // namespace std

namespace christalz {

class Model {
public:

Model(const gfx::LogicalDevicePtr& device,
      const std::string& model_path,
      const std::string& texture_path);
~Model();

const gfx::ComputeTexturePtr& texture() const { return texture_; }
const gfx::ComputeBufferPtr& vertices() const { return vertices_; }
const gfx::ComputeBufferPtr& indices() const { return indices_; }
uint32_t num_indices() const { return num_indices_; }

private:
 gfx::LogicalDeviceWeakPtr device_;
 gfx::ComputeTexturePtr texture_;
 gfx::ComputeBufferPtr vertices_;
 gfx::ComputeBufferPtr indices_;
 uint32_t num_indices_;

};

} // christalz