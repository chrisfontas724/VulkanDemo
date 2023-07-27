// Copyright 2019 Chris Fontas. All rights reserved.
// Use of this source code is governed by the license that can be
// found in the LICENSE file.


#include <UsefulUtils/logging.hpp>
#include "demos/viking_room.hpp"

INITIALIZE_EASYLOGGINGPP

const uint32_t kDisplayWidth = 1800;
const uint32_t kDisplayHeight = 1100;

// Set up a window with the delegate and start polling.
int main(int argc, char** argv) {
    START_EASYLOGGINGPP(argc, argv);

    VikingRoom viking_room(kDisplayWidth, kDisplayHeight);
    viking_room.run();
    
    return 0;
}
