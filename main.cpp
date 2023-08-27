// Copyright 2019 Chris Fontas. All rights reserved.
// Use of this source code is governed by the license that can be
// found in the LICENSE file.


#include <UsefulUtils/logging.hpp>
#include "demos/multipass_example.hpp"
#include "demos/naive_path_tracer.hpp"
#include "demos/path_tracer_khr.hpp"
#include "demos/viking_room.hpp"
#include "demos/demo_harness.hpp"

#define GLM_ENABLE_EXPERIMENTAL 

// Set up the demos.
int main(int argc, char** argv) {
    START_EASYLOGGINGPP(argc, argv);

    uint32_t x_res = 512;
    uint32_t y_res = 512;
    for (uint32_t i = 0; i < argc; i++) {
        std::string arg(argv[i]);
        if (arg.find("--xres=") != std::string::npos) {
            x_res = std::stoi(arg.substr(strlen("--xres=")));
        }
        if (arg.find("--yres=") != std::string::npos) {
            y_res = std::stoi(arg.substr(strlen("--yres="))); 
        }
    }

    auto harness = DemoHarness(x_res, y_res);
    harness.addDemo(std::move(std::make_shared<NaivePathTracer>()));
 //   harness.addDemo(std::move(std::make_shared<PathTracerKHR>()));
 //   harness.addDemo(std::move(std::make_shared<VikingRoom>()));
    return harness.run();
}
