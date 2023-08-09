#include <shader_resource.hpp>
#include <VulkanWrappers/shader_compiler.hpp>
#include <FileStreaming/file_stream.hpp>

namespace christalz {

std::shared_ptr<ShaderResource> ShaderResource::createGraphics(
    const gfx::LogicalDevicePtr& device,
    const cxl::FileSystem& fs,
    const std::string& program_name,
    std::vector<std::string> include_paths,
    std::vector<std::string> macros) {

  cxl::FileStream vert_shader;
  cxl::FileStream frag_shader;
  bool result = vert_shader.load(&fs, program_name + ".vert");
  result |= frag_shader.load(&fs, program_name + ".frag");
  CXL_DCHECK(result);

  gfx::SpirV vertex_spirv, fragment_spirv;
  gfx::ShaderCompiler compiler;
  compiler.compile(gfx::ShaderCompiler::Type::eVert, vert_shader.text(), include_paths, macros, &vertex_spirv);
  compiler.compile(gfx::ShaderCompiler::Type::eFrag, frag_shader.text(), include_paths, macros, &fragment_spirv);
  CXL_DCHECK(vertex_spirv.size() > 0);
  CXL_DCHECK(fragment_spirv.size() > 0);

  auto shader_program =
        gfx::ShaderProgram::createGraphics(device, vertex_spirv, fragment_spirv);
  CXL_DCHECK(shader_program);

  auto resource = std::make_shared<ShaderResource>();

  resource->device_ = device;
  resource->program_ = shader_program;
  return resource;
}

std::shared_ptr<ShaderResource> ShaderResource::createCompute(
    const gfx::LogicalDevicePtr& device,
    const cxl::FileSystem& fs,
    const std::string& program_name,
    std::vector<std::string> include_paths,
    std::vector<std::string> macros) {

  cxl::FileStream shader;
  bool result = shader.load(&fs, program_name + ".comp");
  CXL_DCHECK(result) << "Could not load shader " << program_name << ".comp";

  gfx::SpirV spirv;
  gfx::ShaderCompiler compiler;
  compiler.compile(gfx::ShaderCompiler::Type::eComp, shader.text(), include_paths, macros, &spirv);
  CXL_DCHECK(spirv.size() > 0);

  CXL_VLOG(1) << "Starting to create compute shader";
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