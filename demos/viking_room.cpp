// Copyright 2023 Sic Studios. All rights reserved.
// Use of this source code is governed by our license that can be
// found in the LICENSE file.


#include "viking_room.hpp"
#include <VulkanWrappers/command_buffer.hpp>

namespace {

const int MAX_FRAMES_IN_FLIGHT = 2;

struct UniformBufferObject {
    alignas(16) glm::mat4 model;
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;
};

glm::vec3 eye_pos = glm::vec3(2, 2, 2);
glm::vec3 direction = glm::normalize(glm::vec3(0) - eye_pos);
float degrees = 90;

} // anonymous namespace

void VikingRoom::setup(gfx::LogicalDevicePtr logical_device, int32_t num_swap, int32_t width, int32_t height) {
    logical_device_ = logical_device;
    num_swap_images_ = num_swap;

    cxl::FileSystem fs(cxl::FileSystem::currentExecutablePath() + "/resources/spirv");
    model_shader_ = christalz::ShaderResource::createGraphics(logical_device, fs, "model");
    model_ = std::make_shared<christalz::Model>(logical_device, 
       cxl::FileSystem::currentExecutablePath() + "/resources//models/viking_room.obj", 
       cxl::FileSystem::currentExecutablePath() + "/resources/textures/viking_room.png");
    CXL_DCHECK(model_shader_);
    CXL_DCHECK(model_);

    ubo_buffer_ = gfx::ComputeBuffer::createHostAccessableUniform(logical_device, sizeof(UniformBufferObject));
    CXL_DCHECK(ubo_buffer_);

    resize(width, height);
}

void VikingRoom::resize(uint32_t width, uint32_t height) {
    auto logical_device = logical_device_.lock();
    width_ = width;
    height_ = height;

    render_passes_.clear();
    color_textures_.clear();
    resolve_textures_.clear();
    depth_textures_.clear();


    gfx::RenderPassBuilder builder(logical_device);
    vk::SampleCountFlagBits samples = vk::SampleCountFlagBits::e4;

    color_textures_.resize(num_swap_images_);
    for (uint32_t i = 0; i < num_swap_images_; i++) {
        color_textures_[0] = gfx::ImageUtils::createColorAttachment(logical_device, width, height, samples);
        CXL_DCHECK(color_textures_[0]);
    }
    
    resolve_textures_.resize(num_swap_images_);
    for (uint32_t i = 0; i < num_swap_images_; i++) {
        resolve_textures_[0] = gfx::ImageUtils::createColorAttachment(logical_device, width,
                                                                     height, vk::SampleCountFlagBits::e1);
        CXL_DCHECK(resolve_textures_[0]);
    }

    depth_textures_.resize(num_swap_images_);
    for (uint32_t i = 0; i < num_swap_images_; i++) {
      depth_textures_[0] =
        gfx::ImageUtils::createDepthTexture(logical_device, width, height, samples);
      CXL_DCHECK(depth_textures_[0]);
    }

    for (int32_t tex_index = 0; tex_index < num_swap_images_; tex_index++) {
        builder.reset();
        builder.addColorAttachment(color_textures_[0]);
        builder.addDepthAttachment(depth_textures_[0]);
        builder.addResolveAttachment(resolve_textures_[0]);

        builder.addSubpass({.bind_point = vk::PipelineBindPoint::eGraphics,
                            .input_indices = {},
                            .color_indices = {0},
                            .resolve_index = 0,
                            .depth_index = 0});
        render_passes_.push_back(std::move(builder.build()));
    }
}

VikingRoom::~VikingRoom() {
    auto logical_device = logical_device_.lock();
    logical_device->waitIdle();

    ubo_buffer_.reset();

    for (auto& pass : render_passes_) {
        pass.reset();
    }

    color_textures_.clear();
    resolve_textures_.clear();
    depth_textures_.clear();

    model_.reset();
    model_shader_.reset();
}


gfx::ComputeTexturePtr
VikingRoom::renderFrame(gfx::CommandBufferPtr command_buffer, 
                        uint32_t image_index, uint32_t frame,
                        std::vector<vk::Semaphore>* signal_semaphores,
                        std::vector<vk::PipelineStageFlags>* signal_wait_stages) { 
    UniformBufferObject ubo; 
    ubo.model = glm::rotate(glm::mat4(1.0f), glm::radians(degrees), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.view = glm::lookAt(eye_pos, eye_pos + direction, glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.proj = glm::perspective(glm::radians(45.0f), float(width_) / float(height_), 0.1f, 100.0f);
    ubo.proj[1][1] *= -1;
    ubo_buffer_->write(&ubo, 1);
    degrees += 0.01;
         
    // Record graphics commands.
    resolve_textures_[0]->transitionImageLayout(*command_buffer.get(), vk::ImageLayout::eColorAttachmentOptimal);
    command_buffer->beginRenderPass(render_passes_[image_index]);
 
    command_buffer->setVertexAttribute(/*binding*/ 0, /*location*/ 0, /*format*/ vk::Format::eR32G32B32A32Sfloat);
    command_buffer->setVertexAttribute(/*binding*/ 0, /*location*/ 1, /*format*/ vk::Format::eR32G32B32A32Sfloat);
    command_buffer->setVertexAttribute(/*binding*/ 0, /*location*/ 2, /*format*/ vk::Format::eR32G32Sfloat);                              
    command_buffer->setProgram(model_shader_->program());
    command_buffer->bindVertexBuffer(model_->vertices());
    command_buffer->bindIndexBuffer(model_->indices());
    command_buffer->bindUniformBuffer(0, 0, ubo_buffer_);
    command_buffer->bindTexture(0, 1, model_->texture());
    command_buffer->setDefaultState(gfx::CommandBufferState::DefaultState::kOpaque);
    command_buffer->setDepth(/*test*/ true, /*write*/ true);
    command_buffer->drawIndexed(model_->num_indices());
    sample_++;

    command_buffer->endRenderPass();
    resolve_textures_[0]->transitionImageLayout(*command_buffer.get(), vk::ImageLayout::eShaderReadOnlyOptimal); 
    return resolve_textures_[0];
}


