// Copyright 2023 Sic Studios. All rights reserved.
// Use of this source code is governed by our license that can be
// found in the LICENSE file.


#include "naive_path_tracer.hpp"

namespace {

const int MAX_FRAMES_IN_FLIGHT = 2;
const int MAX_BOUNCES = 8;
uint32_t sample = 1;

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

NaivePathTracer::~NaivePathTracer() {
    auto logical_device = logical_device_.lock();
    logical_device->waitIdle();

    for (auto& pass : render_passes_) {
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
    text_renderer_->set_color(glm::vec4(1,0,0,1));

    gfx::RenderPassBuilder builder(logical_device);
    vk::SampleCountFlagBits samples = vk::SampleCountFlagBits::e4;

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
        builder.addColorAttachment(color_textures_[tex_index]);
        builder.addResolveAttachment(resolve_textures_[tex_index]);

        builder.addSubpass({.bind_point = vk::PipelineBindPoint::eGraphics,
                            .input_indices = {},
                            .color_indices = {0},
                            .resolve_index = 0});
        render_passes_.push_back(std::move(builder.build()));
    }

  
    // logical_device_ = logical_device;
    // width_ = width;
    // height_ = height;
    
    // // compute_command_buffers_ = gfx::CommandBuffer::create(logical_device, gfx::Queue::Type::kCompute,
    // //                                                       vk::CommandBufferLevel::ePrimary, num_swap);
    // // CXL_DCHECK(compute_command_buffers_.size() == num_swap);

    // render_semaphores_ = logical_device->createSemaphores(MAX_FRAMES_IN_FLIGHT);
    // // compute_semaphores_ = logical_device->createSemaphores(MAX_FRAMES_IN_FLIGHT);

    // text_renderer_ = std::make_shared<TextRenderer>(logical_device);
    // CXL_DCHECK(text_renderer_);

    // cxl::FileSystem fs("c:/Users/Chris/Desktop/Rendering Projects/VulkanDemo/data/shaders");
    // CXL_LOG(INFO) << "Create camera shader";
    // ray_generator_ = christalz::ShaderResource::createCompute(logical_device, fs, "cameras/pinhole_camera", {fs.directory()});
    // CXL_DCHECK(ray_generator_);

    // CXL_LOG(INFO) << "Create lighting shader";
    // lighter_ = christalz::ShaderResource::createGraphics(logical_device, fs, "lighting/ray", {fs.directory()});
    // CXL_DCHECK(lighter_);

    // CXL_LOG(INFO) << "Create intersection shader";
    // hit_tester_ = christalz::ShaderResource::createCompute(logical_device, fs, "raytraversal/intersect", {fs.directory()});
    // CXL_DCHECK(hit_tester_);

    // CXL_LOG(INFO) << "Create bounce shader";
    // bouncer_ = christalz::ShaderResource::createCompute(logical_device, fs, "raytraversal/bounce", {fs.directory()});
    // CXL_DCHECK(bouncer_);

    // for (uint32_t i = 0; i < num_swap; i++) {
    //     rays_.push_back(gfx::ComputeBuffer::createUniformBuffer(logical_device, sizeof(Ray) * width * height));
    //     random_seeds_.push_back(gfx::ComputeBuffer::createUniformBuffer(logical_device, sizeof(float) * width * height));
    //     hits_.push_back(gfx::ComputeBuffer::createUniformBuffer(logical_device, sizeof(HitPoint) * width * height));
    // }

    // camera_ = Camera {
    //     .position = glm::vec4(278, 273, -800, 1.0),
    //     .direction = glm::vec4(0,0,1,0),
    //     .up = glm::vec4(0,1,0,0),
    //     .focal_length = 0.035,
    //     .width = 0.025,
    //     .height = 0.025,
    // };


    // gfx::RenderPassBuilder builder(logical_device);
    // vk::SampleCountFlagBits samples = vk::SampleCountFlagBits::e4;

    // color_textures_.resize(num_swap);
    // for (uint32_t i = 0; i < num_swap; i++) {
    //     color_textures_[i] = gfx::ImageUtils::createColorAttachment(logical_device, width, height, samples);
    //     CXL_DCHECK(color_textures_[i]);
    // }
    
    // resolve_textures_.resize(num_swap);
    // for (uint32_t i = 0; i < num_swap; i++) {
    //     resolve_textures_[i] = gfx::ImageUtils::createColorAttachment(logical_device, width,
    //                                                                  height, vk::SampleCountFlagBits::e1);
    //     CXL_DCHECK(resolve_textures_[i]);
    // }

    // for (uint32_t tex_index = 0; tex_index < num_swap; tex_index++) {
    //     builder.reset();
    //     builder.addColorAttachment(color_textures_[tex_index]);
    //     builder.addResolveAttachment(resolve_textures_[tex_index]);

    //     builder.addSubpass({.bind_point = vk::PipelineBindPoint::eGraphics,
    //                         .input_indices = {},
    //                         .color_indices = {0},
    //                         .resolve_index = 0});
    //     tex_index++;

    //     render_passes_.push_back(std::move(builder.build()));
    // }
}

gfx::ComputeTexturePtr NaivePathTracer::renderFrame(gfx::CommandBufferPtr command_buffer, 
                                                    uint32_t image_index, 
                                                    uint32_t frame) {
            
    // Record graphics commands.
    resolve_textures_[image_index]->transitionImageLayout(*command_buffer.get(), vk::ImageLayout::eColorAttachmentOptimal);
    command_buffer->beginRenderPass(render_passes_[image_index]);

    // {       
    //     command_buffer->setVertexAttribute(/*binding*/ 0, /*location*/ 0,
    //                                         /*format*/ vk::Format::eR32G32B32A32Sfloat);
    //     command_buffer->setVertexAttribute(/*binding*/ 0, /*location*/ 1,
    //                                         /*format*/ vk::Format::eR32G32B32A32Sfloat);
    //     command_buffer->setVertexAttribute(/*binding*/ 0, /*location*/ 2,
    //                                         /*format*/ vk::Format::eR32G32Sfloat);                              
    //     command_buffer->setProgram(model_shader_->program());
    //     command_buffer->bindVertexBuffer(model_->vertices());
    //     command_buffer->bindIndexBuffer(model_->indices());
    //     command_buffer->bindUniformBuffer(0, 0, ubo_buffer_);
    //     command_buffer->bindTexture(0, 1, model_->texture());
    //     command_buffer->setDefaultState(default_state_);
    //     command_buffer->setDepth(/*test*/ true, /*write*/ true);
    //     command_buffer->drawIndexed(model_->num_indices());
    // }
            
    // Render Debug Text.
    {
        std::string text = "sample: " + std::to_string(sample);
        text_renderer_->renderText(command_buffer, text, {-0.9, 0.8}, {-0.5, 0.9}, text.size());
        sample++;
    }

    command_buffer->endRenderPass();
    resolve_textures_[image_index]->transitionImageLayout(*command_buffer.get(), vk::ImageLayout::eShaderReadOnlyOptimal); 
    return resolve_textures_[image_index];

    // CXL_LOG(INFO) << "Render frame!";
    // // auto compute_buffer = compute_command_buffers_[image_index];

    // // // Generate rays.
    // // compute_buffer->reset();
    // // compute_buffer->beginRecording();
    // // compute_buffer->setProgram(ray_generator_->program());
    // // compute_buffer->bindUniformBuffer(0, 0, rays_[image_index]);
    // // compute_buffer->bindUniformBuffer(0, 1, random_seeds_[image_index]);
    // // compute_buffer->pushConstants(camera_);

    // // // Hit testing.
    // // for (uint32_t i = 0; i < MAX_BOUNCES; i++) {
    // //     compute_buffer->setProgram(hit_tester_->program());

    // //     if (i < MAX_BOUNCES-1) {
    // //         compute_buffer->setProgram(bouncer_->program());
    // //     }
    // // }

    // // compute_buffer->endRecording();

    // // // Submit compute commands.


    // // Record graphics commands.
    // resolve_textures_[image_index]->transitionImageLayout(*command_buffer.get(), vk::ImageLayout::eColorAttachmentOptimal);
    // command_buffer->beginRenderPass(render_passes_[image_index]);
   
    // // command_buffer->setVertexAttribute(/*binding*/ 0, /*location*/ 0,
    // //                                    /*format*/ vk::Format::eR32G32B32A32Sfloat);
    // // graphics_buffer->setProgram(lighter_->program());
    // // graphics_buffer->setDefaultState(gfx::CommandBufferState::DefaultState::kCustomRaytrace);
    // // command_buffer->setDepth(/*test*/ false, /*write*/ false);
    // // graphics_buffer->setProgram(lighter_->program());
    // // graphics_buffer->bindUniformBuffer(0, 0, rays_[image_index]);
    // // graphics_buffer->bindUniformBuffer(0, 1, hits_[image_index]);
    // // graphics_buffer->pushConstants(glm::vec2(window_config_.width, window_config_.height));
    // // graphics_buffer->draw(window_config_.width * window_config_.height);
    // // Render Debug Text.
    // {
    //     std::string text = "sample: " + std::to_string(sample);
    //     text_renderer_->renderText(command_buffer, text, {-0.9, 0.8}, {-0.5, 0.9}, text.size());
    //     sample++;
    // }

    // command_buffer->endRenderPass();
    // resolve_textures_[image_index]->transitionImageLayout(*command_buffer.get(), vk::ImageLayout::eShaderReadOnlyOptimal); 
    // return {{render_semaphores_[frame]}, resolve_textures_[image_index]};
}