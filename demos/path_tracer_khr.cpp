// Copyright 2023 Sic Studios. All rights reserved.
// Use of this source code is governed by our license that can be
// found in the LICENSE file.


#include "path_tracer_khr.hpp"
#include <VulkanWrappers/acceleration_structure.hpp>
#include <FileStreaming/memory_stream.hpp>

namespace {

const int MAX_FRAMES_IN_FLIGHT = 2;
int sample = 1;

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



gfx::Geometry PathTracerKHR::createGeometry(const gfx::LogicalDevicePtr& logical_device, 
                            const std::vector<float>& positions, 
                            const std::vector<uint32_t>& indices,
                            const Material& material) {
    static uint64_t identifier = 1;
    gfx::Geometry geometry; 
    vk::BufferUsageFlags flags = vk::BufferUsageFlagBits::eShaderDeviceAddress | vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR;
    geometry.attributes[gfx::VTX_POS] = gfx::ComputeBuffer::createFromVector(logical_device, positions, flags);
    geometry.indices = gfx::ComputeBuffer::createFromVector(logical_device, indices, flags);
    geometry.flags = gfx::VTX_POS_FLAG;
    geometry.num_indices = 6;
    geometry.num_vertices = 4;
    geometry.identifier = identifier++;

    auto mat_buf = gfx::ComputeBuffer::createHostAccessableBuffer(logical_device, sizeof(Material), vk::BufferUsageFlagBits::eShaderDeviceAddress);
    mat_buf->write(&material, 1);
    materials_map_[geometry.identifier] = mat_buf;
    return geometry;
}

void PathTracerKHR::setup(gfx::LogicalDevicePtr logical_device, int32_t num_swap, int32_t width, int32_t height) {
    CXL_DCHECK(logical_device);
    logical_device_ = logical_device;
    width_ = width;
    height_ = height;

    compute_command_buffers_ = gfx::CommandBuffer::create(logical_device, gfx::Queue::Type::kCompute,
                                                          vk::CommandBufferLevel::ePrimary, num_swap);

    accum_textures_[0] = gfx::ImageUtils::createAccumulationAttachment(logical_device, width, height, vk::ImageUsageFlagBits::eStorage, vk::ImageLayout::eGeneral);
    accum_textures_[1] = gfx::ImageUtils::createAccumulationAttachment(logical_device, width, height, vk::ImageUsageFlagBits::eStorage, vk::ImageLayout::eGeneral);

    resolve_texture_ = gfx::ImageUtils::createStorageImage(logical_device, width,
                                                           height, vk::SampleCountFlagBits::e1);
    CXL_DCHECK(resolve_texture_);

    compute_semaphores_ = logical_device->createSemaphores(MAX_FRAMES_IN_FLIGHT);


    shader_manager_ = std::make_shared<gfx::RayTracingShaderManager>(logical_device);
    CXL_DCHECK(shader_manager_);

    auto raygen = getModule(logical_device, "pathtrace.rgen.spv", vk::ShaderStageFlagBits::eRaygenKHR);
    auto closest = getModule(logical_device, "pathtrace.rchit.spv", vk::ShaderStageFlagBits::eClosestHitKHR);
    auto miss = getModule(logical_device, "pathtrace.rmiss.spv", vk::ShaderStageFlagBits::eMissKHR);
    CXL_DCHECK(raygen);
    CXL_DCHECK(closest);
    CXL_DCHECK(miss);

    auto raygen_id = shader_manager_->set_raygen_shader(raygen);
    auto closest_id = shader_manager_->add_closest_hit_shader(closest);
    auto miss_id = shader_manager_->add_miss_shader(miss);

    auto hit_group_id = shader_manager_->create_hit_group(/*any*/10000, closest_id, /*intersect*/10000);

    shader_manager_->build();

    // Camera
    camera_.sensor_width = 0.025;
    camera_.sensor_height = 0.025;
    camera_.focal_length = 0.035;
    camera_.matrix = glm::translate(glm::mat4(1), glm::vec3(278, 273, -800));

    std::vector<gfx::Geometry> geometries;

    // Light
    geometries.push_back(createGeometry(
                        logical_device, 
                        {343.0, 548.75, 227.0,
                        343.0, 548.75, 332.0,
                        213.0, 548.75, 332.0,
                        213.0, 548.75, 227.0},
                        {0,1,2,0,2,3}, 
                        Material(glm::vec4(0,0,0,0), glm::vec4(50,50,50,1)))); 



    // Back wall.
    geometries.push_back(createGeometry(
                        logical_device,
                        {549.6, 0.0, 559.2,
                        0.0,  0.0, 559.2,
                        0.0, 548.8, 559.2,
                        556.0, 548.8, 559.},
                        {0,1,2,0,2,3}, 
                        Material(glm::vec4(0.9, 0.9, 0.9, 1.0)))); 

    // Left wall
    geometries.push_back(createGeometry(
                        logical_device,
                        {552.8,   0.0,   0.0,
                        549.6,   0.0, 559.2,
                        556.0, 548.8, 559.2,
                        556.0, 548.8,   0.0},
                        {0,1,2,0,2,3}, 
                        Material(glm::vec4(0.9,0.05,0.05, 1.0)))); 

    // Right wall
    geometries.push_back(createGeometry(
                        logical_device,
                        {0.0,  0.0, 559.2,
                        0.0,   0.0,   0.0,
                        0.0, 548.8,   0.0,
                        0.0, 548.8, 559.2},
                        {0,1,2,0,2,3}, 
                        Material(glm::vec4(0.05,0.9,0.05, 1.0)))); 

     // Floor
    geometries.push_back(createGeometry(
                        logical_device,
                        {552.8, 0.0, 0.0,
                        0, 0, 0,
                        0,0, 559.2,
                        549.6, 0.0, 559.2},
                        {0,1,2,0,2,3}, 
                        Material(glm::vec4(0.9, 0.9, 0.9, 1.0)))); 

    


    std::vector<uint32_t> instances = {1, 2, 3, 4, 5};   

    std::vector<ObjDesc> obj_descs;
    for (auto instance : instances) {
        ObjDesc desc;
        desc.materialAddress = materials_map_[instance]->device_address();
        obj_descs.push_back(desc);
    }                         

    obj_descriptions_ = gfx::ComputeBuffer::createFromVector(logical_device, obj_descs, vk::BufferUsageFlagBits::eStorageBuffer);

    as_ = std::make_shared<gfx::AccelerationStructure>(logical_device);
    as_->buildTopLevel(instances, geometries);
    CXL_DCHECK(as_);
}

void PathTracerKHR::resize(uint32_t width, uint32_t height) {
}

gfx::ComputeTexturePtr PathTracerKHR::renderFrame(
                gfx::CommandBufferPtr command_buffer, 
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

    compute_buffer->setProgram(shader_manager_);
    compute_buffer->setRecursiveDepth(1);

    // Set descriptors.
    compute_buffer->bindAccelerationStructure(0,0, as_);
    compute_buffer->bindStorageImage(0, 1, accum_textures_[texture_index]);
    compute_buffer->bindStorageImage(0, 2, accum_textures_[(texture_index + 1) % 2]);
    compute_buffer->bindStorageImage(0, 3, resolve_texture_);
    compute_buffer->bindUniformBuffer(1, 0, obj_descriptions_);
    texture_index = (texture_index + 1) % 2;

    // set push constants.
    compute_buffer->pushConstants(camera_.matrix);
    compute_buffer->pushConstants(camera_.focal_length, 64u);
    compute_buffer->pushConstants(camera_.sensor_width, 68u);
    compute_buffer->pushConstants(camera_.sensor_height, 72u);
    compute_buffer->pushConstants(width_, 76u);
    compute_buffer->pushConstants(height_, 80u);
    compute_buffer->pushConstants(sample, 84u);
    sample++;

    compute_buffer->traceRays(width_, height_);
	
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

PathTracerKHR::~PathTracerKHR() {
    auto logical_device = logical_device_.lock();
    as_.reset();
    shader_manager_.reset();
    accum_textures_[0].reset();
    accum_textures_[1].reset();
    resolve_texture_.reset();

    for (auto& semaphore : compute_semaphores_) {
        logical_device->vk().destroy(semaphore);
    }

    compute_command_buffers_.clear();
}
