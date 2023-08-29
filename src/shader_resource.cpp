#include "Shader_resource.hpp"
#include <FileStreaming/memory_stream.hpp>
#include <VulkanWrappers/shader_compiler.hpp>

namespace christalz {

std::shared_ptr<ShaderResource> ShaderResource::createGraphics(
    const gfx::LogicalDevicePtr& device,
    const cxl::FileSystem& fs,
    const std::string& program_name) {

  cxl::MemoryStream vert_stream;
  cxl::MemoryStream frag_stream;
  bool result = vert_stream.load(&fs, program_name + ".vert.spv");
  CXL_DCHECK(result) << "Couldn't load " << program_name  << ".vert.spv";
  result |= frag_stream.load(&fs, program_name + ".frag.spv");
  CXL_DCHECK(result) << "Couldn't load " << program_name << ".frag.spv";

  auto vert_data = vert_stream.data<uint32_t>();
  auto frag_data = frag_stream.data<uint32_t>();

  gfx::SpirV vertex_spirv(vert_data, vert_data + vert_stream.size<uint32_t>());
  gfx::SpirV frag_spirv(frag_data, frag_data + frag_stream.size<uint32_t>());

  auto shader_program =
        gfx::ShaderProgram::createGraphics(device, vertex_spirv, frag_spirv);
  CXL_DCHECK(shader_program);

  auto resource = std::make_shared<ShaderResource>();

  resource->device_ = device;
  resource->program_ = shader_program;
  return resource;
}

std::shared_ptr<ShaderResource> ShaderResource::createCompute(
    const gfx::LogicalDevicePtr& device,
    const cxl::FileSystem& fs,
    const std::string& program_name) {
  cxl::MemoryStream stream;
  bool result = stream.load(&fs, program_name + ".comp.spv");
  CXL_DCHECK(result) << "Couldn't load " << program_name << ".comp.spv";

  auto data = stream.data<uint32_t>();

  gfx::SpirV spirv(data, data + stream.size<uint32_t>());

  auto shader_program =
        gfx::ShaderProgram::createCompute(device, spirv);
  CXL_DCHECK(shader_program);

  auto resource = std::make_shared<ShaderResource>();

  resource->device_ = device;
  resource->program_ = shader_program;
  return resource;

}

ShaderResource::~ShaderResource() {
    program_.reset();
}


} // christalz