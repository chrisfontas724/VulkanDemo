// Copyright 2019 Sic Studios. All rights reserved.
// Use of this source code is governed by our license that can be
// found in the LICENSE file.

#ifndef DALI_VK_RAY_TRACER_HPP_
#define DALI_VK_RAY_TRACER_HPP_

#include "engine_interface.hpp"
#include <vk_wrappers/swap_chain.hpp>
#include <vk_wrappers/render_pass.hpp>

#include <windowing/glfw_window.hpp>
#include <windowing/window_visitor.hpp>


namespace dali {
class VKRayTracer : public Engine {
public:

    VKRayTracer(bool enable_validation) {}
    ~VKRayTracer() override;

    void linkToWindow(display::Window* window) override;

    void resizeFramebuffer(uint32_t width, uint32_t height) override;

    void render(float delta, bool clear_frame, std::function<void(uint32_t, bool, bool)> func) override;

    void createInstance(const std::vector<const char*>& extensions);
    void setSurface(const vk::SurfaceKHR& surface);

    void cleanup() override;

    std::unique_ptr<gfx::SwapChain> swap_chain_;

    std::shared_ptr<gfx::LogicalDevice> logical_device_;
    std::shared_ptr<gfx::PhysicalDevice> physical_device_;
    vk::SurfaceKHR surface_;

private:
    struct VKWindowVisitor : public display::WindowVisitor {
        VKWindowVisitor(VKRayTracer* in_engine)
        : engine(in_engine) {}

        void visit(display::GLFWWindow* window) override;

        VKRayTracer* engine;
    };
    friend VKWindowVisitor;

    std::unique_ptr<gfx::Instance> vk_instance_;
    std::vector<gfx::ComputeTexturePtr> color_textures_;
    std::vector<gfx::ComputeTexturePtr> resolve_textures_;
    std::vector<gfx::ComputeTexturePtr> depth_textures_;
    std::vector<gfx::CommandBufferPtr> graphics_command_buffers_;

    std::vector<gfx::RenderPassInfo> model_render_passes_;
    std::vector<gfx::RenderPassInfo> display_render_passes_;

};
} // dali

#endif // DALI_VK_RAY_TRACER_HPP_