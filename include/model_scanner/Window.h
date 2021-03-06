#ifndef MODEL_SCANNER_WINDOW_H
#define MODEL_SCANNER_WINDOW_H

#include <GL/glew.h>
#include <GL/glut.h>
#include <glm/matrix.hpp>
#include <model_scanner/Camera.h>
#include <model_scanner/AprilTagDetector.h>
#include <model_scanner/Octree.h>

namespace model_scanner {

class Window {
public:
  Window(const std::string& deviceName, const std::string& calibrationFile,
         const std::string& outFileName, int octreeDepth, GLuint width = 0,
         GLuint height = 0, const std::string& winname = "Model Scanner");
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
  float _threshold;
  Octree _octree;
  std::string _outFileName;

  GLuint _prog;
  GLuint _shaderMaskModeLoc;
  GLuint _shaderTexLoc;
  GLuint _shaderScreenSizeLoc;
  GLuint _shaderinvProjLoc;
  GLuint _shaderInvModelViewLoc;
  GLuint _shaderThresholdLoc;
  GLuint _shaderOctreeSsbo;

  void render0();
  void render1();
  void render2();
  void render3();

  static void idle();
  static void resize(int width, int height);
  static void display();
  static void keyboard(unsigned char key, int x, int y);

  static std::string loadFile(const std::string& filename);

  static Window* gWindow;

  static constexpr double TAG_SIZE = 0.08333333333;
  static constexpr double SQUARE_SIZE = 0.05;
  static constexpr glm::vec3 OFFSET{ 0.0, -0.125, SQUARE_SIZE };
};

}  // namespace model_scanner

#endif  // MODEL_SCANNER_WINDOW_H
