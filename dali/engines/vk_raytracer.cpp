// Copyright 2019 Sic Studios. All rights reserved.
// Use of this source code is governed by our license that can be
// found in the LICENSE file.

#include "dali/engines/vk_raytracer.hpp"
#include "core/logging/logging.hpp"
#include "dali/graphics/vk_wrappers/compute_pipeline.hpp"
#include "dali/graphics/vk_wrappers/physical_device.hpp"
#include "dali/engine_data/vulkan/vk_engine_data.hpp"
#include "dali/engine_data/vulkan/vk_shape_data.hpp"
#include "dali/engines/types/ray.hpp"
#include "dali/engines/types/intersection.hpp"
#include "dali/model_data/vertex.hpp"
#include "dali/graphics/vk_wrappers/utils/render_pass_utils.hpp"
#include "geometry/util/math_functions.hpp"

namespace dali {

namespace {

    // Create device extension list.
const std::vector<std::string> kDeviceExtensions = {
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
    VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME};

} // namespace


void VKRayTracer::linkToWindow(display::Window* window) {
    if (!window->supports_vulkan()) {
        throw std::runtime_error("This window doesn't have vulkan support!!");
    }

    try {
        createInstance(window->getExtensions());
        vk::SurfaceKHR surface = window->createVKSurface(graphics_instance_->vk());
        setSurface(surface);
    } catch (...) {throw;}
}

void VKRayTracer::createInstance(const std::vector<const char *> &extensions) {
    try {
        // Initialize the graphics API.
        graphics_instance_ = gfx::Instance::create("VulkanInstance", extensions, enable_validation_);
        CXL_VLOG(3) << "Successfully created graphics instance!";

        // Create dispatch queue.
        dispatch_queue_ = std::make_unique<cfx::DispatchQueue>(2);
    } catch (...) {
        std::cout << "VKRayTracer could not instantiate its graphics instance" << std::endl;
        throw;
    }
}

void VKRayTracer::setSurface(const vk::SurfaceKHR& surface) {
    CXL_DCHECK(graphics_instance_);

    // Set surface.
    surface_ = surface;

    // Pick the best device given the provided surface.
    auto physical_device = graphics_instance_->pickBestDevice(surface_, kDeviceExtensions);
    CXL_VLOG(3) << "The best physical device is " << physical_device->name();

    // Make a logical device from the physical device.
    auto logical_device =
        std::make_shared<gfx::LogicalDevice>(physical_device, surface, kDeviceExtensions);
    CXL_VLOG(3) << "Successfully created a logical device!";
}

VKRayTracer::~VKRayTracer() {
}

void VKRayTracer::render(float delta, bool clear_frame, std::function<void(uint32_t, bool, bool)> func) {

}



} // namespace dali