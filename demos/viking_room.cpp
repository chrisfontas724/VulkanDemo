// Copyright 2023 Sic Studios. All rights reserved.
// Use of this source code is governed by our license that can be
// found in the LICENSE file.


#include "viking_room.hpp"

namespace {

const int MAX_FRAMES_IN_FLIGHT = 2;

struct UniformBufferObject {
    alignas(16) glm::mat4 model;
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;
};

glm::vec3 eye_pos = glm::vec3(2, 2, 2);
glm::vec3 direction = glm::normalize(glm::vec3(0) - eye_pos);

gfx::CommandBufferState::DefaultState default_state_ =
    gfx::CommandBufferState::DefaultState::kOpaque;

float degrees = 90;

} // anonymous namespace

VikingRoom::VikingRoom(uint32_t width, uint32_t height) 
: Demo("Viking Room", width, height) {

    const auto& swapchain_textures = swap_chain_->textures();
    auto num_swap = swapchain_textures.size();

    command_buffers_ = gfx::CommandBuffer::create(logical_device_, gfx::Queue::Type::kGraphics,
                                                  vk::CommandBufferLevel::ePrimary, num_swap);
    CXL_DCHECK(command_buffers_.size() == num_swap);

    render_semaphores_ = logical_device_->createSemaphores(MAX_FRAMES_IN_FLIGHT);

    text_renderer_ = std::make_shared<TextRenderer>(logical_device_);

    gfx::RenderPassBuilder builder(logical_device_);
    vk::SampleCountFlagBits samples = vk::SampleCountFlagBits::e4;

    color_textures_.resize(num_swap);
    for (uint32_t i = 0; i < num_swap; i++) {
        color_textures_[i] = gfx::ImageUtils::createColorAttachment(logical_device_, width, height, samples);
        CXL_DCHECK(color_textures_[i]);
    }
    
    resolve_textures_.resize(num_swap);
    for (uint32_t i = 0; i < num_swap; i++) {
        resolve_textures_[i] = gfx::ImageUtils::createColorAttachment(logical_device_, width,
                                                                     height, vk::SampleCountFlagBits::e1);
        CXL_DCHECK(resolve_textures_[i]);
    }

    depth_textures_.resize(num_swap);
    for (uint32_t i = 0; i < num_swap; i++) {
      depth_textures_[i] =
        gfx::ImageUtils::createDepthTexture(logical_device_, width, height, samples);
      CXL_DCHECK(depth_textures_[i]);
    }

    uint32_t tex_index = 0;
    for (const auto& texture : swapchain_textures) {
        CXL_DCHECK(texture);
        builder.reset();
        builder.addColorAttachment(color_textures_[tex_index]);
        builder.addDepthAttachment(depth_textures_[tex_index]);
        builder.addResolveAttachment(resolve_textures_[tex_index]);

        builder.addSubpass({.bind_point = vk::PipelineBindPoint::eGraphics,
                            .input_indices = {},
                            .color_indices = {0},
                            .resolve_index = 0,
                            .depth_index = 0});
        tex_index++;

        render_passes_.push_back(std::move(builder.build()));
    }


    for (const auto& texture : swapchain_textures) {
        builder.reset();
        builder.addColorAttachment(texture);
        builder.addSubpass({.bind_point = vk::PipelineBindPoint::eGraphics,
                            .input_indices = {},
                            .color_indices = {0}});
        display_render_passes_.push_back(std::move(builder.build()));
    }


    cxl::FileSystem fs("c:/Users/Chris/Desktop/Rendering Projects/VulkanDemo/data/shaders/");
    model_shader_ = christalz::ShaderResource::createGraphics(logical_device_, fs, "lighting/model");
    post_shader_ = christalz::ShaderResource::createGraphics(logical_device_, fs, "posteffects/post");
    model_ = std::make_shared<christalz::Model>(logical_device_, 
        "C:/Users/Chris/Desktop/Rendering Projects/VulkanDemo/data/models/viking_room.obj", 
        "C:/Users/Chris/Desktop/Rendering Projects/VulkanDemo/data/textures/viking_room.png");
    CXL_DCHECK(model_shader_);
    CXL_DCHECK(post_shader_);
    CXL_DCHECK(model_);

    ubo_buffer_ = gfx::ComputeBuffer::createHostAccessableUniform(logical_device_,
                                                                      sizeof(UniformBufferObject));
    CXL_DCHECK(ubo_buffer_);
}


VikingRoom::~VikingRoom() {
    logical_device_->waitIdle();

    for (auto& pass : render_passes_) {
        logical_device_->vk().destroyRenderPass(pass.render_pass);
    }

    for (auto& pass : display_render_passes_) {
        logical_device_->vk().destroyRenderPass(pass.render_pass);
    }

    for (auto& semaphore : render_semaphores_) {
        logical_device_->vk().destroy(semaphore);
    }

    for (auto& texture: color_textures_) {
        texture.reset();
    }
    for (auto& texture: resolve_textures_) {
        texture.reset();
    }
    for (auto& texture: depth_textures_) {
        texture.reset();
    }


    command_buffers_.clear();

    model_shader_.reset();
    post_shader_.reset();
}


int32_t VikingRoom::run() {
   uint32_t sample = 1;
    while (!window_->shouldClose()) {
        window_->poll();

        {
            UniformBufferObject ubo; 
            ubo.model = glm::rotate(glm::mat4(1.0f), glm::radians(degrees), glm::vec3(0.0f, 0.0f, 1.0f));
            ubo.view = glm::lookAt(eye_pos, eye_pos + direction, glm::vec3(0.0f, 0.0f, 1.0f));
            ubo.proj = glm::perspective(glm::radians(45.0f),
                                    float(window_config_.width) / float(window_config_.height), 0.1f, 100.0f);
            ubo.proj[1][1] *= -1;
            ubo_buffer_->write(&ubo, 1);
            degrees += 0.01;
        }


        swap_chain_->beginFrame([&](vk::Semaphore& semaphore, vk::Fence& fence, uint32_t image_index,
                                    uint32_t frame) -> std::vector<vk::Semaphore> {
            // Record graphics commands.
            auto graphics_buffer = command_buffers_[image_index];
            graphics_buffer->reset();
            graphics_buffer->beginRecording();
            graphics_buffer->beginRenderPass(render_passes_[image_index]);

            {       
                graphics_buffer->setVertexAttribute(/*binding*/ 0, /*location*/ 0,
                                                    /*format*/ vk::Format::eR32G32B32A32Sfloat);
                graphics_buffer->setVertexAttribute(/*binding*/ 0, /*location*/ 1,
                                                    /*format*/ vk::Format::eR32G32B32A32Sfloat);
                graphics_buffer->setVertexAttribute(/*binding*/ 0, /*location*/ 2,
                                                    /*format*/ vk::Format::eR32G32Sfloat);                              
                graphics_buffer->setProgram(model_shader_->program());
                graphics_buffer->bindVertexBuffer(model_->vertices());
                graphics_buffer->bindIndexBuffer(model_->indices());
                graphics_buffer->bindUniformBuffer(0, 0, ubo_buffer_);
                graphics_buffer->bindTexture(0, 1, model_->texture());
                graphics_buffer->setDefaultState(default_state_);
                graphics_buffer->setDepth(/*test*/ true, /*write*/ true);
                graphics_buffer->drawIndexed(model_->num_indices());
            }
            
            // Render Debug Text.
            {
                std::string text = "sample: " + std::to_string(sample);
                graphics_buffer->setDefaultState(default_state_);
                text_renderer_->renderText(graphics_buffer, text, {-0.9, 0.8}, {-0.5, 0.9}, text.size());
                sample++;
            }

            graphics_buffer->endRenderPass();

            resolve_textures_[image_index]->transitionImageLayout(*graphics_buffer.get(), vk::ImageLayout::eShaderReadOnlyOptimal);

            {
                graphics_buffer->beginRenderPass(display_render_passes_[image_index]);
                graphics_buffer->setProgram(post_shader_->program());
                graphics_buffer->setDefaultState(gfx::CommandBufferState::DefaultState::kOpaque);
                graphics_buffer->bindTexture(0, 0, resolve_textures_[image_index]);
                graphics_buffer->setDepth(/*test*/ false, /*write*/ false);
                graphics_buffer->draw(3);
                graphics_buffer->endRenderPass();
            }

            resolve_textures_[image_index]->transitionImageLayout(*graphics_buffer.get(), vk::ImageLayout::eColorAttachmentOptimal);
            graphics_buffer->endRecording();

            // Submit graphics commands.
            vk::PipelineStageFlags graphicsWaitStages[] = {
                vk::PipelineStageFlagBits::eColorAttachmentOutput};
            vk::SubmitInfo submit_info(1U, &semaphore, graphicsWaitStages, 1U,
                                       &graphics_buffer->vk(), 1U, &render_semaphores_[frame]);
            logical_device_->getQueue(gfx::Queue::Type::kPresent).submit(submit_info, fence);

            return {render_semaphores_[frame]};
         });
    }
    return 0;
}