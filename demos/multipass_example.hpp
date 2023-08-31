// Copyright 2023 Sic Studios. All rights reserved.
// Use of this source code is governed by our license that can be
// found in the LICENSE file.


#ifndef MULTIPASS_EXAMLE_HPP_
#define MULTIPASS_EXAMLE_HPP_

#include "demo.hpp"
#include "src/text_renderer.hpp"
#include "src/model.hpp"
#include "src/shader_resource.hpp"
#include <VulkanWrappers/compute_texture.hpp>

class MultipassExample : public Demo {

public:

    ~MultipassExample();
    void setup(gfx::LogicalDevicePtr logical_device, int32_t num_swap, int32_t width, int32_t height) override;

    void resize(uint32_t width, uint32_t height) override;

    gfx::ComputeTexturePtr
    renderFrame(gfx::CommandBufferPtr command_buffer, 
                uint32_t image_index, 
                uint32_t frame,
                std::vector<vk::Semaphore>* signal_semaphores = nullptr,
                std::vector<vk::PipelineStageFlags>* signal_wait_stages = nullptr) override;
    
    std::string name() override { return "Multipass Example"; }

    void processEvent(display::InputEvent event) override {};

private:

    std::vector<gfx::RenderPassInfo> render_passes_;

    std::shared_ptr<christalz::ShaderResource> model_shader_;
    std::shared_ptr<christalz::Model> model_;

    std::vector<gfx::ComputeTexturePtr> first_pass_textures_;
    std::vector<gfx::ComputeTexturePtr> second_pass_textures_;
    std::vector<gfx::ComputeTexturePtr> depth_textures_;
};

#endif // MULTIPASS_EXAMLE_HPP_