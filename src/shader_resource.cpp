#include <shader_resource.hpp>
#include <shader_compiler.hpp>

namespace christalz {

std::shared_ptr<ShaderResource> ShaderResource::createGraphics(
    const gfx::LogicalDevicePtr& device,
    const cxl::FileSystem& fs,
    const std::string& program_name) {

  cxl::FileStream vert_shader;
  cxl::FileStream frag_shader;

  std::cout << "Program name: " << program_name << std::endl;
  bool result = vert_shader.load(&fs, program_name + ".vert");
  CXL_DCHECK(result) << "Directory: " << fs.directory() << ", " << program_name + ".vert";

  result = frag_shader.load(&fs, program_name + ".frag");
  CXL_DCHECK(result);

  gfx::SpirV vertex_spirv, fragment_spirv;
  gfx::ShaderCompiler compiler;
  compiler.compile(EShLanguage::EShLangVertex, vert_shader.text(), {}, {}, &vertex_spirv);
  compiler.compile(EShLanguage::EShLangFragment, frag_shader.text(), {}, {}, &fragment_spirv);
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
    const std::string& program_name) {
  // TODO.
}

ShaderResource::~ShaderResource() {
    program_.reset();
}


} // christalz