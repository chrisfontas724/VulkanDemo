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
};
 

struct HitPoint {
    alignas(16) glm::vec4 pos;
    alignas(16) glm::vec4 norm;
    alignas(16) glm::vec4 tan;
    alignas(16) glm::vec4 col;
    alignas(8)  glm::vec2 uv_coord;
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
    ray_generator_ = christalz::ShaderResource::createCompute(logical_device_, fs, "cameras/pinhole_camera", {fs.directory()});
    CXL_DCHECK(ray_generator_);

    for (uint32_t i = 0; i < num_swap; i++) {
        rays_.push_back(gfx::ComputeBuffer::createUniformBuffer(logical_device_, sizeof(Ray) * width * height));
        random_seeds_.push_back(gfx::ComputeBuffer::createUniformBuffer(logical_device_, sizeof(float) * width * height));
        hits_.push_back(gfx::ComputeBuffer::createUniformBuffer(logical_device_, sizeof(HitPoint) * width * height));
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

            // Submit compute commands.


            // Render to a framebuffer.
            graphics_buffer->reset();
            graphics_buffer->beginRenderPass(render_passes_[image_index]);
            graphics_buffer->setProgram(lighter_->program());
            graphics_buffer->setDefaultState(gfx::CommandBufferState::DefaultState::kCustomRaytrace);
            graphics_buffer->setDepth(/*test*/ false, /*write*/ false);

            // Position
            graphics_buffer->setVertexAttribute(/*binding*/ 0, /*location*/ 0,
                                                    /*format*/ vk::Format::eR32G32B32A32Sfloat);
            // Normal                                        
            graphics_buffer->setVertexAttribute(/*binding*/ 0, /*location*/ 1,
                                                    /*format*/ vk::Format::eR32G32B32A32Sfloat);
            // Tangent
            graphics_buffer->setVertexAttribute(/*binding*/ 0, /*location*/ 2,
                                                /*format*/ vk::Format::eR32G32B32A32Sfloat);
            // Color
            graphics_buffer->setVertexAttribute(/*binding*/ 0, /*location*/ 2,
                                                /*format*/ vk::Format::eR32G32B32A32Sfloat);
            // UV Coord
            graphics_buffer->setVertexAttribute(/*binding*/ 0, /*location*/ 2,
                                                /*format*/ vk::Format::eR32G32Sfloat);   

            graphics_buffer->bindVertexBuffer(hits_[image_index]);
        });        

    }
    return 0;
}