// Copyright 2023 Sic Studios. All rights reserved.
// Use of this source code is governed by our license that can be
// found in the LICENSE file.

#ifndef DEMO_HARNESS_HPP_
#define DEMO_HARNESS_HPP_

#include "demo.hpp"
#include "src/shader_resource.hpp"

#include <VulkanWrappers/logical_device.hpp>
#include <VulkanWrappers/render_pass.hpp>
#include <VulkanWrappers/swap_chain.hpp>

#include <Windowing/glfw_window.hpp>


class DemoHarness {
public:

    DemoHarness(uint32_t width, uint32_t height);
    ~DemoHarness();

    void addDemo(std::shared_ptr<Demo> demo) {
        demo->setup(logical_device_, swap_chain_->textures().size(), window_config_.width, window_config_.height);
        demos_.push_back(demo);
        if (demos_.size() == 1) {
            current_demo_ = demo;
            window_->set_title(current_demo_->name());
        }
    }

    int32_t run();

private:

    class WindowDelegate : public display::WindowDelegate{
    public:
        WindowDelegate(DemoHarness* harness):
        harness_(harness){}
        void onUpdate() override;
        void onResize(int32_t width, int32_t height) override;
        void onWindowMove(int32_t x, int32_t y) override;
        void onStart(display::Window*) override;
        void onClose() override;
     private:
        DemoHarness* harness_;
    };

    void recreateSwapchain(int32_t width, int32_t height);
    void checkInputManager(const display::InputManager* mngr);

    std::shared_ptr<WindowDelegate> window_delegate_ = nullptr;
    std::vector<std::shared_ptr<Demo>> demos_;
    gfx::InstancePtr instance_ = nullptr;
    gfx::PhysicalDevicePtr physical_device_ = nullptr;
    gfx::LogicalDevicePtr logical_device_ = nullptr;
    gfx::SwapChainPtr swap_chain_ = nullptr;
    std::shared_ptr<christalz::ShaderResource> post_shader_ = nullptr;

    display::Window::Config window_config_;
    std::shared_ptr<display::GLFWWindow> window_ = nullptr;
    std::shared_ptr<display::WindowDelegate> delegate_ = nullptr;
    vk::SurfaceKHR surface_;

    std::vector<gfx::CommandBufferPtr> command_buffers_;
    std::vector<gfx::RenderPassInfo> display_render_passes_;
    std::vector<vk::Semaphore> render_semaphores_;
    std::shared_ptr<Demo> current_demo_ = nullptr;
};

#endif // DEMO_HARNESS_HPP_

