#ifndef MODEL_SCANNER_WINDOW_H
#define MODEL_SCANNER_WINDOW_H

#include <GL/glew.h>
#include <GL/glut.h>
#include <glm/matrix.hpp>
#include <model_scanner/Camera.h>
#include <model_scanner/AprilTagDetector.h>

namespace model_scanner {

class Window {
public:
  Window(const std::string& deviceName, const std::string& calibrationFile,
         uint octreeDepth = 4, GLuint width = 0, GLuint height = 0,
         const std::string& winname = "Model Scanner");
  ~Window();

private:
  Camera _camera;
  AprilTagDetector _aprilTagDetector;

  GLuint _tex[4];
  GLuint _frameBuffers[4];
  GLuint _depthRenderBuffers[4];

  GLuint _width;
  GLuint _height;
  std::string _winname;
  GLuint _mainWindow;

  glm::mat4 _projMatrix;

  GLuint _prog;
  GLuint _shaderMaskModeLoc;
  GLuint _shaderTexLoc;
  GLuint _shaderScreenSizeLoc;
  GLuint _shaderinvProjLoc;
  GLuint _shaderInvModelViewLoc;
  GLuint _shaderOctreeSsbo;

  void render0();
  void render1();
  void render2();
  void render3();

  static void idle();
  static void resize(int width, int height);
  static void display();

  static std::string loadFile(const std::string& filename);

  static Window* gWindow;

  static constexpr double TAG_SIZE = 0.08333333333;
  static constexpr double SQUARE_SIZE = 0.05;
  static constexpr glm::vec3 OFFSET{ 0.0, -0.125, 0.025 };

  struct alignas(16) OctreeNode {
    uint hits;
    uint total;
    uint _unused[2];
    glm::vec4 minPoint;
    glm::vec4 maxPoint;
  };

  struct alignas(16) OctreeHeader {
    uint depth;
    uint size;
    uint _unused[2];
  };

  struct alignas(16) Octree {
    uint depth;
    uint size;
    OctreeNode nodes[];
  };
};

}  // namespace model_scanner

#endif  // MODEL_SCANNER_WINDOW_H
