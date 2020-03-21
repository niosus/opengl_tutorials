#include "opengl_tutorials/ui/glfw/viewer.h"

#include <iostream>

namespace gl_tutorials {
namespace glfw {

namespace {
void ErrorCallback(int error, const char* description) {
  std::cerr << "error[" << error << "]:" << description << std::endl;
}
}  // namespace

bool Viewer::Initialize(const WindowSize& window_size,
                        const GlVersion& gl_verions) {
  glfwSetErrorCallback(ErrorCallback);
  if (!glfwInit()) { return false; }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, gl_verions.major);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, gl_verions.minor);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  // TODO(igor): what are these nullptrs?
  window_ = glfwCreateWindow(window_size.width,
                             window_size.height,
                             window_name_.c_str(),
                             nullptr,
                             nullptr);
  if (!window_) { return false; }
  glfwMakeContextCurrent(window_);

  if (!gladLoadGL()) {
    std::cerr << "Failed to initialize GLAD" << std::endl;
    return false;
  }

  glfwSetFramebufferSizeCallback(window_, Viewer::OnResize);
  Resize(window_size);

  initialized_ = true;
  return true;
}

}  // namespace glfw
}  // namespace gl_tutorials
