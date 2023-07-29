// Copyright 2023 Sic Studios. All rights reserved.
// Use of this source code is governed by our license that can be
// found in the LICENSE file.

#ifndef TEXT_RENDERER_HPP_
#define TEXT_RENDERER_HPP_

#include <glm/vec2.hpp>
#include <shader_program.hpp>
#include <command_buffer.hpp>

class TextRenderer {

public:

    TextRenderer(const gfx::LogicalDevicePtr& device);

    void renderText(gfx::CommandBufferPtr cmd_buffer,
                    std::string& text, 
                    glm::vec2 top_left, 
                    glm::vec2 bottom_right,
                    uint32_t num_per_row);

private:

    gfx::LogicalDeviceWeakPtr device_;
    gfx::ShaderProgramPtr shader_;
    gfx::ComputeTexturePtr glyph_texture_;
};

#endif // TEXT_RENDERER_HPP_