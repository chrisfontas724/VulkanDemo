// Copyright 2023 Sic Studios. All rights reserved.
// Use of this source code is governed by our license that can be
// found in the LICENSE file.


#include "demo_harness.hpp"
#include <iostream>

#include <UsefulUtils/logging.hpp>

#include <VulkanWrappers/command_buffer.hpp>
#include <VulkanWrappers/instance.hpp>

namespace {

uint32_t index = 0;
const int MAX_FRAMES_IN_FLIGHT = 2;


const std::vector<const char*> kDeviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

} // anonymous namespace


DemoHarness::DemoHarness(uint32_t width, uint32_t height) {
    std::string title = "DemoHarness";
    window_config_.title = title;
    window_config_.width = width;
    window_config_.height = height;
    window_delegate_ = std::make_shared<WindowDelegate>(this);

    platform_ = std::make_shared<display::Platform>(window_config_, window_delegate_);
    CXL_DCHECK(platform_);
}

void DemoHarness::initialize(PlatformNativeWindowHandle window, std::vector<const char*> extensions, int32_t width, int32_t height) {
    instance_ = gfx::Instance::create("DemoHarness", extensions, /*validation*/true);   
    CXL_DCHECK(instance_);

    surface_ = instance_->createSurface(window);

    physical_device_ = instance_->pickBestDevice(surface_, kDeviceExtensions);
    CXL_DCHECK(physical_device_);

    // Make a logical device from the physical device.
    logical_device_ =
        std::make_shared<gfx::LogicalDevice>(physical_device_, surface_, kDeviceExtensions);
    CXL_DCHECK(logical_device_);

    cxl::FileSystem fs("c:/Users/Chris/Desktop/Rendering Projects/VulkanDemo/data/shaders/");
    post_shader_ = christalz::ShaderResource::createGraphics(logical_device_, fs, "posteffects/post");
    CXL_DCHECK(post_shader_);

    recreateSwapchain(width, height);    
}


int32_t DemoHarness::run() {  
    platform_->runEventLoop();
    return 0;
}

void DemoHarness::render() {
    while (should_render_) {
        CXL_DCHECK(current_demo_);

        processInputEvents();
        swap_chain_->beginFrame([&](vk::Semaphore& image_available_semaphore, vk::Fence& in_flight_fence, uint32_t image_index,
                                    uint32_t frame) -> std::vector<vk::Semaphore> {
            auto command_buffer = command_buffers_[image_index];
            command_buffer->reset();
            command_buffer->beginRecording();

            std::vector<vk::Semaphore> demo_semaphores;
            std::vector<vk::PipelineStageFlags> demo_wait_stages;
            auto texture = current_demo_->renderFrame(command_buffer, image_index, frame, &demo_semaphores, &demo_wait_stages);
        
            command_buffer->beginRenderPass(display_render_passes_[image_index]);
            command_buffer->setProgram(post_shader_->program());
            command_buffer->setDefaultState(gfx::CommandBufferState::DefaultState::kOpaque);
            command_buffer->bindTexture(0, 0, texture);
            command_buffer->setDepth(/*test*/ false, /*write*/ false);
            command_buffer->draw(3);
            command_buffer->endRenderPass();
            command_buffer->endRecording();

            demo_semaphores.push_back(image_available_semaphore);
            demo_wait_stages.push_back(vk::PipelineStageFlagBits::eColorAttachmentOutput);
            CXL_DCHECK(demo_semaphores.size() == demo_wait_stages.size());

            // Submit graphics commands.
            vk::PipelineStageFlags graphicsWaitStages[] = {vk::PipelineStageFlagBits::eColorAttachmentOutput};
            vk::SubmitInfo submit_info(/*wait_semaphore_count*/demo_semaphores.size(), 
                                       /*wait_semaphores*/demo_semaphores.data(),
                                       /*wait_stages*/demo_wait_stages.data(),
                                       /*command_buffer_count*/1U,
                                       /*command_buffers*/&command_buffer->vk(), 
                                       /*signal_semaphore_count*/1U, 
                                       /*signal_semaphores*/&render_semaphores_[frame]);
            logical_device_->getQueue(gfx::Queue::Type::kGraphics).submit(submit_info, in_flight_fence);

            return {render_semaphores_[frame]};
         });
    }
}

void DemoHarness::processInputEvents() {
    // Retrieve input events from platform layer.
    std::queue<display::InputEvent> inputEvents = platform_->getInputEvents();

    // Process input events
    while (!inputEvents.empty()) {
        display::InputEvent event = inputEvents.front();
        inputEvents.pop();

        if (event.type == display::InputEventType::KeyPressed && event.key == display::KeyCode::A) {
            index++;
            index %= demos_.size();
            current_demo_ = demos_[index];
        }
    }
}


void DemoHarness::recreateSwapchain(int32_t width, int32_t height) {
    CXL_DCHECK(logical_device_);
    logical_device_->waitIdle();
    if (swap_chain_) {
        for (const auto &semaphore : render_semaphores_) {
            logical_device_->vk().destroy(semaphore);
        }
        for (auto& pass : display_render_passes_) {
            logical_device_->vk().destroyFramebuffer(pass.frame_buffer);
            logical_device_->vk().destroyRenderPass(pass.render_pass);
        }

        render_semaphores_.clear();
        display_render_passes_.clear();
        command_buffers_.clear();
        swap_chain_.reset();
    }
    
    swap_chain_ = std::make_unique<gfx::SwapChain>(logical_device_, surface_, width, height);
    CXL_DCHECK(swap_chain_);

    const auto& swapchain_textures = swap_chain_->textures();
    auto num_swap = swapchain_textures.size();

    command_buffers_ = gfx::CommandBuffer::create(logical_device_, gfx::Queue::Type::kGraphics,
                                                  vk::CommandBufferLevel::ePrimary, num_swap);
    CXL_DCHECK(command_buffers_.size() == num_swap);

    render_semaphores_ = logical_device_->createSemaphores(MAX_FRAMES_IN_FLIGHT);

    gfx::RenderPassBuilder builder(logical_device_);
    for (const auto& texture : swapchain_textures) {
        builder.reset();
        builder.addColorAttachment(texture);
        builder.addSubpass({.bind_point = vk::PipelineBindPoint::eGraphics,
                            .input_indices = {},
                            .color_indices = {0}});
        display_render_passes_.push_back(std::move(builder.build()));
    }

    // Update the demos.
    for (auto demo : demos_) {
        demo->setup(logical_device_, num_swap, width, height); //resize(width, height);
    }
}

void DemoHarness::WindowDelegate::onUpdate() {}

void DemoHarness::WindowDelegate::onResize(int32_t width, int32_t height) {
  if (harness_->logical_device_) {
    harness_->recreateSwapchain(width, height);
  }
}
    

void DemoHarness::WindowDelegate::onWindowMove(int32_t x, int32_t y) {

}
    
void DemoHarness::WindowDelegate::onStart(PlatformNativeWindowHandle window, std::vector<const char*> extensions, int32_t width, int32_t height) {
    harness_->initialize(window, extensions, width, height);
    harness_->render_thread_ = std::thread([this]{
        harness_->current_demo_ = harness_->demos_[0];
        harness_->should_render_ = true;
        harness_->render();
    });
}
    
void DemoHarness::WindowDelegate::onClose() {
    harness_->should_render_ = false;
    harness_->render_thread_.join();
}


DemoHarness::~DemoHarness() {
    logical_device_->waitIdle();

    command_buffers_.clear();
    demos_.clear();
    current_demo_.reset();

    for (auto& semaphore : render_semaphores_) {
        logical_device_->vk().destroy(semaphore);
    }
    render_semaphores_.clear();


    post_shader_.reset();
    swap_chain_.reset();
    instance_->vk().destroySurfaceKHR(surface_);

    for (auto& pass : display_render_passes_) {
        logical_device_->vk().destroyFramebuffer(pass.frame_buffer);
        logical_device_->vk().destroyRenderPass(pass.render_pass);
    }

    display_render_passes_.clear();

    logical_device_.reset();
    physical_device_.reset();
    instance_.reset();
    platform_.reset();
}