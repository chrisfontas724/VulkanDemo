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
#include  "vk_wrappers/utils/render_pass_utils.hpp"
#include "vk_wrappers/swap_chain.hpp"
#include "vk_wrappers/command_buffer.hpp"


INITIALIZE_EASYLOGGINGPP

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
    } else if (input->key(display::KeyCode::B)) {
        CXL_LOG(INFO) << "Pressed B";
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
    auto instance = gfx::Instance::create("VulkanDemo", window->getExtensions(), true);
    
    // Create display surface KHR.
    vk::SurfaceKHR surface = window->createVKSurface(instance->vk());

    // Pick the best device given the provided surface.
    auto physical_device = instance->pickBestDevice(surface);
    CXL_VLOG(3) << "The best physical device is " << physical_device->name();

    // Make a logical device from the physical device.
    auto logical_device = std::make_shared<gfx::LogicalDevice>(physical_device, surface);
    CXL_VLOG(3) << "Successfully created a logical device!";

    // Create display render pass
    auto support_details = physical_device->querySwapChainSupport(surface);
    auto display_format = gfx::SwapChain::chooseSwapSurfaceFormat(support_details.formats).format;
    auto display_render_pass = gfx::RenderPassUtils::createPresentationRenderPass(logical_device, display_format);

    // Create swapchain.
    auto swap_chain = std::make_unique<gfx::SwapChain>(logical_device, surface, config.width, config.height);

    // Create frame buffers.
    swap_chain->createFrameBuffers(display_render_pass);
    auto num_frame_buffers = swap_chain->frame_buffers().size();

    // Create command buffers.
    auto graphics_command_buffers = gfx::CommandBuffer::create(logical_device, gfx::Queue::Type::kGraphics,
                                                               vk::CommandBufferLevel::ePrimary,
                                                                num_frame_buffers);


    auto fs = cxl::FileSystem::getDesktop();
    std::cout << fs->directory() << std::endl;
    
    if (!window->supports_vulkan()) {
        std::cerr << "Window doesn't support vulkan.";
        return 1;
    }
    
    std::cout << "Begin loop!" << std::endl;
    while (!window->shouldClose()) {
        window->poll();
        checkInput(window->input_manager());
    }

    return 0;
}

