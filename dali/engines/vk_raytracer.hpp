// Copyright 2019 Sic Studios. All rights reserved.
// Use of this source code is governed by our license that can be
// found in the LICENSE file.

#ifndef DALI_VK_RAY_TRACER_HPP_
#define DALI_VK_RAY_TRACER_HPP_

#include "dali/engines/vk_engine.hpp"
#include "dali/graphics/vk_wrappers/swap_chain.hpp"
#include "dali/graphics/vk_wrappers/queue.hpp"
#include "dali/graphics/vk_wrappers/command_buffer.hpp"
#include "dali/graphics/vk_wrappers/compute_buffer.hpp"
#include "dali/graphics/vk_wrappers/graphics_pipeline.hpp"
#include "dali/graphics/vk_wrappers/descriptor_set.hpp"
#include "dali/engines/ray_tracing/generation/pinhole_generator.hpp"
#include "dali/engines/ray_tracing/traversal/hit_tester.hpp"
#include "dali/engines/ray_tracing/background/background_generator.hpp"
#include "dali/engines/ray_tracing/ordering/ray_reorderer.hpp"
#include "core/threading/dispatch_queue.hpp"
#include "dali/graphics/vk_wrappers/frame_buffer.hpp"
#include "dali/util/screenshotter.hpp"
#include "dali/util/rng_seeder.hpp"
#include "geometry/sampling/halton_sampler.hpp"

namespace dali {
class VKEngineData;
class VKRayTracer : public Engine{
public:

    VKRayTracer(bool enable_validation) {}
    ~VKRayTracer() override;

    void linkToWindow(display::Window* window) override;

    void resizeFramebuffer(uint32_t width, uint32_t height) override {}

    void render(float delta, bool clear_frame, std::function<void(uint32_t, bool, bool)> func) override;

    void cleanup() override;

private:

    void createInstance(const std::vector<const char*>& extensions);
    void setSurface(const vk::SurfaceKHR& surface) override;

};
} // dali

#endif // DALI_VK_RAY_TRACER_HPP_