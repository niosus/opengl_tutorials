#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "absl/strings/str_format.h"
#include "gl/core/buffer.h"
#include "gl/core/program.h"
#include "gl/core/shader.h"
#include "gl/core/uniform.h"
#include "gl/core/vertex_array_buffer.h"
#include "gl/ui/glfw/viewer.h"
#include "gl/utils/eigen_traits.h"
#include "utils/eigen_utils.h"
#include "utils/image.h"

#include <GLFW/glfw3.h>

#include <iostream>
#include <ostream>
#include <vector>

using utils::Image;

const eigen::vector<float> vertices{
    // positions        // colors         // texture coords
    0.5f,  0.5f,  0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,  // top right
    0.5f,  -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,  // bottom right
    -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,  // bottom left
    -0.5f, 0.5f,  0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f   // top left
};
const std::vector<uint32_t> indices = {0, 1, 3, 1, 2, 3};  // Two triangles.

int main(int argc, char *argv[]) {
  absl::ParseCommandLine(argc, argv);

  gl::glfw::Viewer viewer{"OpenGlViewer"};
  const bool success = viewer.Initialize();
  if (!success) { return EXIT_FAILURE; }

  const auto image =
      Image::CreateFrom("opengl_tutorials/textures/textures/container.jpg");
  if (!image) {
    absl::FPrintF(stderr, "Error: Image not found.\n");
    return EXIT_FAILURE;
  }
  absl::PrintF("Width: %d, Height: %d, Number of channels: %d\n",
               image->width(),
               image->height(),
               image->number_of_channels());

  unsigned int texture;
  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexImage2D(GL_TEXTURE_2D,
               0,
               GL_RGB,
               image->width(),
               image->height(),
               0,
               GL_RGB,
               GL_UNSIGNED_BYTE,
               image->data());
  glGenerateMipmap(GL_TEXTURE_2D);

  const std::shared_ptr<gl::Shader> vertex_shader{gl::Shader::CreateFromFile(
      "opengl_tutorials/textures/shaders/triangle.vert")};
  const std::shared_ptr<gl::Shader> fragment_shader{gl::Shader::CreateFromFile(
      "opengl_tutorials/textures/shaders/triangle.frag")};
  if (!vertex_shader || !fragment_shader) { exit(EXIT_FAILURE); }

  // glUniform1i(glGetUniformLocation(fragment_shader->id(), "texture1"), 0);

  const auto program =
      gl::Program::CreateFromShaders({vertex_shader, fragment_shader});
  if (!program) {
    std::cerr << "Failed to link program" << std::endl;
    return EXIT_FAILURE;
  }

  gl::Uniform uniform{"ourColor", program->id()};

  gl::VertexArrayBuffer vertex_array_buffer{};
  vertex_array_buffer.AssignBuffer(
      std::make_shared<gl::Buffer>(gl::Buffer::Type::kArrayBuffer,
                                   gl::Buffer::Usage::kStaticDraw,
                                   vertices));
  vertex_array_buffer.AssignBuffer(
      std::make_shared<gl::Buffer>(gl::Buffer::Type::kElementArrayBuffer,
                                   gl::Buffer::Usage::kStaticDraw,
                                   indices));
  const int stride = 8;
  const int pos_offset = 0;
  vertex_array_buffer.EnableVertexAttributePointer(0, stride, pos_offset, 3);
  const int color_offset = 3;
  vertex_array_buffer.EnableVertexAttributePointer(1, stride, color_offset, 3);
  const int texture_coords_offset = 6;
  vertex_array_buffer.EnableVertexAttributePointer(
      2, stride, texture_coords_offset, 2);

  while (!viewer.ShouldClose()) {
    viewer.ProcessInput();

    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    program->Use();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);

    vertex_array_buffer.Draw(GL_TRIANGLES);
    viewer.Spin();
  }
  return EXIT_SUCCESS;
}