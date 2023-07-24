// Copyright 2019 Sic Studios. All rights reserved.
// Use of this source code is governed by our license that can be
// found in the LICENSE file.

#ifndef INCLUDE_DEMO_SHADER_RESOURCE_HPP_
#define INCLUDE_DEMO_SHADER_RESOURCE_HPP_

#include <FileStreaming/file_system.hpp>
#include <vk_wrappers/shader_program.hpp>

namespace christalz {

class ShaderResource {
public:

static std::shared_ptr<ShaderResource> createGraphics(
    const gfx::LogicalDevicePtr& device,
    const cxl::FileSystem& fs,
    const std::string& program_name);

static std::shared_ptr<ShaderResource> createCompute(
    const gfx::LogicalDevicePtr& device,
    const cxl::FileSystem& fs,
    const std::string& program_name);

~ShaderResource();

const gfx::ShaderProgramPtr& program() const { return program_; }

private:
 gfx::LogicalDeviceWeakPtr device_;
 gfx::ShaderProgramPtr program_;
};

} // christalz

#endif // INCLUDE_DEMO_SHADER_RESOURCE_HPP_
