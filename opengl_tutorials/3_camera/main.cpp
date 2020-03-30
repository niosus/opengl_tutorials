#include "absl/strings/str_format.h"
#include "gl/core/buffer.h"
#include "gl/core/program.h"
#include "gl/core/shader.h"
#include "gl/core/texture.h"
#include "gl/core/uniform.h"
#include "gl/core/vertex_array_buffer.h"
#include "gl/ui/glfw/viewer.h"
#include "gl/utils/camera.h"
#include "gl/utils/eigen_traits.h"
#include "utils/eigen_utils.h"
#include "utils/image.h"

#include <Eigen/Geometry>
#include <GLFW/glfw3.h>

#include <iostream>
#include <ostream>
#include <vector>

using namespace units::literals;
using utils::Image;

const eigen::vector<Eigen::Vector3f> vertices{
    {-0.5f, -0.5f, -0.5f}, {0.5f, -0.5f, -0.5f}, {0.5f, 0.5f, -0.5f},
    {0.5f, 0.5f, -0.5f},   {-0.5f, 0.5f, -0.5f}, {-0.5f, -0.5f, -0.5f},
    {-0.5f, -0.5f, 0.5f},  {0.5f, -0.5f, 0.5f},  {0.5f, 0.5f, 0.5f},
    {0.5f, 0.5f, 0.5f},    {-0.5f, 0.5f, 0.5f},  {-0.5f, -0.5f, 0.5f},
    {-0.5f, 0.5f, 0.5f},   {-0.5f, 0.5f, -0.5f}, {-0.5f, -0.5f, -0.5f},
    {-0.5f, -0.5f, -0.5f}, {-0.5f, -0.5f, 0.5f}, {-0.5f, 0.5f, 0.5f},
    {0.5f, 0.5f, 0.5f},    {0.5f, 0.5f, -0.5f},  {0.5f, -0.5f, -0.5f},
    {0.5f, -0.5f, -0.5f},  {0.5f, -0.5f, 0.5f},  {0.5f, 0.5f, 0.5f},
    {-0.5f, -0.5f, -0.5f}, {0.5f, -0.5f, -0.5f}, {0.5f, -0.5f, 0.5f},
    {0.5f, -0.5f, 0.5f},   {-0.5f, -0.5f, 0.5f}, {-0.5f, -0.5f, -0.5f},
    {-0.5f, 0.5f, -0.5f},  {0.5f, 0.5f, -0.5f},  {0.5f, 0.5f, 0.5f},
    {0.5f, 0.5f, 0.5f},    {-0.5f, 0.5f, 0.5f},  {-0.5f, 0.5f, -0.5f}};

const eigen::vector<Eigen::Vector2f> texture_coordinates{
    {0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {1.0f, 1.0f}, {0.0f, 1.0f},
    {0.0f, 0.0f}, {0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {1.0f, 1.0f},
    {0.0f, 1.0f}, {0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f},
    {0.0f, 1.0f}, {0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f},
    {0.0f, 1.0f}, {0.0f, 1.0f}, {0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 1.0f},
    {1.0f, 1.0f}, {1.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 1.0f},
    {0.0f, 1.0f}, {1.0f, 1.0f}, {1.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f},
    {0.0f, 1.0f}};

Eigen::Vector3f GetLocalCameraPos(float radius,
                                  float rot_vertical,
                                  float rot_horizontal) {
  float radius_2d = radius * cosf(rot_vertical);
  return {radius_2d * cosf(rot_horizontal),
          radius_2d * sinf(rot_horizontal),
          radius * sinf(rot_vertical)};
}

int main(int argc, char* argv[]) {
  gl::glfw::Viewer viewer{"OpenGlViewer"};
  const bool success = viewer.Initialize();
  if (!success) { return EXIT_FAILURE; }

  const auto image_container =
      Image::CreateFrom("opengl_tutorials/3_camera/textures/container.jpg");
  const auto image_face = Image::CreateFrom(
      "opengl_tutorials/3_camera/textures/awesomeface.png", true);
  if (!image_container || !image_face) {
    absl::FPrintF(stderr, "Error: images not found.\n");
    return EXIT_FAILURE;
  }

  auto texture_1{gl::Texture::Builder{gl::Texture::Type::kTexture2D,
                                      gl::Texture::Identifier::kTexture0}
                     .WithSaneDefaults()
                     .WithImage(*image_container)
                     .Build()};
  auto texture_2{gl::Texture::Builder{gl::Texture::Type::kTexture2D,
                                      gl::Texture::Identifier::kTexture1}
                     .WithSaneDefaults()
                     .WithImage(*image_face)
                     .Build()};

  const std::shared_ptr<gl::Shader> vertex_shader{gl::Shader::CreateFromFile(
      "opengl_tutorials/3_camera/shaders/triangle.vert")};
  const std::shared_ptr<gl::Shader> fragment_shader{gl::Shader::CreateFromFile(
      "opengl_tutorials/3_camera/shaders/triangle.frag")};
  if (!vertex_shader || !fragment_shader) { exit(EXIT_FAILURE); }

  const auto program =
      gl::Program::CreateFromShaders({vertex_shader, fragment_shader});
  if (!program) {
    std::cerr << "Failed to link program" << std::endl;
    return EXIT_FAILURE;
  }
  program->Use();
  program->SetUniform("texture1", 0);
  program->SetUniform("texture2", 1);

  float mixture = 0.5f;
  program->SetUniform("mix_ratio", mixture);

  gl::Camera camera{};

  Eigen::Matrix4f perspective = camera.Perspective(
      45_deg, viewer.window_size().width, viewer.window_size().height);
  program->SetUniform("projection", perspective);

  gl::VertexArrayBuffer vertex_array_buffer{};
  vertex_array_buffer.EnableVertexAttributePointer(
      0,
      std::make_shared<gl::Buffer>(gl::Buffer::Type::kArrayBuffer,
                                   gl::Buffer::Usage::kStaticDraw,
                                   vertices));
  vertex_array_buffer.EnableVertexAttributePointer(
      1,
      std::make_shared<gl::Buffer>(gl::Buffer::Type::kArrayBuffer,
                                   gl::Buffer::Usage::kStaticDraw,
                                   texture_coordinates));

  viewer.user_input_handler().RegisterKeyboardCallback(
      [&camera](gl::KeyboardKey key, gl::PressState state) {
        if (state != gl::PressState::kPressed) { return; }
        switch (key) {
          case gl::KeyboardKey::kArrowUp:
            camera.Rotate(gl::Camera::RotationDirection::kVertical, 0.1_rad);
            break;
          case gl::KeyboardKey::kArrowDown:
            camera.Rotate(gl::Camera::RotationDirection::kVertical, -0.1_rad);
            break;
          case gl::KeyboardKey::kArrowLeft:
            camera.Rotate(gl::Camera::RotationDirection::kHorizontal, -0.1_rad);
            break;
          case gl::KeyboardKey::kArrowRight:
            camera.Rotate(gl::Camera::RotationDirection::kHorizontal, 0.1_rad);
            break;
          default: return;
        }
      });

  viewer.user_input_handler().RegisterMouseCallback(
      [&camera](gl::MouseKey key,
                gl::PressState state,
                float x_increment,
                float y_increment) {
        if (state != gl::PressState::kPressed) { return; }
        float modifier = 0.01f;
        camera.Rotate(gl::Camera::RotationDirection::kHorizontal,
                      units::angle::radian_t{x_increment},
                      modifier);
        camera.Rotate(gl::Camera::RotationDirection::kVertical,
                      units::angle::radian_t{y_increment},
                      modifier);
      });

  camera.LookAt({0, 0, 0}, {5, 0, 0});

  while (!viewer.ShouldClose()) {
    viewer.ProcessInput();

    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    program->Use();
    texture_1->Bind();
    texture_2->Bind();
    program->SetUniform("view", camera.TfCameraWorld().matrix());
    Eigen::Affine3f model{Eigen::AngleAxisf{0.0f, Eigen::Vector3f::UnitX()}};
    program->SetUniform("model", model.matrix());

    vertex_array_buffer.Draw(GL_TRIANGLES);
    viewer.Spin();
  }
  return EXIT_SUCCESS;
}
