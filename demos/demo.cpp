// Copyright 2023 Sic Studios. All rights reserved.
// Use of this source code is governed by our license that can be
// found in the LICENSE file.


#include "demo.hpp"
#include "instance.hpp"
#include <iostream>
#include <UsefulUtils/logging.hpp>

namespace {
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

Demo::Demo(const std::string& name, uint32_t width, uint32_t height) {
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

    std::cout << "Seriously...." << std::endl;

    // Make a logical device from the physical device.
    logical_device_ =
        std::make_shared<gfx::LogicalDevice>(physical_device_, surface_, kDeviceExtensions);
    CXL_DCHECK(logical_device_);

    std::cout << "Hmm...." << std::endl;

    int32_t display_width, display_height;
    window_->getSize(&display_width, &display_height);
    std::cout << "meow" << std::endl;
    swap_chain_ = std::make_unique<gfx::SwapChain>(logical_device_, surface_, display_width, display_height);
    CXL_DCHECK(swap_chain_);
    std::cout << "ugh...." << std::endl;
}

Demo::~Demo() {
    logical_device_->waitIdle();

    swap_chain_.reset();
}