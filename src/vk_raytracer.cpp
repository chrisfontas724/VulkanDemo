// Copyright 2019 Sic Studios. All rights reserved.
// Use of this source code is governed by our license that can be
// found in the LICENSE file.

#include <demo/vk_raytracer.hpp>
#include <vk_wrappers/physical_device.hpp>
#include <vk_wrappers/logical_device.hpp>
#include <vk_wrappers/image_utils.hpp>
#include <vk_wrappers/command_buffer.hpp>
#include <windowing/glfw_window.hpp>
#include <vk_wrappers/instance.hpp>
#include <core/logging.hpp>


namespace dali {

namespace {

// Create device extension list.
const std::vector<const char*> kDeviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME,
    VK_KHR_MAINTENANCE3_EXTENSION_NAME,
    VK_NV_RAY_TRACING_EXTENSION_NAME, 
                              
    // KHR Ray Tracing Extensions
    VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME,
    VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME,
    VK_KHR_MAINTENANCE3_EXTENSION_NAME,
    VK_KHR_PIPELINE_LIBRARY_EXTENSION_NAME,
    VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME,
    VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME
};

} // namespace


void VKRayTracer::VKWindowVisitor::visit(display::GLFWWindow* window) {
    CXL_DCHECK(window);
    CXL_DCHECK(window->supports_vulkan());
    engine->createInstance(window->getExtensions());

    const auto& instance = engine->vk_instance_->vk();
    engine->setSurface(window->createVKSurface(instance));
      
    int32_t width, height;
    window->getSize(&width, &height);
    engine->resizeFramebuffer(width, height);
}

void VKRayTracer::linkToWindow(display::Window* window) {
    VKWindowVisitor visitor(this);
    window->accept(&visitor);
}

void VKRayTracer::createInstance(const std::vector<const char *> &extensions) {
    // Initialize the graphics API.
    vk_instance_ = gfx::Instance::create("VulkanInstance", extensions, /*validation*/true);
    CXL_VLOG(3) << "Successfully created graphics instance!";

    // Create dispatch queue.
    //dispatch_queue_ = std::make_unique<cfx::DispatchQueue>(2);
}

void VKRayTracer::setSurface(const vk::SurfaceKHR& surface) {
    CXL_DCHECK(vk_instance_);

    // Set surface.
    surface_ = surface;

    // Pick the best device given the provided surface.
    physical_device_ = vk_instance_->pickBestDevice(surface_, kDeviceExtensions);
    CXL_VLOG(3) << "The best physical device is " << physical_device_->name();

    // Make a logical device from the physical device.
    logical_device_ =
        std::make_shared<gfx::LogicalDevice>(physical_device_, surface_, kDeviceExtensions);
    CXL_VLOG(3) << "Successfully created a logical device!";
}


void VKRayTracer::resizeFramebuffer(uint32_t width, uint32_t height) {
    // Make a new swapchain.
    swap_chain_ = std::make_unique<gfx::SwapChain>(logical_device_, surface_, width, height);
    CXL_VLOG(3) << "Successfully created the swap chain!";

    const auto& swapchain_textures = swap_chain_->textures();
    auto num_swap = swapchain_textures.size();

    auto num_frame_buffers = num_swap;
    CXL_VLOG(3) << "Successfully created a swapchain!";

    vk::SampleCountFlagBits samples = vk::SampleCountFlagBits::e4;

    color_textures_.resize(num_swap);
    for (uint32_t i = 0; i < num_swap; i++) {
        color_textures_[i] = gfx::ImageUtils::createColorAttachment(logical_device_, width,
                                                                   height, samples);
        CXL_DCHECK(color_textures_[i]);
    }


    resolve_textures_.resize(num_swap);
    for (uint32_t i = 0; i < num_swap; i++) {
        resolve_textures_[i] = gfx::ImageUtils::createColorAttachment(logical_device_, width,
                                                                   height, vk::SampleCountFlagBits::e1);
        CXL_DCHECK(resolve_textures_[i]);
    }


    depth_textures_.resize(num_swap);
    for (uint32_t i = 0; i < num_swap; i++) {
      depth_textures_[i] =
        gfx::ImageUtils::createDepthTexture(logical_device_, width, height, samples);
    } 

    // Create command buffers.
    graphics_command_buffers_ =
        gfx::CommandBuffer::create(logical_device_, gfx::Queue::Type::kGraphics,
                                   vk::CommandBufferLevel::ePrimary, num_swap);
}


VKRayTracer::~VKRayTracer() {
}

void VKRayTracer::render(float delta, bool clear_frame, std::function<void(uint32_t, bool, bool)> func) {

}

void VKRayTracer::cleanup() {

}

} // namespace dali