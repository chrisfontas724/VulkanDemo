// Copyright 2023 Sic Studios. All rights reserved.
// Use of this source code is governed by our license that can be
// found in the LICENSE file.

#ifndef DEMO_HPP_
#define DEMO_HPP_

#include <string>
#include "logical_device.hpp"
#include "swap_chain.hpp"
#include <Windowing/glfw_window.hpp>

class Demo {

public:
    ~Demo();

    virtual void run() = 0;

protected:
    Demo(const std::string& name, uint32_t width, uint32_t height);

    gfx::InstancePtr instance_;
    gfx::PhysicalDevicePtr physical_device_;
    gfx::LogicalDevicePtr logical_device_;
    gfx::SwapChainPtr swap_chain_;
    display::Window::Config window_config_;
    std::shared_ptr<display::GLFWWindow> window_;
    std::shared_ptr<display::WindowDelegate> delegate_;
    vk::SurfaceKHR surface_;
};

#endif // DEMO_HPP_
