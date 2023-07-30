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

    NaivePathTracer(uint32_t width, uint32_t height);
    ~NaivePathTracer();

    int32_t run() override;

private:

    struct Camera {
        glm::vec4 position;
        glm::vec4 direction;
        glm::vec4 up;
        glm::vec2 sensor_size;
        float focal_length;
        float width;
        float height;
}   ;

    Camera camera_;

    std::shared_ptr<TextRenderer> text_renderer_;
    std::vector<gfx::CommandBufferPtr> command_buffers_;

    std::vector<gfx::RenderPassInfo> render_passes_;
    std::vector<gfx::RenderPassInfo> display_render_passes_;

    std::shared_ptr<christalz::ShaderResource> ray_generator_;
    std::shared_ptr<christalz::ShaderResource> hit_tester_;
    std::shared_ptr<christalz::ShaderResource> bouncer_;
    std::shared_ptr<christalz::ShaderResource> lighter_;

    std::vector<gfx::ComputeTexturePtr> color_textures_;
    std::vector<gfx::ComputeTexturePtr> resolve_textures_;

    std::vector<gfx::CommandBufferPtr> graphics_command_buffers_;
    std::vector<gfx::CommandBufferPtr> compute_command_buffers_;

    std::vector<vk::Semaphore> render_semaphores_;
    std::vector<vk::Semaphore> compute_semaphores_;

    std::vector<vk::Fence> compute_fences_;

    std::vector<gfx::ComputeBufferPtr> rays_;
    std::vector<gfx::ComputeBufferPtr> hits_;
    std::vector<gfx::ComputeBufferPtr> random_seeds_;
    std::unique_ptr<cxl::DispatchQueue> dispatch_queue_;
};

#endif // NAIVE_PATH_TRACER_HPP_
