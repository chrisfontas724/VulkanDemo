// Copyright 2019 Sic Studios. All rights reserved.
// Use of this source code is governed by our license that can be
// found in the LICENSE file.

#ifndef DALI_ENGINE_INTERFACE_HPP_
#define DALI_ENGINE_INTERFACE_HPP_

#include "streaming/file_system.hpp"
#include "display/window.hpp"

namespace dali {

class Engine {
public:

    enum class API {
        kVulkan,
        kOpenCL,
        kOpenGL,
    };

    virtual ~Engine(){}

    virtual void linkToWindow(display::Window* window) = 0;

    virtual void resizeFramebuffer(uint32_t width, uint32_t height) = 0;

    virtual void render(float delta, bool clear_frame, std::function<void(uint32_t, bool, bool)> func) = 0;

    virtual void cleanup() = 0;


protected:

};
} // dali

#endif // DALI_ENGINE_INTERFACE_HPP_