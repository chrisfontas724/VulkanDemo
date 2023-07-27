// Copyright 2019 Chris Fontas. All rights reserved.
// Use of this source code is governed by the license that can be
// found in the LICENSE file.

#include <iostream>
#include <unordered_map>

#include <Windowing/window.hpp>
#include <UsefulUtils/logging.hpp>
#include "stdio.h"
#include <FileStreaming/file_system.hpp>
#include  <command_buffer.hpp>
#include  <forward_declarations.hpp>
#include  <instance.hpp>
#include  <logical_device.hpp>
#include  <physical_device.hpp>
#include  <render_pass.hpp>
#include  <swap_chain.hpp>
#include "text_renderer.hpp"

#include "application_runner.hpp"
#include <vk_raytracer.hpp>
#include <shader_resource.hpp>
#include <model.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>
#include <glm/vec2.hpp>

INITIALIZE_EASYLOGGINGPP

const std::string MODEL_PATH = "<FileStreaming/data/viking_room.obj";
const std::string TEXTURE_PATH = "<FileStreaming/data/viking_room.png";

const uint32_t kDisplayWidth = 1800;
const uint32_t kDisplayHeight = 1100;

struct UniformBufferObject {
    alignas(16) glm::mat4 model;
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;
};

glm::vec3 eye_pos = glm::vec3(2, 2, 2);
glm::vec3 direction = glm::normalize(glm::vec3(0) - eye_pos);

gfx::CommandBufferState::DefaultState default_state_ =
    gfx::CommandBufferState::DefaultState::kOpaque;

// Example dummy delegate class.
class Delegate : public display::WindowDelegate {
public:
    
    // |WindowDelegate|
    void onUpdate() override { /*std::cout << "onUpdate" << std::endl;*/ }
    
    void onResize(int32_t width, int32_t height) override {
        std::cout << "onResize" << std::endl;
    }
    
    void onWindowMove(int32_t x, int32_t y) override {
        std::cout << "onWindowMove" << std::endl;
    }
    
    void onStart(display::Window*) override {
        std::cout << "onStart" << std::endl;
    }
    
    void onClose() override {
        std::cout << "onClose" << std::endl;
    }
};


// Example InputManager checks.
void checkInput(const display::InputManager* input) {
    if (input->key(display::KeyCode::A)) {
        CXL_LOG(INFO) << "Pressed A";
        default_state_ = gfx::CommandBufferState::DefaultState::kOpaque;
    } else if (input->key(display::KeyCode::B)) {
        CXL_LOG(INFO) << "Pressed B";
        default_state_ = gfx::CommandBufferState::DefaultState::kWireFrame;
    } else if (input->key(display::KeyCode::C)) {
        CXL_LOG(INFO) << "Pressed C";
        default_state_ = gfx::CommandBufferState::DefaultState::kCustomRaytrace;
    } else if (input->key(display::KeyCode::D)) {
        eye_pos += glm::vec3(0.01f) * direction;
    } else if (input->key(display::KeyCode::E)) {
        eye_pos -= glm::vec3(0.01f) * direction;
    }
    // etc...
}

// Set up a window with the delegate and start polling.
int main(int argc, char** argv) {
    START_EASYLOGGINGPP(argc, argv);
    CXL_VLOG(3) << "Start app!";

    display::Window::Config config;
    config.name = "Vulkan Demo";
    config.width = kDisplayWidth;
    config.height = kDisplayHeight;
    auto delegate = std::make_shared<Delegate>();
    auto window = std::make_shared<display::GLFWWindow>(config, delegate);
    auto engine = std::make_shared<dali::VKRayTracer>(/*validation*/true);

    CXL_VLOG(3) << "Made window!";

    CXL_DCHECK(engine);

    engine->linkToWindow(window.get());


    auto& logical_device = engine->logical_device_;
    auto& swap_chain = engine->swap_chain_;
    CXL_DCHECK(swap_chain);

    const auto& swapchain_textures = swap_chain->textures();
    auto num_swap = swapchain_textures.size();

    auto num_frame_buffers = num_swap;
    CXL_VLOG(3) << "Successfully created a swapchain!";

    // Create command buffers.
    auto graphics_command_buffers =
        gfx::CommandBuffer::create(logical_device, gfx::Queue::Type::kGraphics,
                                   vk::CommandBufferLevel::ePrimary, num_frame_buffers);
    CXL_VLOG(3) << "Successfully created command buffers!";

    const int MAX_FRAMES_IN_FLIGHT = 2;
    auto render_semaphores = logical_device->createSemaphores(MAX_FRAMES_IN_FLIGHT);
    CXL_VLOG(3) << "Created render semaphores!";

    TextRenderer text_renderer(logical_device);

    gfx::RenderPassBuilder builder(logical_device);
    std::vector<gfx::RenderPassInfo> render_passes;
    std::vector<gfx::RenderPassInfo> display_render_passes;

    vk::SampleCountFlagBits samples = vk::SampleCountFlagBits::e4;

    CXL_VLOG(5) << "Creating color attachments............................";
    gfx::ComputeTexturePtr color_textures[num_swap];
    for (uint32_t i = 0; i < num_swap; i++) {
        color_textures[i] = gfx::ImageUtils::createColorAttachment(logical_device, kDisplayWidth,
                                                                   kDisplayHeight, samples);
        CXL_DCHECK(color_textures[i]);
    }


    CXL_VLOG(5) << "Creating resolve attachments............................";
    gfx::ComputeTexturePtr resolve_textures[num_swap];
    for (uint32_t i = 0; i < num_swap; i++) {
        resolve_textures[i] = gfx::ImageUtils::createColorAttachment(logical_device, kDisplayWidth,
                                                                   kDisplayHeight, vk::SampleCountFlagBits::e1);
        CXL_DCHECK(resolve_textures[i]);
    }


    CXL_VLOG(5) << "Creating depth texture attachments.....................";
    gfx::ComputeTexturePtr depth_textures[num_swap];
    for (uint32_t i = 0; i < num_swap; i++) {
      depth_textures[i] =
        gfx::ImageUtils::createDepthTexture(logical_device, kDisplayWidth, kDisplayHeight, samples);
    }

    uint32_t tex_index = 0;
    CXL_VLOG(5) << "Working on render pass builder..........................: " << swapchain_textures.size();
    for (const auto& texture : swapchain_textures) {
        CXL_DCHECK(texture);
        builder.reset();
        builder.addColorAttachment(color_textures[tex_index]);
        builder.addDepthAttachment(depth_textures[tex_index]);
        builder.addResolveAttachment(resolve_textures[tex_index]);

        builder.addSubpass({.bind_point = vk::PipelineBindPoint::eGraphics,
                            .input_indices = {},
                            .color_indices = {0},
                            .resolve_index = 0,
                            .depth_index = 0});
        tex_index++;

        render_passes.push_back(std::move(builder.build()));
        CXL_VLOG(5) << "BUILT!!!!!";
    }


    for (const auto& texture : swapchain_textures) {
        builder.reset();
        builder.addColorAttachment(texture);
        builder.addSubpass({.bind_point = vk::PipelineBindPoint::eGraphics,
                            .input_indices = {},
                            .color_indices = {0}});
        display_render_passes.push_back(std::move(builder.build()));
    }


    cxl::FileSystem fs("c:/Users/Chris/Desktop/Rendering Projects/VulkanDemo/data/shaders");
    auto model_shader = christalz::ShaderResource::createGraphics(logical_device, fs, "model");
    auto post_shader = christalz::ShaderResource::createGraphics(logical_device, fs, "post");
    auto model = std::make_shared<christalz::Model>(logical_device, 
    "C:/Users/Chris/Desktop/Rendering Projects/VulkanDemo/data/lucy_resized.obj", //viking_room.obj", 
    "C:/Users/Chris/Desktop/Rendering Projects/VulkanDemo/data/viking_room.png");

    auto ubo_buffer = gfx::ComputeBuffer::createHostAccessableUniform(logical_device,
                                                                      sizeof(UniformBufferObject));
    CXL_DCHECK(ubo_buffer);

    float degrees = 90;


    std::cout << "Begin loop!" << std::endl;
    uint32_t sample = 1;
    while (!window->shouldClose()) {
        window->poll();
        checkInput(window->input_manager());

        UniformBufferObject ubo{};
        ubo.model =
            glm::rotate(glm::mat4(1.0f), glm::radians(degrees), glm::vec3(0.0f, 0.0f, 1.0f));
        ubo.view = glm::lookAt(eye_pos, eye_pos + direction, glm::vec3(0.0f, 0.0f, 1.0f));
        ubo.proj = glm::perspective(glm::radians(45.0f),
                                    float(kDisplayWidth) / float(kDisplayHeight), 0.1f, 100.0f);
        ubo.proj[1][1] *= -1;

        degrees += 0.01;
        ubo_buffer->write(&ubo, 1);

        swap_chain->beginFrame([&](vk::Semaphore& semaphore, vk::Fence& fence, uint32_t image_index,
                                    uint32_t frame) -> std::vector<vk::Semaphore> {
            // Record graphics commands.
            auto graphics_buffer = graphics_command_buffers[image_index];
            graphics_buffer->reset();
            graphics_buffer->beginRecording();
            graphics_buffer->beginRenderPass(render_passes[image_index]);

            graphics_buffer->setVertexAttribute(/*binding*/ 0, /*location*/ 0,
                                               /*format*/ vk::Format::eR32G32B32A32Sfloat);
            graphics_buffer->setVertexAttribute(/*binding*/ 0, /*location*/ 1,
                                               /*format*/ vk::Format::eR32G32B32A32Sfloat);
            graphics_buffer->setVertexAttribute(/*binding*/ 0, /*location*/ 2,
                                               /*format*/ vk::Format::eR32G32Sfloat);
            graphics_buffer->setProgram(model_shader->program());
            graphics_buffer->bindVertexBuffer(model->vertices());
            graphics_buffer->bindIndexBuffer(model->indices());
            graphics_buffer->bindUniformBuffer(0, 0, ubo_buffer);
            graphics_buffer->bindTexture(0, 1, model->texture());
            graphics_buffer->setDefaultState(default_state_);
            graphics_buffer->setDepth(/*test*/ true, /*write*/ true);
            graphics_buffer->drawIndexed(model->num_indices());

            
            std::string text = "sa";//mple: " + sample;
            graphics_buffer->setDefaultState(default_state_);
            text_renderer.renderText(graphics_buffer, text, {-0.5, -0.5}, {0, 0}, 1);
            sample++;

            graphics_buffer->endRenderPass();

            resolve_textures[image_index]->transitionImageLayout(*graphics_buffer.get(), vk::ImageLayout::eShaderReadOnlyOptimal);

            graphics_buffer->beginRenderPass(display_render_passes[image_index]);
            graphics_buffer->setProgram(post_shader->program());
            graphics_buffer->setDefaultState(gfx::CommandBufferState::DefaultState::kOpaque);
            graphics_buffer->bindTexture(0, 0, resolve_textures[image_index]);
            graphics_buffer->setDepth(/*test*/ false, /*write*/ false);
            graphics_buffer->draw(3);
            
            graphics_buffer->endRenderPass();

            resolve_textures[image_index]->transitionImageLayout(*graphics_buffer.get(), vk::ImageLayout::eColorAttachmentOptimal);
            graphics_buffer->endRecording();

            // Submit graphics commands.
            vk::PipelineStageFlags graphicsWaitStages[] = {
                vk::PipelineStageFlagBits::eColorAttachmentOutput};
            vk::SubmitInfo submit_info(1U, &semaphore, graphicsWaitStages, 1U,
                                       &graphics_buffer->vk(), 1U, &render_semaphores[frame]);
            logical_device->getQueue(gfx::Queue::Type::kPresent).submit(submit_info, fence);

            return {render_semaphores[frame]};
         });
    }

    logical_device->waitIdle();
    ubo_buffer.reset();
    model.reset();

    for (auto& pass : render_passes) {
        logical_device->vk().destroyRenderPass(pass.render_pass);
    }

    for (auto& pass : display_render_passes) {
        logical_device->vk().destroyRenderPass(pass.render_pass);
    }

    for (auto& semaphore : render_semaphores) {
        logical_device->vk().destroy(semaphore);
    }

    for (auto& texture: color_textures) {
        texture.reset();
    }
    for (auto& texture: depth_textures) {
        texture.reset();
    }


    swap_chain.reset();

    graphics_command_buffers.clear();

    model_shader.reset();
    post_shader.reset();

    CXL_VLOG(5) << "Finished!!!";
    return 0;
}
