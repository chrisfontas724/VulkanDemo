// Copyright 2023 Sic Studios. All rights reserved.
// Use of this source code is governed by our license that can be
// found in the LICENSE file.


#ifndef MULTIPASS_EXAMLE_HPP_
#define MULTIPASS_EXAMLE_HPP_

#include "demo.hpp"

class MultipassExample : public Demo {

public:

    ~MultipassExample();
    void setup(gfx::LogicalDevicePtr logical_device, int32_t num_swap, int32_t width, int32_t height) override;

    gfx::ComputeTexturePtr
    renderFrame(gfx::CommandBufferPtr command_buffer, 
                uint32_t image_index, 
                uint32_t frame,
                std::vector<vk::Semaphore>* signal_semaphores = nullptr,
                std::vector<vk::PipelineStageFlags>* signal_wait_stages = nullptr) override;
    
    std::string name() override { return "Multipass Example"; }

};

#endif // MULTIPASS_EXAMLE_HPP_