// Copyright 2023 Sic Studios. All rights reserved.
// Use of this source code is governed by our license that can be
// found in the LICENSE file.


#include "demo_harness.hpp"
#include "instance.hpp"
#include <iostream>
#include "command_buffer.hpp"
#include <UsefulUtils/logging.hpp>

namespace {

uint32_t index = 0;

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

const std::vector<const char*> kDeviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};


} // anonymous namespace


DemoHarness::DemoHarness(uint32_t width, uint32_t height) {
    std::string name = "DemoHarness";
    window_config_.name = name;
    window_config_.width = width;
    window_config_.height = height;
    delegate_ = std::make_shared<Delegate>();
    window_ = std::make_shared<display::GLFWWindow>(window_config_, delegate_); 
    CXL_DCHECK(window_);

    instance_ = gfx::Instance::create(name, window_->getExtensions(), /*validation*/true);   
    CXL_DCHECK(instance_);

    surface_ = window_->createVKSurface(instance_->vk());
    CXL_DCHECK(surface_);

    physical_device_ = instance_->pickBestDevice(surface_, kDeviceExtensions);
    CXL_DCHECK(physical_device_);

    // Make a logical device from the physical device.
    logical_device_ =
        std::make_shared<gfx::LogicalDevice>(physical_device_, surface_, kDeviceExtensions);
    CXL_DCHECK(logical_device_);

    int32_t display_width, display_height;
    window_->getSize(&display_width, &display_height);
    swap_chain_ = std::make_unique<gfx::SwapChain>(logical_device_, surface_, display_width, display_height);
    CXL_DCHECK(swap_chain_);

    const auto& swapchain_textures = swap_chain_->textures();
    auto num_swap = swapchain_textures.size();

    command_buffers_ = gfx::CommandBuffer::create(logical_device_, gfx::Queue::Type::kGraphics,
                                                  vk::CommandBufferLevel::ePrimary, num_swap);
    CXL_DCHECK(command_buffers_.size() == num_swap);

    gfx::RenderPassBuilder builder(logical_device_);
    for (const auto& texture : swapchain_textures) {
        builder.reset();
        builder.addColorAttachment(texture);
        builder.addSubpass({.bind_point = vk::PipelineBindPoint::eGraphics,
                            .input_indices = {},
                            .color_indices = {0}});
        display_render_passes_.push_back(std::move(builder.build()));
    }

    cxl::FileSystem fs("c:/Users/Chris/Desktop/Rendering Projects/VulkanDemo/data/shaders/");
    post_shader_ = christalz::ShaderResource::createGraphics(logical_device_, fs, "posteffects/post");
    CXL_DCHECK(post_shader_);
}


int32_t DemoHarness::run() {
    auto demo = current_demo();
    if (!demo) {
        return 0;
    }

    while (!window_->shouldClose()) {
        window_->poll();

        swap_chain_->beginFrame([&](vk::Semaphore& semaphore, vk::Fence& fence, uint32_t image_index,
                                    uint32_t frame) -> std::vector<vk::Semaphore> {
      
            auto command_buffer = command_buffers_[image_index];
            auto [semaphores, texture] = demo->renderFrame(command_buffer, image_index, frame);
        
            command_buffer->beginRenderPass(display_render_passes_[image_index]);
            command_buffer->setProgram(post_shader_->program());
            command_buffer->setDefaultState(gfx::CommandBufferState::DefaultState::kOpaque);
            command_buffer->bindTexture(0, 0, texture);
            command_buffer->setDepth(/*test*/ false, /*write*/ false);
            command_buffer->draw(3);
            command_buffer->endRenderPass();
            command_buffer->endRecording();

            // Submit graphics commands.
            vk::PipelineStageFlags graphicsWaitStages[] = {
            vk::PipelineStageFlagBits::eColorAttachmentOutput};
            vk::SubmitInfo submit_info(/*wait_semaphore_count*/1U, 
                                       /*wait_semaphores*/&semaphore, 
                                       graphicsWaitStages, 
                                       /*command_buffer_count*/1U,
                                       /*command_buffers*/&command_buffer->vk(), 
                                       /*signal_semaphore_count*/semaphores.size(), 
                                       /*signal_semaphores*/semaphores.data());
            logical_device_->getQueue(gfx::Queue::Type::kPresent).submit(submit_info, fence);

            return semaphores;
         });
    }
    return 0;
}

std::shared_ptr<Demo> DemoHarness::current_demo() {
    return index < demos_.size() ? demos_[index] : nullptr;
}

DemoHarness::~DemoHarness() {
    logical_device_->waitIdle();
    command_buffers_.clear();

    for (auto demo : demos_) {
        demo.reset();
    }
    demos_.clear();

    for (auto& pass : display_render_passes_) {
        logical_device_->vk().destroyRenderPass(pass.render_pass);
    }

    post_shader_.reset();
    swap_chain_.reset();
}