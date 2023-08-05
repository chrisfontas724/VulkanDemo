// Copyright 2023 Sic Studios. All rights reserved.
// Use of this source code is governed by our license that can be
// found in the LICENSE file.


#include "naive_path_tracer.hpp"

namespace {

const int MAX_FRAMES_IN_FLIGHT = 2;
const int MAX_BOUNCES = 15;
uint32_t sample = 1;

} // anonymous namespace

NaivePathTracer::~NaivePathTracer() {
    auto logical_device = logical_device_.lock();
    logical_device->waitIdle();

    for (auto& pass : render_passes_) {
        logical_device->vk().destroyRenderPass(pass.render_pass);
    }

    for (auto& pass : accumulation_passes_) {
        logical_device->vk().destroyRenderPass(pass.render_pass);
    }

    for (auto& semaphore : compute_semaphores_) {
        logical_device->vk().destroy(semaphore);
    }

    for (auto& texture: color_textures_) {
        texture.reset();
    }
    for (auto& texture: resolve_textures_) {
        texture.reset();
    }

    compute_command_buffers_.clear();
}


void NaivePathTracer::setup(gfx::LogicalDevicePtr logical_device, int32_t num_swap, int32_t width, int32_t height) {
    logical_device_ = logical_device;
    width_ = width;
    height_ = height;

    text_renderer_ = std::make_shared<TextRenderer>(logical_device);
    text_renderer_->set_color(glm::vec4(1,1,1,1));

    gfx::RenderPassBuilder builder(logical_device);
    vk::SampleCountFlagBits samples = vk::SampleCountFlagBits::e4;

    accum_textures_.resize(num_swap);
    for (uint32_t i = 0; i < num_swap; i++) {
        accum_textures_[i] = gfx::ImageUtils::createAccumulationAttachment(logical_device, width, height);
        CXL_DCHECK(accum_textures_[i]);
    }

    color_textures_.resize(num_swap);
    for (uint32_t i = 0; i < num_swap; i++) {
        color_textures_[i] = gfx::ImageUtils::createColorAttachment(logical_device, width, height, samples);
        CXL_DCHECK(color_textures_[i]);
    }
    
    resolve_textures_.resize(num_swap);
    for (uint32_t i = 0; i < num_swap; i++) {
        resolve_textures_[i] = gfx::ImageUtils::createColorAttachment(logical_device, width,
                                                                     height, vk::SampleCountFlagBits::e1);
        CXL_DCHECK(resolve_textures_[i]);
    }


    for (int32_t tex_index = 0; tex_index < num_swap; tex_index++) {
        builder.reset();
        builder.addColorAttachment(accum_textures_[tex_index], {
            .load_op = vk::AttachmentLoadOp::eLoad,
            .store_op = vk::AttachmentStoreOp::eStore,
         });
        builder.addSubpass({.bind_point = vk::PipelineBindPoint::eGraphics,
                            .input_indices = {},
                            .color_indices = {0}});
        accumulation_passes_.push_back(std::move(builder.build()));
    }


    for (int32_t tex_index = 0; tex_index < num_swap; tex_index++) {
        builder.reset();
        builder.addColorAttachment(color_textures_[tex_index]);
        builder.addResolveAttachment(resolve_textures_[tex_index]);
        builder.addSubpass({.bind_point = vk::PipelineBindPoint::eGraphics,
                            .input_indices = {},
                            .color_indices = {0},
                            .resolve_index = 0});
        render_passes_.push_back(std::move(builder.build()));
    }

  
    compute_command_buffers_ = gfx::CommandBuffer::create(logical_device, gfx::Queue::Type::kCompute,
                                                          vk::CommandBufferLevel::ePrimary, num_swap);
    CXL_DCHECK(compute_command_buffers_.size() == num_swap);

    compute_semaphores_ = logical_device->createSemaphores(MAX_FRAMES_IN_FLIGHT);

    cxl::FileSystem fs("c:/Users/Chris/Desktop/Rendering Projects/VulkanDemo/data/shaders");
    CXL_LOG(INFO) << "Create rng shader";
    rng_seeder_ = christalz::ShaderResource::createCompute(logical_device, fs, "sampling/rng_seeding", {fs.directory()});
    CXL_DCHECK(rng_seeder_);
 
    CXL_LOG(INFO) << "Create camera shader";
    ray_generator_ = christalz::ShaderResource::createCompute(logical_device, fs, "cameras/pinhole_camera", {fs.directory()});
    CXL_DCHECK(ray_generator_);

    CXL_LOG(INFO) << "Create lighting shader";
    lighter_ = christalz::ShaderResource::createGraphics(logical_device, fs, "lighting/ray", {fs.directory()});
    CXL_DCHECK(lighter_);

    CXL_LOG(INFO) << "Create resolve shader";
    resolve_ = christalz::ShaderResource::createGraphics(logical_device, fs, "lighting/resolve", {fs.directory()});
    CXL_DCHECK(resolve_);

    CXL_LOG(INFO) << "Create intersection shader";
    hit_tester_ = christalz::ShaderResource::createCompute(logical_device, fs, "raytraversal/intersect", {fs.directory()});
    CXL_DCHECK(hit_tester_);

    CXL_LOG(INFO) << "Create bounce shader";
    bouncer_ = christalz::ShaderResource::createCompute(logical_device, fs, "raytraversal/bounce", {fs.directory()});
    CXL_DCHECK(bouncer_);

    std::vector<HitPoint> hits;
    hits.resize(width * height);
    for (uint32_t i = 0; i < num_swap; i++) {
        rays_.push_back(gfx::ComputeBuffer::createStorageBuffer(logical_device, sizeof(Ray) * width * height));
        random_seeds_.push_back(gfx::ComputeBuffer::createStorageBuffer(logical_device, sizeof(float) * width * height));
        hits_.push_back(gfx::ComputeBuffer::createFromVector(logical_device, hits, vk::BufferUsageFlagBits::eStorageBuffer));
    }

    auto compute_buffer = compute_command_buffers_[0];
    compute_buffer->reset();
    compute_buffer->beginRecording();

    for (uint32_t i = 0; i < num_swap; i++) {
        compute_buffer->setProgram(rng_seeder_->program());
        compute_buffer->bindUniformBuffer(0, 0, random_seeds_[i]);
        compute_buffer->pushConstants(glm::ivec2(width_, height_));
        compute_buffer->dispatch(width_ / 16, height_  / 16, 1);
    }
    compute_buffer->endRecording();
    logical_device->getQueue(gfx::Queue::Type::kCompute).submit(compute_buffer);
    logical_device->waitIdle();

    camera_ = Camera {
        .position = glm::vec4(278, 273, -800, 1.0),
        .direction = glm::vec4(0,0,1,0),
        .focal_length = 0.035,
        .width = 0.025,
        .height = 0.025,
        .x_res = width_,
        .y_res = height_
    };

    meshes_ = {
        // Floor - White
        Mesh::createRectangle(logical_device,
                              glm::vec4(552.8, 0.0, 0.0, 1.0),
                              glm::vec4(0, 0, 0, 1.0),
                              glm::vec4(0,0, 559.2, 1.0),
                              glm::vec4(549.6, 0.0, 559.2, 1.0),
                              Material(glm::vec4(0.9, 0.9, 0.9, 1.0))),

        // Left wall - Red
        Mesh::createRectangle(logical_device,
                              glm::vec4(552.8,   0.0,   0.0, 1.0),
                              glm::vec4(549.6,   0.0, 559.2, 1.0),
                              glm::vec4(556.0, 548.8, 559.2, 1.0),
                              glm::vec4(556.0, 548.8,   0.0, 1.0),
                              Material(glm::vec4(0.9,0.05,0.05, 1.0))),

        // Right wall - Green
        Mesh::createRectangle(logical_device,
                              glm::vec4(0.0,  0.0, 559.2, 1.0),
                              glm::vec4(0.0,   0.0,   0.0, 1.0),
                              glm::vec4(0.0, 548.8,   0.0, 1.0),
                              glm::vec4(0.0, 548.8, 559.2, 1.0),
                              Material(glm::vec4(0.05,0.9,0.05, 1.0))),

        // Back wall - White
        Mesh::createRectangle(logical_device,
                              glm::vec4(549.6, 0.0, 559.2, 1.0),
                              glm::vec4(0.0,  0.0, 559.2, 1.0),
                              glm::vec4(0.0, 548.8, 559.2, 1.0),
                              glm::vec4(556.0, 548.8, 559.2, 1.0),
                              Material(glm::vec4(0.9, 0.9, 0.9, 1.0))),

        // Ceiling - White
        Mesh::createRectangle(logical_device,
                              glm::vec4(556.0, 548.8, 0.0, 1.0),
                              glm::vec4(556.0, 548.8, 559.2, 1.0),
                              glm::vec4(0.0, 548.8, 559.2, 1.0),
                              glm::vec4(0.0, 548.8, 0.0, 1.0),
                              Material(glm::vec4(0.9, 0.9, 0.9, 1.0))),

        // Light
        Mesh::createRectangle(logical_device, 
                              glm::vec4(343.0, 548.75, 227.0, 1.0),
                              glm::vec4(343.0, 548.75, 332.0, 1.0),
                              glm::vec4(213.0, 548.75, 332.0, 1.0),
                              glm::vec4(213.0, 548.75, 227.0, 1.0),
                              Material(glm::vec4(0), glm::vec4(50, 50, 50, 1.0))),

        // Tall box - White
        Mesh(logical_device,
             {glm::vec4(423.0, 330.0, 247.0, 1.0),
              glm::vec4(265.0, 330.0, 296.0, 1.0),
              glm::vec4(314.0, 330.0, 456.0, 1.0),
              glm::vec4(472.0, 330.0, 406.0, 1.0),

              glm::vec4(423.0,   0.0, 247.0, 1.0),
              glm::vec4(423.0, 330.0, 247.0, 1.0),
              glm::vec4(472.0, 330.0, 406.0, 1.0),
              glm::vec4(472.0,   0.0, 406.0, 1.0),

              glm::vec4(472.0,   0.0, 406.0, 1.0),
              glm::vec4(472.0, 330.0, 406.0, 1.0),
              glm::vec4(314.0, 330.0, 456.0, 1.0),
              glm::vec4(314.0,   0.0, 456.0, 1.0),

              glm::vec4(314.0,   0.0, 456.0, 1.0),
              glm::vec4(314.0, 330.0, 456.0, 1.0),
              glm::vec4(265.0, 330.0, 296.0, 1.0),
              glm::vec4(265.0,   0.0, 296.0, 1.0),

              glm::vec4(265.0,   0.0, 296.0, 1.0),
              glm::vec4(265.0, 330.0, 296.0, 1.0),
              glm::vec4(423.0, 330.0, 247.0, 1.0),
              glm::vec4(423.0,   0.0, 247.0, 1.0)},

              {0, 1, 2, 0, 2, 3,
               4, 5, 6, 4, 6, 7,
               8, 9, 10, 8, 10, 11,
               12, 13, 14, 12, 14, 15,
               16, 17, 18, 16, 18, 19},

               Material(glm::vec4(0.7))),

        // Short box - White
        Mesh(logical_device,
             {glm::vec4(130.0, 165.0, 65.0, 1.0),
              glm::vec4(82.0, 165.0, 225.0, 1.0),
              glm::vec4(240.0, 165.0, 272.0, 1.0),
              glm::vec4(290.0, 165.0, 114.0, 1.0),

              glm::vec4(290.0, 0.0, 114.0, 1.0),
              glm::vec4(290.0, 165.0, 114.0, 1.0),
              glm::vec4(240.0, 165.0, 272.0, 1.0),
              glm::vec4(240.0,   0.0, 272.0, 1.0),

              glm::vec4(130.0,   0.0,  65.0, 1.0),
              glm::vec4(130.0, 165.0,  65.0, 1.0),
              glm::vec4(290.0, 165.0, 114.0, 1.0),
              glm::vec4(290.0,   0.0, 114.0, 1.0),

              glm::vec4(82.0,   0.0, 225.0, 1.0),
              glm::vec4(82.0, 165.0, 225.0, 1.0),
              glm::vec4(130.0, 165.0,  65.0, 1.0),
              glm::vec4(130.0,   0.0,  65.0, 1.0),

              glm::vec4(240.0,   0.0, 272.0, 1.0),
              glm::vec4(240.0, 165.0, 272.0, 1.0),
              glm::vec4(82.0, 165.0, 225.0, 1.0),
              glm::vec4(82.0,   0.0, 225.0, 1.0)},

              {0, 1, 2, 
               0, 2, 3,
               4, 5, 6, 
               4, 6, 7,
               8, 9, 10, 
               8, 10, 11,
               12, 13, 14, 
               12, 14, 15,
               16, 17, 18, 
               16, 18, 19},

              Material(glm::vec4(0.7)))
    };
}

gfx::ComputeTexturePtr NaivePathTracer::renderFrame(gfx::CommandBufferPtr command_buffer, 
                                                    uint32_t image_index, 
                                                    uint32_t frame) {
    auto logical_device = logical_device_.lock();      
    auto compute_buffer = compute_command_buffers_[image_index];
    compute_buffer->reset();
    compute_buffer->beginRecording();

    // Generate rays.
    compute_buffer->setProgram(ray_generator_->program());
    compute_buffer->bindUniformBuffer(0, 0, rays_[image_index]);
    compute_buffer->bindUniformBuffer(0, 1, random_seeds_[image_index]);
    compute_buffer->pushConstants(camera_);
    compute_buffer->dispatch(width_ / 32, height_ / 32, 1);

    // Hit testing.
    for (uint32_t i = 0; i < MAX_BOUNCES; i++) {
        compute_buffer->setProgram(hit_tester_->program());
        compute_buffer->bindUniformBuffer(0, 0, rays_[image_index]);
        compute_buffer->bindUniformBuffer(0, 1, hits_[image_index]);
        for (uint32_t j = 0; j < meshes_.size(); j++) {
            compute_buffer->bindUniformBuffer(1, 0, meshes_[j].vertices);
            compute_buffer->bindUniformBuffer(1, 1, meshes_[j].triangles);
            compute_buffer->pushConstants(meshes_[j].material);
            compute_buffer->pushConstants(meshes_[j].num_triangles, sizeof(Material));
            compute_buffer->dispatch(width_ * height_ / 512, 1, 1);
        }

        compute_buffer->setProgram(bouncer_->program());
        compute_buffer->bindUniformBuffer(0, 2, random_seeds_[image_index]);
        compute_buffer->dispatch(width_ * height_ / 512, 1, 1);
    }

    compute_buffer->endRecording();
    vk::SubmitInfo submit_info(/*wait_semaphore_count*/0U, 
                               /*wait_semaphores*/nullptr, 
                               /*wait_stages*/{}, 
                               /*command_buffer_count*/1U,
                               /*command_buffers*/&compute_buffer->vk(), 
                               /*signal_semaphore_count*/0u,//1U, 
                              /*signal_semaphores*/nullptr);//&compute_semaphores_[frame]);
    logical_device->getQueue(gfx::Queue::Type::kCompute).submit(submit_info, vk::Fence());
    logical_device->waitIdle();

    // Render to the accumulation buffer.
    accum_textures_[image_index]->transitionImageLayout(*command_buffer.get(), vk::ImageLayout::eColorAttachmentOptimal);
    resolve_textures_[image_index]->transitionImageLayout(*command_buffer.get(), vk::ImageLayout::eColorAttachmentOptimal);
    command_buffer->beginRenderPass(accumulation_passes_[image_index]); 
    command_buffer->setProgram(lighter_->program());
    command_buffer->setDefaultState(gfx::CommandBufferState::DefaultState::kCustomRaytrace);
    command_buffer->setDepth(/*test*/ false, /*write*/ false);
    command_buffer->setProgram(lighter_->program());
    command_buffer->bindUniformBuffer(0, 0, rays_[image_index]);
    command_buffer->draw(width_ * height_);
    command_buffer->endRenderPass();

    // Average out the accumulation buffer.
    accum_textures_[image_index]->transitionImageLayout(*command_buffer.get(), vk::ImageLayout::eShaderReadOnlyOptimal);
    command_buffer->beginRenderPass(render_passes_[image_index]); 
    command_buffer->setProgram(resolve_->program());
    command_buffer->setDefaultState(gfx::CommandBufferState::DefaultState::kOpaque);
    command_buffer->bindTexture(0, 0, accum_textures_[image_index]);
    command_buffer->pushConstants(sample);
    command_buffer->draw(3);

    // Render Debug Text.
    std::string text = "sample: " + std::to_string(sample);
    text_renderer_->renderText(command_buffer, text, {-0.9, 0.8}, {-0.5, 0.9}, text.size());
    sample++;

    command_buffer->endRenderPass();
    resolve_textures_[image_index]->transitionImageLayout(*command_buffer.get(), vk::ImageLayout::eShaderReadOnlyOptimal); 
    return resolve_textures_[image_index];
}