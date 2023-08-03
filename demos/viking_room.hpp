// Copyright 2023 Sic Studios. All rights reserved.
// Use of this source code is governed by our license that can be
// found in the LICENSE file.


#ifndef VIKING_ROOM_HPP_
#define VIKING_ROOM_HPP_

#include <string>
#include "demo.hpp"
#include "text_renderer.hpp"
#include "shader_resource.hpp"
#include "compute_texture.hpp"
#include "model.hpp"

class VikingRoom : public Demo {

public:

    ~VikingRoom();

    void setup(gfx::LogicalDevicePtr logical_device, int32_t num_swap, int32_t width, int32_t height) override;

    gfx::ComputeTexturePtr
    renderFrame(gfx::CommandBufferPtr command_buffer, 
                uint32_t image_index, 
                uint32_t frame) override;

    std::string name() override { return "Viking Room"; }

private:
    std::shared_ptr<TextRenderer> text_renderer_;
    std::vector<gfx::RenderPassInfo> render_passes_;

    std::shared_ptr<christalz::ShaderResource> model_shader_;
    std::shared_ptr<christalz::Model> model_;

    gfx::ComputeBufferPtr ubo_buffer_;

    std::vector<gfx::ComputeTexturePtr> color_textures_;
    std::vector<gfx::ComputeTexturePtr> resolve_textures_;
    std::vector<gfx::ComputeTexturePtr> depth_textures_;
};

#endif // VIKING_ROOM_HPP_
