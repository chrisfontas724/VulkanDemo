// Copyright 2023 Sic Studios. All rights reserved.
// Use of this source code is governed by our license that can be
// found in the LICENSE file.

#include "text_renderer.hpp"
#include "stb_image.h"
#include <map>
#include <cmath>

#include <FileStreaming/file_stream.hpp>

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
    cxl::FileSystem fs(cxl::FileSystem::currentExecutablePath() + "/resources/spirv");
    shader_ = christalz::ShaderResource::createGraphics(device, fs, "text");
    CXL_DCHECK(shader_);
  }  

  // Create Texture
  {
    std::string texture_path = cxl::FileSystem::currentExecutablePath() + "/resources/textures/text_bitmap.png";
    int32_t texWidth, texHeight, texChannels;
    stbi_uc* pixels =
        stbi_load(texture_path.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    CXL_DCHECK(pixels);
    glyph_texture_ = gfx::ImageUtils::create8BitImage(device, texWidth, texHeight, vk::Format::eR8G8B8A8Unorm,
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

    cmd_buffer->setProgram(shader_->program());
    cmd_buffer->bindTexture(glyph_texture_, 0, 0);
    cmd_buffer->setDepth(/*test*/ false, /*write*/ false);
    cmd_buffer->setDefaultState(gfx::CommandBufferState::DefaultState::kTranslucent);

    for (int i = 0; i < text.size(); i++) {
        auto glyph = text[i];
        CXL_DCHECK(kGlyphMap.find(glyph) != kGlyphMap.end());
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

TextRenderer::~TextRenderer() {
  shader_.reset();
  glyph_texture_.reset();
}
