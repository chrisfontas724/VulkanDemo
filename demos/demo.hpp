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
    virtual void setup(gfx::LogicalDevicePtr logical_device, int32_t num_swap, int32_t width, int32_t height) = 0;

    virtual gfx::ComputeTexturePtr renderFrame(gfx::CommandBufferPtr command_buffer, uint32_t image_index, uint32_t frame,             
                                               std::vector<vk::Semaphore>* signal_semaphores = nullptr,
                                               std::vector<vk::PipelineStageFlags>* signal_wait_stages = nullptr) = 0;

    virtual std::string name() = 0;

protected:
    gfx::LogicalDeviceWeakPtr logical_device_;
    uint32_t width_, height_;
};

#endif // DEMO_HPP_
