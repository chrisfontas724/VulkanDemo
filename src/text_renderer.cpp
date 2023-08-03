// Copyright 2023 Sic Studios. All rights reserved.
// Use of this source code is governed by our license that can be
// found in the LICENSE file.

#include "text_renderer.hpp"
#include "stb_image.h"
#include <map>
#include <cmath>

namespace {

const std::map<char, glm::vec2> kGlyphMap = 
{
    // Ordered by (row, column)
    {' ', {0,0} },
    {'!', {0,1} },
    // TODO
    {'0', {1,6}},
    {'1', {1,7}},
    {'2', {1,8}},
    {'3', {1,9}},
    {'4', {2,0}},
    {'5', {2,1}},
    {'6', {2,2}},
    {'7', {2,3}},
    {'8', {2,4}},
    {'9', {2,5}},
    {':', {2,6}},
    // TODO
    {'a', {6,5}},
    {'e', {6, 9}},
    {'l', {7,6}},
    {'m', {7,7}},
    {'p', {8, 0}},
    {'s', {8,3}},
};

const float kWidth = 10, kHeight = 10;
const float kEpsilon = 0.005;

} // anonymous namespace


TextRenderer::TextRenderer(const gfx::LogicalDevicePtr& device)
: device_(device) {

  // Create Shader
  {
    cxl::FileSystem fs("c:/Users/Chris/Desktop/Rendering Projects/VulkanDemo/data/shaders/debug/");
    cxl::FileStream vert_shader;
    cxl::FileStream frag_shader;
    bool result = vert_shader.load(&fs, "text.vert");
    CXL_DCHECK(result);

    result = frag_shader.load(&fs, "text.frag");
    CXL_DCHECK(result);

    gfx::SpirV vertex_spirv, fragment_spirv;
    gfx::ShaderCompiler compiler;
    compiler.compile(EShLanguage::EShLangVertex, vert_shader.text(), {}, {}, &vertex_spirv);
    compiler.compile(EShLanguage::EShLangFragment, frag_shader.text(), {}, {}, &fragment_spirv);
    CXL_DCHECK(vertex_spirv.size() > 0);
    CXL_DCHECK(fragment_spirv.size() > 0);

    shader_ = gfx::ShaderProgram::createGraphics(device_.lock(), vertex_spirv, fragment_spirv); 
    shader_->name = "Test name";
  }  

  // Create Texture
  {
    std::string texture_path = "c:/Users/Chris/Desktop/Rendering Projects/VulkanDemo/data/textures/text_bitmap.png";
    int32_t texWidth, texHeight, texChannels;
    stbi_uc* pixels =
        stbi_load(texture_path.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    CXL_DCHECK(pixels);
    glyph_texture_ = gfx::ImageUtils::create8BitUnormImage(device, texWidth, texHeight, 4,
                                                    vk::SampleCountFlagBits::e1, pixels);
    CXL_DCHECK(glyph_texture_);
  }
}

void TextRenderer::renderText(gfx::CommandBufferPtr cmd_buffer,
                              std::string& text, 
                              glm::vec2 top_left, 
                              glm::vec2 bottom_right,
                              uint32_t num_per_row ) {
    uint32_t num_rows = (text.size() + num_per_row - 1) / num_per_row;
    float glyph_width = fabs(bottom_right.x - top_left.x) / num_per_row;
    float glyph_height = fabs(bottom_right.y - top_left.y) / num_rows;

    cmd_buffer->setProgram(shader_);
    cmd_buffer->bindTexture(0, 0, glyph_texture_);
    cmd_buffer->setDepth(/*test*/ false, /*write*/ false);
    cmd_buffer->setDefaultState(gfx::CommandBufferState::DefaultState::kTranslucent);

    for (int i = 0; i < text.size(); i++) {
        auto glyph = text[i];
        auto glyph_coords = kGlyphMap.at(glyph);

        int curr_col = i % num_per_row;
        int curr_row = i / num_per_row;
        glm::vec2 positions[4];
        positions[0] = { top_left.x + curr_col * glyph_width, top_left.y + curr_row * glyph_height };
        positions[1] = { top_left.x + (curr_col + 1) * glyph_width, top_left.y + curr_row * glyph_height};
        positions[2] = { top_left.x + (curr_col + 1) * glyph_width, top_left.y + (curr_row + 1) * glyph_height};
        positions[3] = { top_left.x + curr_col * glyph_width, top_left.y + (curr_row + 1) * glyph_height};

        glm::vec2 uvs[4];
        uvs[0] = glm::vec2(glyph_coords.y / kWidth, kEpsilon + glyph_coords.x / kHeight);
        uvs[1] = glm::vec2((glyph_coords.y + 1) / kWidth, kEpsilon + glyph_coords.x / kHeight);
        uvs[2] = glm::vec2((glyph_coords.y + 1) / kWidth, kEpsilon + (glyph_coords.x + 1) / kHeight);
        uvs[3] = glm::vec2(glyph_coords.y / kWidth, kEpsilon + (glyph_coords.x + 1) / kHeight);

        cmd_buffer->pushConstants(&positions[0], 0u, sizeof(glm::vec2)*4);
        cmd_buffer->pushConstants(&uvs[0], 32u, sizeof(glm::vec2)*4);
        cmd_buffer->pushConstants(color_, 64);
        cmd_buffer->draw(6);
    }
}