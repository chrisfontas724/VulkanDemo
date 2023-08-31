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


void readObjFile(const std::string& filename, std::vector<float>& positions, std::vector<uint32_t>& indices) {
   
    std::string path = cxl::FileSystem::currentExecutablePath() + "/resources/models/" + filename;
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << path << std::endl;
        return;
    }

    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string type;
        iss >> type;

        if (type == "v") {
            float x, y, z;
            iss >> x >> y >> z;
            positions.push_back(x);
            positions.push_back(y);
            positions.push_back(z);
        } else if (type == "f") {
            uint32_t idx1, idx2, idx3;
            iss >> idx1 >> idx2 >> idx3;
            indices.push_back(idx1 - 1); // OBJ indices are 1-based
            indices.push_back(idx2 - 1);
            indices.push_back(idx3 - 1);
        }
    }

    // Do Checks.
    CXL_DCHECK(positions.size() % 3 == 0);
    CXL_DCHECK(indices.size( ) % 3 == 0);
    for (auto ind : indices) {
        CXL_DCHECK(ind < positions.size() / 3) << ind << " " << positions.size() / 3;
    }

    file.close();
}

} // anonymous namespace

static uint64_t identifier = 1;

gfx::Geometry PathTracerKHR::createBBox(const gfx::LogicalDevicePtr& logical_device,
                                          const Material& material) {
    VkAabbPositionsKHR bbox;
    bbox.minX = -1;
    bbox.minY = -1;
    bbox.minZ = -1;
    bbox.maxX = 1;
    bbox.maxY = 1;
    bbox.maxZ = 1;
    gfx::Geometry sphere;
    sphere.identifier = identifier++;
    sphere.type = gfx::GeometryType::eAABB;
	std::vector<VkAabbPositionsKHR> aabbs = {bbox};
	sphere.bbox = gfx::ComputeBuffer::createFromVector(logical_device, aabbs, vk::BufferUsageFlagBits::eShaderDeviceAddress | vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR);
    
    auto mat_buf = gfx::ComputeBuffer::createHostAccessableBuffer(logical_device, sizeof(Material), vk::BufferUsageFlagBits::eShaderDeviceAddress);
    mat_buf->write(&material, 1);
    materials_map_[sphere.identifier] = mat_buf;
    return sphere;
}

gfx::Geometry PathTracerKHR::createGeometry(const gfx::LogicalDevicePtr& logical_device, 
                            const std::vector<float>& positions, 
                            const std::vector<uint32_t>& indices,
                            const Material& material) {
    gfx::Geometry geometry; 
    vk::BufferUsageFlags flags = vk::BufferUsageFlagBits::eShaderDeviceAddress | vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR;
    geometry.attributes[gfx::VTX_POS] = gfx::ComputeBuffer::createFromVector(logical_device, positions, flags);
    geometry.indices = gfx::ComputeBuffer::createFromVector(logical_device, indices, flags);
    geometry.flags = gfx::VTX_POS_FLAG;
    geometry.num_indices = indices.size();
    geometry.num_vertices = positions.size() / 3;
    geometry.identifier = identifier++;

    auto mat_buf = gfx::ComputeBuffer::createHostAccessableBuffer(logical_device, sizeof(Material), vk::BufferUsageFlagBits::eShaderDeviceAddress);
    mat_buf->write(&material, 1);
    materials_map_[geometry.identifier] = mat_buf;
    return geometry;
}

void PathTracerKHR::setup(gfx::LogicalDevicePtr logical_device, int32_t num_swap, int32_t width, int32_t height) {
    CXL_DCHECK(logical_device);
    logical_device_ = logical_device;
    num_swap_images_ = num_swap;
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

    auto sphere_intersect = getModule(logical_device, "sphere.rint.spv", vk::ShaderStageFlagBits::eIntersectionKHR);
    auto sphere_chit = getModule(logical_device, "sphere.rchit.spv", vk::ShaderStageFlagBits::eClosestHitKHR);

    CXL_DCHECK(raygen);
    CXL_DCHECK(closest);
    CXL_DCHECK(miss);

    auto raygen_id = shader_manager_->set_raygen_shader(raygen);
    auto closest_id = shader_manager_->add_closest_hit_shader(closest);
    auto sphere_closest_id = shader_manager_->add_closest_hit_shader(sphere_chit);
    auto sphere_intersect_id = shader_manager_->add_intersection_shader(sphere_intersect);
    auto miss_id = shader_manager_->add_miss_shader(miss);

    auto hit_group_id = shader_manager_->create_hit_group(/*any*/10000, closest_id, /*intersect*/10000);
    auto sphere_hit_group = shader_manager_->create_hit_group(/*any*/10000, sphere_closest_id, sphere_intersect_id);

    shader_manager_->build();

    // Camera
    camera_.sensor_width = 0.025;
    camera_.sensor_height = 0.025;
    camera_.focal_length = 0.035;
    camera_.matrix = glm::translate(glm::mat4(1), glm::vec3(278, 273, -800));

    // Light
    // geometries.push_back(createGeometry(
    //                     logical_device, 
    //                     {343.0, 548.75, 227.0,
    //                     343.0, 548.75, 332.0,
    //                     213.0, 548.75, 332.0,
    //                     213.0, 548.75, 227.0},
    //                     {0,1,2,0,2,3}, 
    //                     Material(glm::vec4(0,0,0,0), glm::vec4(50,50,50,1)))); 


    // Ceiling - White
    geometries.push_back(createGeometry(
                        logical_device,
                        {556.0, 548.8, 0.0,
                        556.0, 548.8, 559.2,
                        0.0, 548.8, 559.2,
                        0.0, 548.8, 0.0},
                        {0,1,2,0,2,3},
                        Material(glm::vec4(0.9, 0.9, 0.9, 1.0))));

    // Back wall.
    geometries.push_back(createGeometry(
                        logical_device,
                        {549.6, 0.0, 559.2,
                        0.0,  0.0, 559.2,
                        0.0, 548.8, 559.2,
                        556.0, 548.8, 559.},
                        {0,1,2,0,2,3}, 
                        Material(glm::vec4(0.9, 0.5, 0.9, 1.0)))); 

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

    std::vector<float> bunny_pos;
    std::vector<uint32_t> bunny_indices;
    readObjFile("lucy_resized.obj", bunny_pos, bunny_indices);
    geometries.push_back(createGeometry(logical_device, bunny_pos, bunny_indices, Material(glm::vec4(0.8))));

    std::vector<gfx::GeomInstance> instances;
    for (uint32_t i = 1; i <= geometries.size(); i++) {
        gfx::GeomInstance instance;
        instance.identifier = i;
        instance.geometryID = i;
        instances.push_back(instance);
    }


    // Duplicate lucy
    gfx::GeomInstance instance;
    instance.identifier = geometries.size();
    instance.geometryID = geometries.size();
    instances.push_back(instance);

 //   Bunny params
 //   glm::vec3 scaleFactors(2000.0f, 2000.f, 2000.f);
 //   glm::vec3 translation(250, -80, 300);
    // Lucy params
    {
        glm::vec3 scaleFactors(210.0f, 210.f, 210.f);
        glm::vec3 translation(410, 0, 250);
        glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), scaleFactors);
        glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), translation);
        glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 finalMatrix = translationMatrix * rotationMatrix * scaleMatrix;
        instances[instances.size()-2].world_transform = finalMatrix;
    }
    {
        glm::vec3 scaleFactors(210.0f, 210.f, 210.f);
        glm::vec3 translation(160, 0, 320);
        glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), scaleFactors);
        glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), translation);
        glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 finalMatrix = translationMatrix * rotationMatrix * scaleMatrix;
        instances[instances.size()-1].world_transform = finalMatrix;
    }


    // Create sphere.
    geometries.push_back(createBBox(logical_device, Material(glm::vec4(0), glm::vec4(50))));
    {
        sphere_.identifier = instance.geometryID;
        sphere_.geometryID = geometries[geometries.size()-1].identifier;
        glm::vec3 scaleFactors(30, 30, 30);
        glm::vec3 translation(265, 510, 230);
        glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), scaleFactors);
        glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), translation);
        glm::mat4 finalMatrix = translationMatrix * scaleMatrix;
        sphere_.world_transform = finalMatrix;
        sphere_.shaderTableOffset = shader_manager_->index_for_hit_group(sphere_hit_group);
        instances.push_back(sphere_);
    }

    std::vector<ObjDesc> obj_descs;
    uint32_t k = 0;
    for (auto& instance : instances) {
        ObjDesc desc;
        desc.materialAddress = materials_map_[instance.geometryID]->device_address();
        auto geometry = geometries[instance.geometryID-1];
        if (geometry.type == gfx::GeometryType::eTriangles) {
            desc.indexAddress = geometries[instance.geometryID-1].indices->device_address();
            desc.vertexAddress = geometries[instance.geometryID-1].attributes[gfx::VTX_POS]->device_address();
        }
        instance.custom_index = k;
        obj_descs.push_back(desc);
        k++;
    }                         

    obj_descriptions_ = gfx::ComputeBuffer::createFromVector(logical_device, obj_descs, vk::BufferUsageFlagBits::eStorageBuffer);

    as_ = std::make_shared<gfx::AccelerationStructure>(logical_device);
    as_->buildTopLevel(instances, geometries);
    CXL_DCHECK(as_);
    CXL_LOG(INFO) << "Made the AS!";

    // Random seeds
    cxl::FileSystem fs(cxl::FileSystem::currentExecutablePath() + "/resources/spirv");
    mwc64x_seeder_ = christalz::ShaderResource::createCompute(logical_device, fs, "mwc64x_seeding");
    CXL_DCHECK(mwc64x_seeder_);
    for (uint32_t i = 0; i < num_swap_images_; i++) {
        random_seeds_.push_back(gfx::ComputeBuffer::createStorageBuffer(logical_device, sizeof(uint32_t) * width_ * height_ * 2));
    }

    auto compute_buffer = compute_command_buffers_[0];
    compute_buffer->reset();
    compute_buffer->beginRecording();

    uint64_t offset = 0;
    for (uint32_t i = 0; i < num_swap_images_; i++) {
        compute_buffer->setProgram(mwc64x_seeder_->program());
        compute_buffer->bindUniformBuffer(0, 0, random_seeds_[i]);
        compute_buffer->pushConstants(offset);
        compute_buffer->dispatch(width_ * height_/ 16, 1, 1);
        offset += width_ * height_;
    }

    compute_buffer->endRecording();
    logical_device->getQueue(gfx::Queue::Type::kCompute).submit(compute_buffer);
    logical_device->waitIdle();
    CXL_LOG(INFO) << "Finished setup!";
}

void PathTracerKHR::resize(uint32_t width, uint32_t height) {
}

void PathTracerKHR::processEvent(display::InputEvent event) {
    if (event.type == display::InputEventType::KeyPressed && event.key == display::KeyCode::I) {
        camera_.matrix = glm::translate(camera_.matrix, glm::vec3(0,0,1));
        clear_image_ = true;
    } else if (event.type == display::InputEventType::KeyPressed && event.key == display::KeyCode::K) {
        camera_.matrix = glm::translate(camera_.matrix, glm::vec3(0,0,-1));
        clear_image_ = true;
    } else if (event.type == display::InputEventType::KeyPressed && event.key == display::KeyCode::J) {
        camera_.matrix = glm::translate(camera_.matrix, glm::vec3(1,0,0));
        clear_image_ = true;
    } else if (event.type == display::InputEventType::KeyPressed && event.key == display::KeyCode::L) {
        camera_.matrix = glm::translate(camera_.matrix, glm::vec3(-1,0,0));
        clear_image_ = true;
    } else if (event.type == display::InputEventType::KeyPressed && event.key == display::KeyCode::O) {
        camera_.matrix = glm::translate(camera_.matrix, glm::vec3(0,1,0));
        clear_image_ = true;
    } else if (event.type == display::InputEventType::KeyPressed && event.key == display::KeyCode::P) {
        camera_.matrix = glm::translate(camera_.matrix, glm::vec3(0,-1,0));
        clear_image_ = true;
    } else if (event.type == display::InputEventType::KeyPressed && event.key == display::KeyCode::Y) {
        // sphere_.world_transform = glm::translate(sphere_.world_transform, glm::vec3(0,1,0));
        // as_->set_matrix(sphere_.identifier, sphere_.world_transform);
        // clear_image_ = true;
    } else if (event.type == display::InputEventType::KeyPressed && event.key == display::KeyCode::H) {
        // sphere_.world_transform = glm::translate(sphere_.world_transform, glm::vec3(0,-1,0));
        // as_->set_matrix(sphere_.identifier, sphere_.world_transform);
        // clear_image_ = true;
    }
};

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

    if (clear_image_) {
        command_buffer->clearColorImage(accum_textures_[0], {0,0,0,0});
        command_buffer->clearColorImage(accum_textures_[1], {0,0,0,0});
        clear_image_ = false;
        sample_ = 1;
    }

    resolve_texture_->transitionImageLayout(*compute_buffer.get(), vk::ImageLayout::eGeneral);

    compute_buffer->setProgram(shader_manager_);
    compute_buffer->setRecursiveDepth(3);

    // Set descriptors.
    compute_buffer->bindAccelerationStructure(0,0, as_);
    compute_buffer->bindStorageImage(1, 1, accum_textures_[texture_index]);
    compute_buffer->bindStorageImage(1, 2, accum_textures_[(texture_index + 1) % 2]);
    compute_buffer->bindStorageImage(1, 3, resolve_texture_);
    compute_buffer->bindUniformBuffer(1, 4, random_seeds_[image_index]);
    compute_buffer->bindUniformBuffer(2, 0, obj_descriptions_);
    texture_index = (texture_index + 1) % 2;

    // set push constants.
    compute_buffer->pushConstants(camera_.matrix);
    compute_buffer->pushConstants(camera_.focal_length, 64u);
    compute_buffer->pushConstants(camera_.sensor_width, 68u);
    compute_buffer->pushConstants(camera_.sensor_height, 72u);
    compute_buffer->pushConstants(width_, 76u);
    compute_buffer->pushConstants(height_, 80u);
    compute_buffer->pushConstants(sample_, 84u);
    sample_++;

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
