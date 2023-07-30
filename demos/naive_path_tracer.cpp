// Copyright 2023 Sic Studios. All rights reserved.
// Use of this source code is governed by our license that can be
// found in the LICENSE file.


#include "naive_path_tracer.hpp"

namespace {

const int MAX_FRAMES_IN_FLIGHT = 2;
const int MAX_BOUNCES = 8;

struct Ray {
    glm::vec4 origin;
    glm::vec4 direction;
    glm::vec4 weight;
    glm::ivec2 coord;
};
 
struct HitPoint {
    alignas(16) glm::vec4 pos;
    alignas(16) glm::vec4 norm;
    alignas(16) glm::vec4 tan;
    alignas(16) glm::vec4 col;
    alignas(8)  glm::vec2 uv_coord;
};

struct Material {
    glm::vec4 diffuse_color;
    glm::vec4 emissive_color;
};

} // anonymous namespace

NaivePathTracer::NaivePathTracer(uint32_t width, uint32_t height) 
: Demo("Naive Path Tracer", width, height) {

    const auto& swapchain_textures = swap_chain_->textures();
    auto num_swap = swapchain_textures.size();

    graphics_command_buffers_ = gfx::CommandBuffer::create(logical_device_, gfx::Queue::Type::kGraphics,
                                                  vk::CommandBufferLevel::ePrimary, num_swap);
    CXL_DCHECK(graphics_command_buffers_.size() == num_swap);

    compute_command_buffers_ = gfx::CommandBuffer::create(logical_device_, gfx::Queue::Type::kCompute,
                                                          vk::CommandBufferLevel::ePrimary, num_swap);
    CXL_DCHECK(compute_command_buffers_.size() == num_swap);

    render_semaphores_ = logical_device_->createSemaphores(MAX_FRAMES_IN_FLIGHT);
    compute_semaphores_ = logical_device_->createSemaphores(MAX_FRAMES_IN_FLIGHT);

    text_renderer_ = std::make_shared<TextRenderer>(logical_device_);
    CXL_DCHECK(text_renderer_);

    cxl::FileSystem fs("c:/Users/Chris/Desktop/Rendering Projects/VulkanDemo/data/shaders");
    CXL_LOG(INFO) << "Create camera shader";
    ray_generator_ = christalz::ShaderResource::createCompute(logical_device_, fs, "cameras/pinhole_camera", {fs.directory()});
    CXL_DCHECK(ray_generator_);

    CXL_LOG(INFO) << "Create lighting shader";
    lighter_ = christalz::ShaderResource::createGraphics(logical_device_, fs, "lighting/ray", {fs.directory()});
    CXL_DCHECK(lighter_);

    CXL_LOG(INFO) << "Create intersection shader";
    hit_tester_ = christalz::ShaderResource::createCompute(logical_device_, fs, "raytraversal/intersect", {fs.directory()});
    CXL_DCHECK(hit_tester_);

    CXL_LOG(INFO) << "Create bounce shader";
    bouncer_ = christalz::ShaderResource::createCompute(logical_device_, fs, "raytraversal/bounce", {fs.directory()});
    CXL_DCHECK(bouncer_);

    for (uint32_t i = 0; i < num_swap; i++) {
        rays_.push_back(gfx::ComputeBuffer::createUniformBuffer(logical_device_, sizeof(Ray) * width * height));
        random_seeds_.push_back(gfx::ComputeBuffer::createUniformBuffer(logical_device_, sizeof(float) * width * height));
        hits_.push_back(gfx::ComputeBuffer::createUniformBuffer(logical_device_, sizeof(HitPoint) * width * height));
    }

    camera_ = Camera {
        .position = glm::vec4(278, 273, -800, 1.0),
        .direction = glm::vec4(0,0,1,0),
        .up = glm::vec4(0,1,0,0),
        .focal_length = 0.035,
        .width = 0.025,
        .height = 0.025,
    };


    gfx::RenderPassBuilder builder(logical_device_);
    vk::SampleCountFlagBits samples = vk::SampleCountFlagBits::e4;

    color_textures_.resize(num_swap);
    for (uint32_t i = 0; i < num_swap; i++) {
        color_textures_[i] = gfx::ImageUtils::createColorAttachment(logical_device_, width, height, samples);
        CXL_DCHECK(color_textures_[i]);
    }
    
    resolve_textures_.resize(num_swap);
    for (uint32_t i = 0; i < num_swap; i++) {
        resolve_textures_[i] = gfx::ImageUtils::createColorAttachment(logical_device_, width,
                                                                     height, vk::SampleCountFlagBits::e1);
        CXL_DCHECK(resolve_textures_[i]);
    }

    uint32_t tex_index = 0;
    for (const auto& texture : swapchain_textures) {
        CXL_DCHECK(texture);
        builder.reset();
        builder.addColorAttachment(color_textures_[tex_index]);
        builder.addResolveAttachment(resolve_textures_[tex_index]);

        builder.addSubpass({.bind_point = vk::PipelineBindPoint::eGraphics,
                            .input_indices = {},
                            .color_indices = {0},
                            .resolve_index = 0});
        tex_index++;

        render_passes_.push_back(std::move(builder.build()));
    }


    for (const auto& texture : swapchain_textures) {
        builder.reset();
        builder.addColorAttachment(texture);
        builder.addSubpass({.bind_point = vk::PipelineBindPoint::eGraphics,
                            .input_indices = {},
                            .color_indices = {0}});
        display_render_passes_.push_back(std::move(builder.build()));
    }
}


NaivePathTracer::~NaivePathTracer() {
    logical_device_->waitIdle();
}


int32_t NaivePathTracer::run() {
   uint32_t sample = 1;
    while (!window_->shouldClose()) {
        window_->poll();
        swap_chain_->beginFrame([&](vk::Semaphore& semaphore, vk::Fence& fence, uint32_t image_index,
                                    uint32_t frame) -> std::vector<vk::Semaphore> {

            auto graphics_buffer = graphics_command_buffers_[image_index];
            auto compute_buffer = compute_command_buffers_[image_index];

            // Generate rays.
            compute_buffer->reset();
            compute_buffer->beginRecording();
            compute_buffer->setProgram(ray_generator_->program());
            compute_buffer->bindUniformBuffer(0, 0, rays_[image_index]);
            compute_buffer->bindUniformBuffer(0, 1, random_seeds_[image_index]);
            compute_buffer->pushConstants(camera_);

            // Hit testing.
            for (uint32_t i = 0; i < MAX_BOUNCES; i++) {
                compute_buffer->setProgram(hit_tester_->program());

                if (i < MAX_BOUNCES-1) {
                    compute_buffer->setProgram(bouncer_->program());
                }
            }

            compute_buffer->endRecording();

            // Submit compute commands.



            // // Render to a framebuffer.
            // graphics_buffer->reset();
            // graphics_buffer->beginRecording();
            // graphics_buffer->beginRenderPass(render_passes_[image_index]);
            // graphics_buffer->setProgram(lighter_->program());
            // graphics_buffer->setDefaultState(gfx::CommandBufferState::DefaultState::kCustomRaytrace);
            // graphics_buffer->setDepth(/*test*/ false, /*write*/ false);
            // graphics_buffer->setProgram(lighter_->program());
            // graphics_buffer->bindUniformBuffer(0, 0, rays_[image_index]);
            // graphics_buffer->bindUniformBuffer(0, 1, hits_[image_index]);
            // graphics_buffer->pushConstants(glm::vec2(window_config_.width, window_config_.height));
            // graphics_buffer->draw(window_config_.width * window_config_.height);
            // graphics_buffer->endRenderPass();
            // graphics_buffer->endRecording();

            // Submit graphics commands.
            vk::PipelineStageFlags graphicsWaitStages[] = {
                vk::PipelineStageFlagBits::eColorAttachmentOutput};
            vk::SubmitInfo submit_info(1U, &semaphore, graphicsWaitStages, 1U,
                                       &graphics_buffer->vk(), 1U, &render_semaphores_[frame]);
            logical_device_->getQueue(gfx::Queue::Type::kPresent).submit(submit_info, fence);

            return {render_semaphores_[frame]};
        });        
    }

    return 0;
}