// Copyright 2023 Sic Studios. All rights reserved.
// Use of this source code is governed by our license that can be
// found in the LICENSE file.


#include "ray_trace_triangle_khr.hpp"
#include <VulkanWrappers/acceleration_structure.hpp>
#include <FileStreaming/memory_stream.hpp>

namespace {

std::shared_ptr<gfx::ShaderModule> getModule(gfx::LogicalDevicePtr device, 
                                             const std::string& program_name,
                                             vk::ShaderStageFlagBits stage) {
  cxl::FileSystem fs(cxl::FileSystem::currentExecutablePath() + "/resources/spirv");
  cxl::MemoryStream stream;
  bool result = stream.load(&fs, program_name);
  CXL_DCHECK(result) << "Couldn't load " << program_name;;

  auto data = stream.data<uint32_t>();

  gfx::SpirV spirv(data, data + stream.size<uint32_t>());

  return std::make_shared<gfx::ShaderModule>(device, stage, spirv);
}

} // anonymous namespace

void RayTraceTriangleKHR::setup(gfx::LogicalDevicePtr logical_device, int32_t num_swap, int32_t width, int32_t height) {
    CXL_DCHECK(logical_device);

    resolve_texture_ = gfx::ImageUtils::createColorAttachment(logical_device, width,
                                                              height, vk::SampleCountFlagBits::e1);
    CXL_DCHECK(resolve_texture_);

    shader_manager_ = std::make_shared<gfx::RayTracingShaderManager>(logical_device);
    CXL_DCHECK(shader_manager_);

    auto raygen = getModule(logical_device, "raygen.rgen.spv", vk::ShaderStageFlagBits::eRaygenKHR);
    auto closest = getModule(logical_device, "closesthit.rchit.spv", vk::ShaderStageFlagBits::eClosestHitKHR);
    auto miss = getModule(logical_device, "miss.rmiss.spv", vk::ShaderStageFlagBits::eMissKHR);

    auto raygen_id = shader_manager_->set_raygen_shader(raygen);
    auto closest_id = shader_manager_->add_closest_hit_shader(closest);
    auto miss_id = shader_manager_->add_miss_shader(miss);

    auto hit_group_id = shader_manager_->create_hit_group(/*any*/0, closest_id, /*intersect*/0);

    shader_manager_->build();

    std::vector<uint32_t> indices = {0,1,2};     
	std::vector<float> positions = {
			  1.0f,  1.0f, 0.0f ,
			 -1.0f,  1.0f, 0.0f ,
			  0.0f, -1.0f, 0.0f
	};

    gfx::Geometry geometry; 
    vk::BufferUsageFlags flags = vk::BufferUsageFlagBits::eShaderDeviceAddress | vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR;
    geometry.attributes[gfx::VTX_POS] = gfx::ComputeBuffer::createFromVector(logical_device, positions, flags);
    geometry.indices = gfx::ComputeBuffer::createFromVector(logical_device, indices, flags);
    geometry.flags = gfx::VTX_POS_FLAG;
    geometry.num_indices = indices.size();
    geometry.num_vertices = positions.size();
    geometry.identifier = 5;
    as_ = std::make_shared<gfx::AccelerationStructure>(logical_device);
    as_->buildTopLevel({geometry.identifier}, {geometry});
    CXL_DCHECK(as_);
}

void RayTraceTriangleKHR::resize(uint32_t width, uint32_t height) {

}

gfx::ComputeTexturePtr
RayTraceTriangleKHR::renderFrame(gfx::CommandBufferPtr command_buffer, 
                uint32_t image_index, 
                uint32_t frame,
                std::vector<vk::Semaphore>* signal_semaphores,
                std::vector<vk::PipelineStageFlags>* signal_wait_stages) {
    
    resolve_texture_->transitionImageLayout(*command_buffer.get(), vk::ImageLayout::eShaderReadOnlyOptimal); 
    return resolve_texture_;
}