# Copyright 2023 Chris Fontas. All rights reserved.
# Use of this source code is governed by the license that can be
# found in the LICENSE file.

set(SOURCE
   ${SOURCE}
   ${CMAKE_CURRENT_SOURCE_DIR}/model.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/shader_resource.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/text_renderer.cpp
   PARENT_SCOPE
)
set(HEADERS
   ${HEADERS}
   ${CMAKE_CURRENT_SOURCE_DIR}/model.hpp
   ${CMAKE_CURRENT_SOURCE_DIR}/shader_resource.hpp
   ${CMAKE_CURRENT_SOURCE_DIR}/text_renderer.hpp
   ${CMAKE_CURRENT_SOURCE_DIR}/tiny_obj_loader.h
   PARENT_SCOPE
)
