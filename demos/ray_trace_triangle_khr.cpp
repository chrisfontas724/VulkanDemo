// Copyright 2023 Sic Studios. All rights reserved.
// Use of this source code is governed by our license that can be
// found in the LICENSE file.


#include "ray_trace_triangle_khr.hpp"
#include <VulkanWrappers/acceleration_structure.hpp>
#include <FileStreaming/memory_stream.hpp>

namespace {

const int MAX_FRAMES_IN_FLIGHT = 2;

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
    logical_device_ = logical_device;
    width_ = width;
    height_ = height;

    compute_command_buffers_ = gfx::CommandBuffer::create(logical_device, gfx::Queue::Type::kCompute,
                                                          vk::CommandBufferLevel::ePrimary, num_swap);

    resolve_texture_ = gfx::ImageUtils::createStorageImage(logical_device, width,
                                                           height, vk::SampleCountFlagBits::e1);
    CXL_DCHECK(resolve_texture_);

    compute_semaphores_ = logical_device->createSemaphores(MAX_FRAMES_IN_FLIGHT);


    shader_manager_ = std::make_shared<gfx::RayTracingShaderManager>(logical_device);
    CXL_DCHECK(shader_manager_);

    auto raygen = getModule(logical_device, "raygen.rgen.spv", vk::ShaderStageFlagBits::eRaygenKHR);
    auto closest = getModule(logical_device, "closesthit.rchit.spv", vk::ShaderStageFlagBits::eClosestHitKHR);
    auto miss = getModule(logical_device, "miss.rmiss.spv", vk::ShaderStageFlagBits::eMissKHR);
    CXL_DCHECK(raygen);
    CXL_DCHECK(closest);
    CXL_DCHECK(miss);

    auto raygen_id = shader_manager_->set_raygen_shader(raygen);
    auto closest_id = shader_manager_->add_closest_hit_shader(closest);
    auto miss_id = shader_manager_->add_miss_shader(miss);

    auto hit_group_id = shader_manager_->create_hit_group(/*any*/10000, closest_id, /*intersect*/10000);

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

    sampler_ = gfx::Sampler::create(logical_device);

    auto pipeline_layout = shader_manager_->pipeline_layout();
    CXL_DCHECK(pipeline_layout);

    auto descriptor_set_layout = pipeline_layout->descriptor_set_layout(0);
    CXL_DCHECK(descriptor_set_layout);

    descriptor_set_ = descriptor_set_layout->createDescriptorSet();
    CXL_DCHECK(descriptor_set_);

    descriptor_set_->set_acceleration_structure(/*index*/0, as_->handle());

    vk::DescriptorImageInfo image_info(sampler_->vk(), resolve_texture_->image_view(), resolve_texture_->layout());
    descriptor_set_->set_storage_image(/*index*/1, image_info);
}

void RayTraceTriangleKHR::resize(uint32_t width, uint32_t height) {

}



gfx::ComputeTexturePtr RayTraceTriangleKHR::renderFrame(
                gfx::CommandBufferPtr graphics_command_buffer, 
                uint32_t image_index, 
                uint32_t frame,
                std::vector<vk::Semaphore>* signal_semaphores,
                std::vector<vk::PipelineStageFlags>* signal_wait_stages) {
    auto logical_device = logical_device_.lock();    
    CXL_DCHECK(logical_device);

    auto compute_buffer = compute_command_buffers_[image_index];
    CXL_DCHECK(compute_buffer);


    compute_buffer->reset();
    compute_buffer->beginRecording();

    resolve_texture_->transitionImageLayout(*compute_buffer.get(), vk::ImageLayout::eGeneral);

	vkCmdBindPipeline(compute_buffer->vk(), VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, shader_manager_->pipeline());

   
    const auto& vkDescriptor = descriptor_set_->vk();
    compute_buffer->vk().bindDescriptorSets(vk::PipelineBindPoint::eRayTracingKHR, 
                                             shader_manager_->pipeline_layout()->vk(), 
                                             /*index*/0, 1, &vkDescriptor, 0, nullptr);

    auto raygen_table = shader_manager_->raygen_shader_entry();
    auto miss_table = shader_manager_->miss_shader_entry();
    auto hit_table = shader_manager_->hit_shader_entry();
    VkStridedDeviceAddressRegionKHR callableShaderSbtEntry{};
	
    logical_device->cmdTraceRaysKHR(
				compute_buffer,
				&raygen_table,
				&miss_table,
				&hit_table,
				&callableShaderSbtEntry,
				width_,
				height_,
				/*depth*/1);

    resolve_texture_->transitionImageLayout(*compute_buffer.get(), vk::ImageLayout::eShaderReadOnlyOptimal); 

    compute_buffer->endRecording();
    vk::SubmitInfo submit_info(/*wait_semaphore_count*/0U, 
                               /*wait_semaphores*/nullptr, 
                               /*wait_stages*/{}, 
                               /*command_buffer_count*/1U,
                               /*command_buffers*/&compute_buffer->vk(), 
                               /*signal_semaphore_count*/1U, 
                               /*signal_semaphores*/&compute_semaphores_[frame]);
    logical_device->getQueue(gfx::Queue::Type::kCompute).submit(submit_info, vk::Fence());

    if (signal_semaphores) {
        signal_semaphores->push_back(compute_semaphores_[frame]);
        signal_wait_stages->push_back(vk::PipelineStageFlagBits::eComputeShader);
    }


    return resolve_texture_;
}

RayTraceTriangleKHR::~RayTraceTriangleKHR() {
    as_.reset();
    resolve_texture_.reset();
}
