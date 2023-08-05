// Copyright 2023 Sic Studios. All rights reserved.
// Use of this source code is governed by our license that can be
// found in the LICENSE file.


#ifndef NAIVE_PATH_TRACER_HPP_
#define NAIVE_PATH_TRACER_HPP_

#include <string>
#include "demo.hpp"
#include "text_renderer.hpp"
#include "shader_resource.hpp"
#include "model.hpp"
#include <UsefulUtils/dispatch_queue.hpp>

class NaivePathTracer : public Demo {

public:

    ~NaivePathTracer();

    void setup(gfx::LogicalDevicePtr logical_device, int32_t num_swap, int32_t width, int32_t height) override;

    gfx::ComputeTexturePtr
    renderFrame(gfx::CommandBufferPtr command_buffer, 
                uint32_t image_index, 
                uint32_t frame) override;
    
    std::string name() override { return "NaivePathTracer"; }

private:

    struct Camera {
        alignas(16) glm::vec4 position;
        alignas(16) glm::vec4 direction;
        alignas(4) float focal_length;
        alignas(4) float width;
        alignas(4) float height;   
        alignas(4) uint32_t x_res;
        alignas(4) uint32_t y_res;
    } alignas(64);

    struct Ray {
        alignas(16) glm::vec4 origin;
        alignas(16) glm::vec4 direction;
        alignas(16) glm::vec4 weight;
        alignas(16) glm::vec4 accumulation;
        alignas(8) glm::vec2 coord;
        int32_t valid = true;
    };
 
    struct HitPoint {
        alignas(16) glm::vec4 pos = glm::vec4(0.f, 0.f, 0.f, 1.f);
        alignas(16) glm::vec4 norm = glm::vec4(0.f);
        alignas(16) glm::vec4 col = glm::vec4(0.f);
        alignas(16) glm::vec4 emission = glm::vec4(0.f);
        alignas(4) float t = -1;
    };

    struct Material {
        Material(glm::vec4 diffuse, glm::vec4 emissive = glm::vec4(0)) 
        : diffuse_color(diffuse)
        , emissive_color(emissive) {}
        alignas(16) glm::vec4 diffuse_color = glm::vec4(0.f);
        alignas(16) glm::vec4 emissive_color = glm::vec4(0.f);
    };

    struct BoundingBox {
        glm::vec4 min_pos = glm::vec4(10000000000);
        glm::vec4 max_pos = glm::vec4(-10000000000);
        BoundingBox(const std::vector<glm::vec4>& vertices) {
            for(auto v : vertices) {
                if (v.x < min_pos.x) min_pos.x = v.x;
                if (v.y < min_pos.y) min_pos.y = v.y;
                if (v.z < min_pos.z) min_pos.z = v.z;
                if (v.x > max_pos.x) max_pos.x = v.x;
                if (v.y > max_pos.y) max_pos.y = v.y;
                if (v.z > max_pos.z) max_pos.z = v.z;
            }
        }
    };


    struct Mesh {
        Mesh(gfx::LogicalDevicePtr logical_device, 
            std::vector<glm::vec4> in_vertices,
            std::vector<uint32_t> in_indices,
            Material in_material) 
            : material(in_material) {
                bbox = BoundingBox(in_vertices);
                vertices = gfx::ComputeBuffer::createFromVector(
                                logical_device, in_vertices, vk::BufferUsageFlagBits::eStorageBuffer);
                
                std::vector<glm::ivec4> in_triangles;
                CXL_DCHECK(in_indices.size() % 3 == 0);
                for (uint32_t i = 0; i < in_indices.size(); i += 3) {
                    in_triangles.push_back(glm::ivec4(
                        in_indices[i],
                        in_indices[i+1],
                        in_indices[i+2],
                        0U
                    ));
                }
                
                num_triangles = in_indices.size() / 3;
                triangles = gfx::ComputeBuffer::createFromVector(
                                logical_device, in_triangles, vk::BufferUsageFlagBits::eStorageBuffer);

        }

        Material material;
        std::optional<BoundingBox> bbox;
        gfx::ComputeBufferPtr vertices;
        gfx::ComputeBufferPtr triangles;
        uint32_t num_triangles;

        static Mesh createRectangle(gfx::LogicalDevicePtr logical_device,
                                    glm::vec4 v0, 
                                    glm::vec4 v1, 
                                    glm::vec4 v2, 
                                    glm::vec4 v3,
                                    Material material) {
            return Mesh(logical_device, {v0, v1, v2, v3}, {0,1,2,0,2,3}, material);
        }
    };



    Camera camera_;
    std::vector<Mesh> meshes_;

    std::shared_ptr<TextRenderer> text_renderer_;

    std::vector<gfx::RenderPassInfo> accumulation_passes_;
    std::vector<gfx::RenderPassInfo> render_passes_;

    std::shared_ptr<christalz::ShaderResource> mwc64x_seeder_;
    std::shared_ptr<christalz::ShaderResource> rng_seeder_;
    std::shared_ptr<christalz::ShaderResource> ray_generator_;
    std::shared_ptr<christalz::ShaderResource> hit_tester_;
    std::shared_ptr<christalz::ShaderResource> bouncer_;
    std::shared_ptr<christalz::ShaderResource> lighter_;
    std::shared_ptr<christalz::ShaderResource> resolve_;

    std::vector<gfx::ComputeTexturePtr> accum_textures_;
    std::vector<gfx::ComputeTexturePtr> resolve_textures_;

    std::vector<gfx::CommandBufferPtr> compute_command_buffers_;

    std::vector<vk::Semaphore> compute_semaphores_;

    std::vector<vk::Fence> compute_fences_;

    std::vector<gfx::ComputeBufferPtr> rays_;
    std::vector<gfx::ComputeBufferPtr> hits_;
    std::vector<gfx::ComputeBufferPtr> random_seeds_;
    std::unique_ptr<cxl::DispatchQueue> dispatch_queue_;
};

#endif // NAIVE_PATH_TRACER_HPP_
