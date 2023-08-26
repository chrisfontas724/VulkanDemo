// Copyright 2023 Sic Studios. All rights reserved.
// Use of this source code is governed by our license that can be
// found in the LICENSE file.


#include "path_tracer_khr.hpp"
#include <VulkanWrappers/acceleration_structure.hpp>


void PathTracerKHR::setup(gfx::LogicalDevicePtr logical_device, int32_t num_swap, int32_t width, int32_t height) {

    std::vector<float> positions = {552.8,   0.0,   0.0, 1.0,
                                    549.6,   0.0, 559.2, 1.0,
                                    556.0, 548.8, 559.2, 1.0,
                                    556.0, 548.8,   0.0, 1.0};
    std::vector<uint32_t> indices = {0,1,2,0,2,3};                                

    gfx::Geometry geometry; 
    geometry.attributes[gfx::VTX_POS_FLAG] = gfx::ComputeBuffer::createFromVector(logical_device, positions, vk::BufferUsageFlagBits::eShaderDeviceAddress);
    geometry.indices = gfx::ComputeBuffer::createFromVector(logical_device, indices,  vk::BufferUsageFlagBits::eShaderDeviceAddress);
    geometry.flags = gfx::VTX_POS_FLAG;
    geometry.num_indices = 6;
    geometry.num_vertices = 4;
    auto as = std::make_shared<gfx::AccelerationStructure>(logical_device, geometry);
    CXL_DCHECK(as);
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