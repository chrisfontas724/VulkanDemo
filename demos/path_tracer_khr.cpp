// Copyright 2023 Sic Studios. All rights reserved.
// Use of this source code is governed by our license that can be
// found in the LICENSE file.


#include "path_tracer_khr.hpp"
#include <VulkanWrappers/acceleration_structure.hpp>


void PathTracerKHR::setup(gfx::LogicalDevicePtr logical_device, int32_t num_swap, int32_t width, int32_t height) {

    gfx::Geometry geometry; 
    for (uint32_t i = 0; i < gfx::VTX_ATTR_COUNT; i++) {
        geometry.attributes[i] = gfx::ComputeBuffer::createStorageBuffer(logical_device, 
                                                                        sizeof(float)*4*3,
                                                                        vk::BufferUsageFlagBits::eShaderDeviceAddress);
    }
//     geometry.indices = gfx::ComputeBuffer::createStorageBuffer(logical_device, sizeof(uint32_t)*3);
//     geometry.flags = 0xFFFFFFFF;
//     geometry.num_indices = 3;
//     geometry.num_vertices = 3;

//     auto as = std::make_shared<gfx::AccelerationStructure>(logical_device, geometry);
//     CXL_DCHECK(as);
}

void PathTracerKHR::resize(uint32_t width, uint32_t height) {

}

gfx::ComputeTexturePtr
PathTracerKHR::renderFrame(gfx::CommandBufferPtr command_buffer, 
                uint32_t image_index, 
                uint32_t frame,
                std::vector<vk::Semaphore>* signal_semaphores,
                std::vector<vk::PipelineStageFlags>* signal_wait_stages) {
    return nullptr;
}