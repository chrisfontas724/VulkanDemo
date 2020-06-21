// Copyright 2019 Chris Fontas. All rights reserved.
// Use of this source code is governed by the license that can be
// found in the LICENSE file.

#include "display/window.hpp"
#include "streaming/file_system.hpp"
#include "stdio.h"
#include <iostream>

#include "logging/logging.hpp"

#include "vk_wrappers/instance.hpp"
#include "vk_wrappers/physical_device.hpp"
#include "vk_wrappers/logical_device.hpp"
#include  "vk_wrappers/render_pass.hpp"
#include "vk_wrappers/swap_chain.hpp"
#include "vk_wrappers/command_buffer.hpp"
#include "vk_wrappers/forward_declarations.hpp"
#include "vk_wrappers/utils/shader_compiler.hpp"
#include "vk_wrappers/shader_program.hpp"

#include <glm/vec2.hpp>

INITIALIZE_EASYLOGGINGPP


const char* kVertexShader = R"(
    layout(location = 0) in vec4 in_position;
    layout(location = 1) in vec4 in_color;

    layout(location = 0) out vec4 out_color;

    void main() {
        out_color = in_color;
        gl_Position = vec4(in_position.xy, 0.5, 1.0);
    }
)";


const char* kFragmentShader = R"(
    layout(location = 0) in vec4 in_color;
    layout(location = 0) out vec4 out_color;
    void main() {
        out_color = in_color;
    }
)";

gfx::CommandBufferState::DefaultState default_state_ = gfx::CommandBufferState::DefaultState::kOpaque;

// Example dummy delegate class.
class Delegate : public display::WindowDelegate {
public:
    
    // |WindowDelegate|
    void onUpdate() override { }
    
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
    }
    // etc...
}

// Set up a window with the delegate and start polling.
int main(int argc, char** argv) {
    START_EASYLOGGINGPP(argc, argv);

    display::Window::Config config;
    config.name = "Vulkan Demo";
    config.width = 1024;
    config.height = 768;
    config.type =  display::Window::Type::kGLFW;
    auto delegate = std::make_shared<Delegate>();
    auto window = display::Window::create(config, std::move(delegate));
    std::cout << "Created window!" << std::endl;

    // Create vulkan instance.
    auto instance = gfx::Instance::create("VulkanDemo", window->getExtensions(), /*validation*/true);
    
    // Create display surface KHR.
    vk::SurfaceKHR surface = window->createVKSurface(instance->vk());

    // Create device extension list.
    auto device_extensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME, VK_NV_RAY_TRACING_EXTENSION_NAME};

    // Pick the best device given the provided surface.
    auto physical_device = instance->pickBestDevice(surface, device_extensions);
    CXL_VLOG(3) << "The best physical device is " << physical_device->name();

    // Make a logical device from the physical device.
    auto logical_device = std::make_shared<gfx::LogicalDevice>(physical_device, surface, device_extensions);
    CXL_VLOG(3) << "Successfully created a logical device!";

    // Create swapchain.
    auto swap_chain = std::make_unique<gfx::SwapChain>(logical_device, surface, config.width, config.height);
    auto num_frame_buffers = 3;
    CXL_VLOG(3) << "Successfully created a swapchain!";

    // Create command buffers.
    auto graphics_command_buffers = gfx::CommandBuffer::create(logical_device, gfx::Queue::Type::kGraphics,
                                                               vk::CommandBufferLevel::ePrimary,
                                                                num_frame_buffers);
    CXL_VLOG(3) << "Successfully created command buffers!";

    const int MAX_FRAMES_IN_FLIGHT = 2;
    auto render_semaphores = logical_device->createSemaphores(MAX_FRAMES_IN_FLIGHT);
    CXL_VLOG(3) << "Created render semaphores!";

    auto fs = cxl::FileSystem::getDesktop();
    
    if (!window->supports_vulkan()) {
        std::cerr << "Window doesn't support vulkan.";
        return 1;
    }

    gfx::RenderPassBuilder builder(logical_device);
    auto swapchain_textures = swap_chain->textures();
    std::vector<gfx::RenderPassInfo> render_passes;
    for (const auto& texture : swapchain_textures) {
        CXL_DCHECK(texture);
        builder.reset();
        builder.addColorAttachment(texture);
        builder.addSubpass({
            .bind_point = vk::PipelineBindPoint::eGraphics,
            .input_indices = {},
            .color_indices = {0},
        });

        render_passes.push_back(std::move(builder.build()));
    }    


    gfx::SpirV vertex_spirv, fragment_spirv;
    gfx::ShaderCompiler compiler;
    compiler.compile(EShLanguage::EShLangVertex, kVertexShader, {}, {},  &vertex_spirv);
    compiler.compile(EShLanguage::EShLangFragment, kFragmentShader, {}, {}, &fragment_spirv);
    CXL_DCHECK(vertex_spirv.size() > 0);
    CXL_DCHECK(fragment_spirv.size() > 0);

    auto shader_program = gfx::ShaderProgram::createGraphics(logical_device, vertex_spirv, fragment_spirv);
    CXL_DCHECK(shader_program);

    struct Vertex{
        glm::vec4 pos;
        glm::vec4 col;
    };

    std::vector<Vertex> vertices;
    vertices.push_back({.pos = glm::vec4(0, -1, 0, 1), .col = glm::vec4(1,0,0,1)});
    vertices.push_back({.pos = glm::vec4(1, 0, 0, 1),  .col = glm::vec4(0,1,0,1)});
    vertices.push_back({.pos = glm::vec4(0, 1, 0, 1), .col = glm::vec4(1,1,1,1)});
    vertices.push_back({.pos = glm::vec4(-1, 0, 0, 1), .col = glm::vec4(1,0,1,1)});
    auto vertex_buffer = gfx::ComputeBuffer::createFromVector(logical_device, vertices, vk::BufferUsageFlagBits::eVertexBuffer);
    CXL_DCHECK(vertex_buffer);

    std::vector<uint32_t> indices = {0, 1, 2, 0, 2, 3};
    auto index_buffer = gfx::ComputeBuffer::createFromVector(logical_device, indices, vk::BufferUsageFlagBits::eIndexBuffer);
    CXL_DCHECK(index_buffer);

    std::cout << "Begin loop!" << std::endl;
    while (!window->shouldClose()) {
        window->poll();
        checkInput(window->input_manager());


        swap_chain->beginFrame([&](
             vk::Semaphore& semaphore, 
             vk::Fence& fence, 
             uint32_t image_index, 
             uint32_t frame) -> std::vector<vk::Semaphore> {

            // Record graphics commands.
            gfx::CommandBuffer& graphics_buffer = graphics_command_buffers[image_index];
            graphics_buffer.reset();
            graphics_buffer.beginRecording();
            graphics_buffer.beginRenderPass(render_passes[image_index], glm::vec4(0,0,0,1));
            graphics_buffer.setVertexAttribute(/*binding*/0, /*location*/0, /*offset*/0, /*format*/vk::Format::eR32G32B32A32Sfloat);
            graphics_buffer.setVertexAttribute(/*binding*/0, /*location*/1, /*offset*/16, /*format*/vk::Format::eR32G32B32A32Sfloat);
            graphics_buffer.setProgram(shader_program);
            graphics_buffer.bindVertexBuffer(vertex_buffer.get());
            graphics_buffer.bindIndexBuffer(index_buffer.get());
            graphics_buffer.setDefaultState(default_state_);
            graphics_buffer.setDepth(/*test*/false, /*write*/false);
            graphics_buffer.drawIndexed(6);
            graphics_buffer.endRenderPass();
            graphics_buffer.endRecording();

            // Submit graphics commands.
            vk::PipelineStageFlags graphicsWaitStages[] = {vk::PipelineStageFlagBits::eColorAttachmentOutput};
            vk::SubmitInfo submit_info(1U, &semaphore, graphicsWaitStages, 1U, &graphics_buffer.vk(), 1U, &render_semaphores[frame]);
            logical_device->getQueue(gfx::Queue::Type::kGraphics).submit(submit_info, fence);
 
            return { render_semaphores[frame] };
        });
    
    }
    return 0;
}

