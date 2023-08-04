// Copyright 2019 Chris Fontas. All rights reserved.
// Use of this source code is governed by the license that can be
// found in the LICENSE file.


#include <UsefulUtils/logging.hpp>
#include "demos/viking_room.hpp"
#include "demos/naive_path_tracer.hpp"
#include "demos/demo_harness.hpp"

INITIALIZE_EASYLOGGINGPP

const uint32_t kDisplayWidth = 1024;
const uint32_t kDisplayHeight = 768;


// Set up a window with the delegate and start polling.
int main(int argc, char** argv) {
    START_EASYLOGGINGPP(argc, argv);

    auto harness = DemoHarness(kDisplayWidth, kDisplayHeight);
    harness.addDemo(std::move(std::make_shared<NaivePathTracer>()));
    harness.addDemo(std::move(std::make_shared<VikingRoom>()));
    return harness.run();
}
