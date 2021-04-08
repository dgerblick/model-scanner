#ifndef MODEL_SCANNER_WINDOW_H
#define MODEL_SCANNER_WINDOW_H

#include <GL/glew.h>
#include <GL/glut.h>
#include <model_scanner/Camera.h>
#include <model_scanner/AprilTagDetector.h>

namespace model_scanner {

class Window {
public:
  Window(const std::string& deviceName, const std::string& calibrationFile,
         GLuint width = 0, GLuint height = 0,
         const std::string& winname = "Model Scanner");
  ~Window();

private:
  Camera _camera;
  AprilTagDetector _aprilTagDetector;

  //GLuint _tex;
  GLuint _tex[4];
  GLuint _frameBuffers[4];
  GLuint _depthRenderBuffers[4];

  GLuint _width;
  GLuint _height;
  std::string _winname;
  GLuint _mainWindow;

  GLdouble _projMatrix[4][4];

  void render0();
  void render1();
  void render2();
  void render3();

  static void idle();
  static void resize(int width, int height);
  static void display();

  static Window* gWindow;

  static constexpr double TAG_SIZE = 0.08333333333;
};

}  // namespace model_scanner

#endif  // MODEL_SCANNER_WINDOW_H
