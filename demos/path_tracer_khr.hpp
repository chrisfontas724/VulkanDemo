// Copyright 2023 Sic Studios. All rights reserved.
// Use of this source code is governed by our license that can be
// found in the LICENSE file.


#ifndef PATH_TRACER_KHR_HPP_
#define PATH_TRACER_KHR_HPP_

#include <string>
#include "demo.hpp"
#include "src/text_renderer.hpp"
#include "src/shader_resource.hpp"
#include "src/model.hpp"
#include <UsefulUtils/dispatch_queue.hpp>
#include <VulkanWrappers/acceleration_structure.hpp>

class PathTracerKHR : public Demo {
public:

    void setup(gfx::LogicalDevicePtr logical_device, int32_t num_swap, int32_t width, int32_t height) override;

    void resize(uint32_t width, uint32_t height) override;

    gfx::ComputeTexturePtr
    renderFrame(gfx::CommandBufferPtr command_buffer, 
                uint32_t image_index, 
                uint32_t frame,
                std::vector<vk::Semaphore>* signal_semaphores = nullptr,
                std::vector<vk::PipelineStageFlags>* signal_wait_stages = nullptr) override;
    
    std::string name() override { return "PathTracerKHR"; }

private:

    std::shared_ptr<gfx::AccelerationStructure> as_;
};

#endif // PATH_TRACER_KHR_HPP_