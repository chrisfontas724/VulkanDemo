// Copyright 2019 Chris Fontas. All rights reserved.
// Use of this source code is governed by the license that can be
// found in the LICENSE file.

#include <iostream>
#include <unordered_map>

#include <windowing/window.hpp>
#include <core/logging.hpp>
#include "stdio.h"
#include <streaming/file_system.hpp>
#include  <vk_wrappers/command_buffer.hpp>
#include  <vk_wrappers/forward_declarations.hpp>
#include  <vk_wrappers/instance.hpp>
#include  <vk_wrappers/logical_device.hpp>
#include  <vk_wrappers/physical_device.hpp>
#include  <vk_wrappers/render_pass.hpp>
#include  <vk_wrappers/shader_program.hpp>
#include  <vk_wrappers/swap_chain.hpp>
#include  <vk_wrappers/image_utils.hpp>
#include  <vk_wrappers/shader_compiler.hpp>

#include <demo/application_runner.hpp>
#include <demo/vk_raytracer.hpp>
#include <demo/model.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>
#include <glm/vec2.hpp>

INITIALIZE_EASYLOGGINGPP

const std::string MODEL_PATH = "../../data/viking_room.obj";
const std::string TEXTURE_PATH = "../../data/viking_room.png";

const uint32_t kDisplayWidth = 1800;
const uint32_t kDisplayHeight = 1100;

const char* kVertexShader = R"(

    layout(set = 0, binding = 0) uniform UniformBufferObject {
        mat4 model;
        mat4 view;
        mat4 proj;
    } ubo;


    layout(location = 0) in vec4 in_position;
    layout(location = 1) in vec4 in_color;
    layout(location = 2) in vec2 in_uvs;

    layout(location = 0) out vec4 out_color;
    layout(location = 1) out vec2 out_uvs;

    void main() {
        out_color = in_color;
        out_uvs = in_uvs;
        gl_Position = ubo.proj * ubo.view * ubo.model * vec4(in_position.xyz, 1.0);
    }
)";

const char* kFragmentShader = R"(
    layout(location = 0) in vec4 in_color;
    layout(location = 1) in vec2 in_uvs;
    layout(location = 0) out vec4 out_color;

    layout(set = 0, binding = 1) uniform sampler2D image;

    void main() {
        out_color = in_color * texture(image, in_uvs);
    }
)";

const char* kFullScreenVertexShader = R"(
// Harded values for a fullscreen quad in normalized device coordinates.
// Instead of actually rendering a quad, we render an oversized triangle
// that fits over the rendered screen.
vec2 positions[3] = vec2[](
    vec2(-1.0, -1.0),
    vec2(-1.0, 3.0),
    vec2(3.0, -1.0)
);

vec2 uv_coords[3] = vec2[](
    vec2(0, 0),
    vec2(0, 2),
    vec2(2, 0)
);

layout(location = 0) out vec2 uv_coord;

// Simply write out the position with no transformation.
void main() {
    gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);
    uv_coord = uv_coords[gl_VertexIndex];
}
)";

const char* kFullScreenFragmentShader = R"(
layout(location = 0) in vec2 uv_coord;

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform sampler2D scene_texture;
void main() {
   vec3 tex_col = max(vec3(0), texture(scene_texture, uv_coord).rgb);
   float lum = dot(tex_col, vec3(.3, .6, .1));
   outColor = vec4(vec3(lum), 1.0);
}
)";


struct UniformBufferObject {
    alignas(16) glm::mat4 model;
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;
};

glm::vec3 eye_pos = glm::vec3(2, 2, 2);
glm::vec3 direction = glm::normalize(glm::vec3(0) - eye_pos);

gfx::CommandBufferState::DefaultState default_state_ =
    gfx::CommandBufferState::DefaultState::kOpaque;

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

    display::Window::Config config;
    config.name = "Vulkan Demo";
    config.width = kDisplayWidth;
    config.height = kDisplayHeight;
    auto window = std::make_shared<display::GLFWWindow>(config);
    auto engine = std::make_shared<dali::VKRayTracer>(/*validation*/true);
    auto app_runner = christalz::ApplicationRunner::create(window, engine);

    auto& logical_device = engine->logical_device_;
    auto& swap_chain = engine->swap_chain_;
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

    auto fs = cxl::FileSystem::getDesktop();


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

    CXL_VLOG(5) << "Compiling shaders...";
    gfx::SpirV vertex_spirv, fragment_spirv;
    gfx::ShaderCompiler compiler;
    compiler.compile(EShLanguage::EShLangVertex, kVertexShader, {}, {}, &vertex_spirv);
    compiler.compile(EShLanguage::EShLangFragment, kFragmentShader, {}, {}, &fragment_spirv);
    CXL_DCHECK(vertex_spirv.size() > 0);
    CXL_DCHECK(fragment_spirv.size() > 0);

    CXL_VLOG(5) << "Creating shader program...";
    auto shader_program =
        gfx::ShaderProgram::createGraphics(logical_device, vertex_spirv, fragment_spirv);
    CXL_DCHECK(shader_program);

    compiler.compile(EShLanguage::EShLangVertex, kFullScreenVertexShader, {}, {}, &vertex_spirv);
    compiler.compile(EShLanguage::EShLangFragment, kFullScreenFragmentShader, {}, {}, &fragment_spirv);
    CXL_DCHECK(vertex_spirv.size() > 0);
    CXL_DCHECK(fragment_spirv.size() > 0);

    CXL_VLOG(5) << "Creating shader program...";
    auto display_shader_program =
        gfx::ShaderProgram::createGraphics(logical_device, vertex_spirv, fragment_spirv);
    CXL_DCHECK(display_shader_program);

    CXL_VLOG(5) << "Loading model...";
    auto model = std::make_shared<christalz::Model>(logical_device, MODEL_PATH, TEXTURE_PATH);

    auto ubo_buffer = gfx::ComputeBuffer::createHostAccessableUniform(logical_device,
                                                                      sizeof(UniformBufferObject));
    CXL_DCHECK(ubo_buffer);

    float degrees = 90;

    std::cout << "Begin loop!" << std::endl;
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
            gfx::CommandBuffer& graphics_buffer = *graphics_command_buffers[image_index].get();
            graphics_buffer.reset();
            graphics_buffer.beginRecording();
            graphics_buffer.beginRenderPass(render_passes[image_index]);
            graphics_buffer.setVertexAttribute(/*binding*/ 0, /*location*/ 0,
                                               /*format*/ vk::Format::eR32G32B32A32Sfloat);
            graphics_buffer.setVertexAttribute(/*binding*/ 0, /*location*/ 1,
                                               /*format*/ vk::Format::eR32G32B32A32Sfloat);
            graphics_buffer.setVertexAttribute(/*binding*/ 0, /*location*/ 2,
                                               /*format*/ vk::Format::eR32G32Sfloat);
            graphics_buffer.setProgram(shader_program);
            graphics_buffer.bindVertexBuffer(model->vertices());
            graphics_buffer.bindIndexBuffer(model->indices());
            graphics_buffer.bindUniformBuffer(0, 0, ubo_buffer);
            graphics_buffer.bindTexture(0, 1, model->texture());
            graphics_buffer.setDefaultState(default_state_);
            graphics_buffer.setDepth(/*test*/ true, /*write*/ true);
            graphics_buffer.drawIndexed(model->num_indices());
            graphics_buffer.endRenderPass();

            resolve_textures[image_index]->transitionImageLayout(graphics_buffer, vk::ImageLayout::eShaderReadOnlyOptimal);

            graphics_buffer.beginRenderPass(display_render_passes[image_index]);
            graphics_buffer.setProgram(display_shader_program);
            graphics_buffer.setDefaultState(gfx::CommandBufferState::DefaultState::kOpaque);
            graphics_buffer.bindTexture(0, 0, resolve_textures[image_index]);
            graphics_buffer.setDepth(/*test*/ false, /*write*/ false);
            graphics_buffer.draw(3);
            graphics_buffer.endRenderPass();

            resolve_textures[image_index]->transitionImageLayout(graphics_buffer, vk::ImageLayout::eColorAttachmentOptimal);
            graphics_buffer.endRecording();

            // Submit graphics commands.
            vk::PipelineStageFlags graphicsWaitStages[] = {
                vk::PipelineStageFlagBits::eColorAttachmentOutput};
            vk::SubmitInfo submit_info(1U, &semaphore, graphicsWaitStages, 1U,
                                       &graphics_buffer.vk(), 1U, &render_semaphores[frame]);
            logical_device->getQueue(gfx::Queue::Type::kGraphics).submit(submit_info, fence);

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

    shader_program.reset();
    display_shader_program.reset();

    CXL_VLOG(5) << "Finished!!!";
    return 0;
}
