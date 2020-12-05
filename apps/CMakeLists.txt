# Copyright 2019 Chris Fontas. All rights reserved.
# Use of this source code is governed by the license that can be
# found in the LICENSE file.

add_executable(demo_app main.cpp)
target_compile_features(demo_app PRIVATE cxx_std_17)

target_link_libraries(demo_app PRIVATE demo)
