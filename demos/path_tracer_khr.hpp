// Copyright 2023 Sic Studios. All rights reserved.
// Use of this source code is governed by our license that can be
// found in the LICENSE file.


#ifndef PATH_TRACER_KHR_HPP_
#define PATH_TRACER_KHR_HPP_

#include <string>
#include "demo.hpp"
#include "src/text_renderer.hpp"
#include "src/shader_resource.hpp"
#include "src/model.hpp"
#include <UsefulUtils/dispatch_queue.hpp>
#include <VulkanWrappers/acceleration_structure.hpp>
#include <VulkanWrappers/ray_tracing_shader_manager.hpp>
#include <VulkanWrappers/compute_buffer.hpp>

class PathTracerKHR : public Demo {
public:

    ~PathTracerKHR();
    void setup(gfx::LogicalDevicePtr logical_device, int32_t num_swap, int32_t width, int32_t height) override;

    void resize(uint32_t width, uint32_t height) override;

    gfx::ComputeTexturePtr
    renderFrame(gfx::CommandBufferPtr command_buffer, 
                uint32_t image_index, 
                uint32_t frame,
                std::vector<vk::Semaphore>* signal_semaphores = nullptr,
                std::vector<vk::PipelineStageFlags>* signal_wait_stages = nullptr) override;
    
    std::string name() override { return "PathTracerKHR"; }

private:

    struct Camera {
        glm::mat4 matrix;
        float focal_length;
        float sensor_width;
        float sensor_height;
    };

    // Information of a obj model when referenced in a shader
    struct ObjDesc  {
        int      txtOffset;             // Texture index offset in the array of textures
        uint64_t vertexAddress;         // Address of the Vertex buffer
        uint64_t indexAddress;          // Address of the index buffer
        uint64_t materialAddress;       // Address of the material buffer
        uint64_t materialIndexAddress;  // Address of the triangle material index buffer
    };

    struct Material {
        Material(glm::vec4 diffuse, glm::vec4 emissive = glm::vec4(0)) 
        : diffuse_color(diffuse)
        , emissive_color(emissive) {}
        alignas(16) glm::vec4 diffuse_color = glm::vec4(0.f);
        alignas(16) glm::vec4 emissive_color = glm::vec4(0.f);
    };

    // Random
    std::shared_ptr<christalz::ShaderResource> mwc64x_seeder_;
    std::vector<gfx::ComputeBufferPtr> random_seeds_;

    // GPU data
    std::vector<gfx::CommandBufferPtr> compute_command_buffers_;
    std::vector<vk::Semaphore> compute_semaphores_;

    // Textures
    gfx::ComputeTexturePtr accum_textures_[2];
    gfx::ComputeTexturePtr resolve_texture_;
    uint32_t texture_index = 0;

    // Scene data.
    Camera camera_;
    std::vector<gfx::Geometry> geometries;

    gfx::ComputeBufferPtr obj_descriptions_;
    std::map<uint64_t, gfx::ComputeBufferPtr> materials_map_;
    std::shared_ptr<gfx::AccelerationStructure> as_;
    std::shared_ptr<gfx::RayTracingShaderManager> shader_manager_;
    gfx::Geometry createGeometry(const gfx::LogicalDevicePtr& logical_device, 
                                const std::vector<float>& positions, 
                                const std::vector<uint32_t>& indices,
                                const Material& material);

    gfx::Geometry createBBox(const gfx::LogicalDevicePtr& logical_device,
                                const Material& material);
};

#endif // PATH_TRACER_KHR_HPP_
