// Copyright 2019 Chris Fontas. All rights reserved.
// Use of this source code is governed by the license that can be
// found in the LICENSE file.

#include "display/window.hpp"
#include "stdio.h"
#include <iostream>

// Example dummy delegate class.
class Delegate : public display::WindowDelegate {
public:
    
    // |WindowDelegate|
    void onUpdate() override { std::cout << "onUpdate" << std::endl; }
    
    void onResize(int32_t width, int32_t height) override {
        std::cout << "onResize" << std::endl;
    }
    
    void onWindowMove(int32_t x, int32_t y) override {
        std::cout << "onWindowMove" << std::endl;
    }
    
    void onStart(display::Window*) override {
        std::cout << "onStart" << std::endl;
    }
    
    void onClose() override {
        std::cout << "onClose" << std::endl;
    }
};

// Example InputManager checks.
void checkInput(const display::InputManager* input) {
    if (input->key(display::KeyCode::A)) {
        std::cout << "Pressed 'A'" << std::endl;
    } else if (input->key(display::KeyCode::B)) {
        std::cout << "Pressed 'B'" << std::endl;
    } else if (input->key(display::KeyCode::C)) {
        std::cout << "Pressed 'C'" << std::endl;
    }
    // etc...
}

// Set up a window with the delegate and start polling.
int main(int arg, char** argv) {
    
    display::Window::Config config;
    config.name = "Window Example";
    config.width = 1024;
    config.height = 768;
    config.type =  display::Window::Type::kGLFW;
    auto delegate = std::make_shared<Delegate>();
    auto window = display::Window::create(config, std::move(delegate));
    std::cout << "Created window!" << std::endl;
    
    if (!window->supports_vulkan()) {
        std::cerr << "Window doesn't support vulkan.";
        return 1;
    }
    
    std::cout << "Begin loop!" << std::endl;
    while (!window->shouldClose()) {
        window->poll();
        checkInput(window->input_manager());
    }

    return 0;
}

