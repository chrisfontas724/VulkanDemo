


#include "multipass_example.hpp"

MultipassExample::~MultipassExample() {
    // auto logical_device = logical_device_.lock();
    // logical_device->waitIdle();

    // for (auto& pass : render_passes_) {
    //     logical_device->vk().destroyRenderPass(pass.render_pass);
    // }

    // for (auto& texture: first_pass_textures_) {
    //     texture.reset();
    // }
    // for (auto& texture: second_pass_textures_) {
    //     texture.reset();
    // }
    // for (auto& texture: depth_textures_) {
    //     texture.reset();
    // }

    // first_pass_textures_.clear();
    // second_pass_textures_.clear();
    // model_shader_.reset();

}

void MultipassExample::setup(gfx::LogicalDevicePtr logical_device, int32_t num_swap, int32_t width, int32_t height) {
    // logical_device_ = logical_device;
    // num_swap_images_ = num_swap;

    // text_renderer_ = std::make_shared<TextRenderer>(logical_device);

    // cxl::FileSystem fs("c:/Users/Chris/Desktop/Rendering Projects/VulkanDemo/data/shaders/");
    // model_shader_ = christalz::ShaderResource::createGraphics(logical_device, fs, "lighting/model");
    // model_ = std::make_shared<christalz::Model>(logical_device, 
    //     "C:/Users/Chris/Desktop/Rendering Projects/VulkanDemo/data/models/viking_room.obj", 
    //     "C:/Users/Chris/Desktop/Rendering Projects/VulkanDemo/data/textures/viking_room.png");
    // CXL_DCHECK(model_shader_);
    // CXL_DCHECK(model_);

    // resize(width, height);
}

void MultipassExample::resize(uint32_t width, uint32_t height) {
    // auto logical_device = logical_device_.lock();
    // width_ = width;
    // height_ = height;

    // for (auto& pass : render_passes_) {
    //     pass.render_pass.reset();
    //     pass.frame_buffer.reset();
    // }

    // for (auto& texture: first_pass_textures_) {
    //     texture.reset();
    // }
    // for (auto& texture: second_pass_textures_) {
    //     texture.reset();
    // }
    // for (auto& texture: depth_textures_) {
    //     texture.reset();
    // }

    // render_passes_.clear();
    // first_pass_textures_.clear();
    // second_pass_textures_.clear();
    // depth_textures_.clear();


    // gfx::RenderPassBuilder builder(logical_device);


    // first_pass_textures_.resize(num_swap_images_);
    // for (uint32_t i = 0; i < num_swap_images_; i++) {
    //     first_pass_textures_[i] = gfx::ImageUtils::createInputAttachment(logical_device, width, height);
    //     CXL_DCHECK(first_pass_textures_[i]);
    // }
    
    // second_pass_textures_.resize(num_swap_images_);
    // for (uint32_t i = 0; i < num_swap_images_; i++) {
    //     second_pass_textures_[i] = gfx::ImageUtils::createColorAttachment(logical_device, width, height);
    //     CXL_DCHECK(second_pass_textures_[i]);
    // }
    
    // depth_textures_.resize(num_swap_images_);
    // for (uint32_t i = 0; i < num_swap_images_; i++) {
    //   depth_textures_[i] =
    //     gfx::ImageUtils::createDepthTexture(logical_device, width, height);
    //   CXL_DCHECK(depth_textures_[i]);
    // }

    // for (int32_t tex_index = 0; tex_index < num_swap_images_; tex_index++) {
    //     builder.reset();
    //     builder.addColorAttachment(first_pass_textures_[tex_index]);
    //     builder.addColorAttachment(second_pass_textures_[tex_index]);
    //     builder.addDepthAttachment(depth_textures_[tex_index]);

    //     builder.addSubpass({.bind_point = vk::PipelineBindPoint::eGraphics,
    //                         .input_indices = {},
    //                         .color_indices = {0},
    //                         .depth_index = 0});
    

    //     builder.addSubpass({.bind_point = vk::PipelineBindPoint::eGraphics,
    //                         .input_indices = {0},
    //                         .color_indices = {1},
    //                         .depth_index = 0});
    //     render_passes_.push_back(std::move(builder.build()));
    // }
}

gfx::ComputeTexturePtr MultipassExample::renderFrame(
                gfx::CommandBufferPtr command_buffer, 
                uint32_t image_index, 
                uint32_t frame,
                std::vector<vk::Semaphore>* signal_semaphores,
                std::vector<vk::PipelineStageFlags>* signal_wait_stages) {

    // second_pass_textures_[image_index]->transitionImageLayout(*command_buffer.get(), vk::ImageLayout::eShaderReadOnlyOptimal); 
    // return second_pass_textures_[image_index];
}