#ifndef MODEL_SCANNER_WINDOW_H
#define MODEL_SCANNER_WINDOW_H

#include <GL/glut.h>
#include <GL/gl.h>
#include <model_scanner/Camera.h>

namespace model_scanner {

class Window {
public:
  Window(const std::string& deviceName, const std::string& calibrationFile,
         GLuint width, GLuint height,
         const std::string& winname = "Model Scanner");
  ~Window();

private:
  Camera _camera;
  GLuint _tex;

  GLuint _width;
  GLuint _height;
  std::string _winname;
  GLuint _mainWindow;

  void init();

  static void idle();
  static void resize(int width, int height);
  static void display();

  static Window* gWindow;
};

}  // namespace model_scanner

#endif  // MODEL_SCANNER_WINDOW_H
