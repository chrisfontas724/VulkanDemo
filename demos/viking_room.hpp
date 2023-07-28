// Copyright 2023 Sic Studios. All rights reserved.
// Use of this source code is governed by our license that can be
// found in the LICENSE file.


#ifndef VIKING_ROOM_HPP_
#define VIKING_ROOM_HPP_

#include <string>
#include "demo.hpp"
#include "text_renderer.hpp"
#include "shader_resource.hpp"
#include "model.hpp"

class VikingRoom : public Demo {

public:

    VikingRoom(uint32_t width, uint32_t height);
    ~VikingRoom();

    int32_t run() override;

private:

    std::shared_ptr<TextRenderer> text_renderer_;
    std::vector<gfx::CommandBufferPtr> command_buffers_;

    std::vector<gfx::RenderPassInfo> render_passes_;
    std::vector<gfx::RenderPassInfo> display_render_passes_;
    std::vector<vk::Semaphore> render_semaphores_;

    std::shared_ptr<christalz::ShaderResource> model_shader_;
    std::shared_ptr<christalz::ShaderResource> post_shader_;
    std::shared_ptr<christalz::Model> model_;

    gfx::ComputeBufferPtr ubo_buffer_;

    std::vector<gfx::ComputeTexturePtr> color_textures_;
    std::vector<gfx::ComputeTexturePtr> resolve_textures_;
    std::vector<gfx::ComputeTexturePtr> depth_textures_;
};

#endif // VIKING_ROOM_HPP_
