#include <model_scanner/Window.h>

namespace model_scanner {

Window::Window(const std::string& deviceName,
               const std::string& calibrationFile, GLuint width, GLuint height,
               const std::string& winname)
  : _camera(deviceName, calibrationFile),
    _width(width),
    _height(height),
    _winname(winname) {
  if (gWindow != nullptr) {
    std::cerr << "Error: Global Window object already exists, nothing will be "
              << "done for window " << _winname << std::endl;
    return;
  }
  gWindow = this;

  if (_width == 0) {
    _width = _camera.width;
  }
  if (_height == 0) {
    _height = _camera.height;
  }

  glutInitWindowSize(_width, _height);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

  glGenTextures(1, &_tex);

  _mainWindow = glutCreateWindow(_winname.c_str());
  glutIdleFunc(Window::idle);
  glutReshapeFunc(Window::resize);
  glutDisplayFunc(Window::display);

  glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
  glEnable(GL_LIGHT0);
  glEnable(GL_LIGHTING);
  glEnable(GL_COLOR_MATERIAL);
  glEnable(GL_DEPTH_TEST);
}

Window::~Window() {
  if (gWindow == this)
    gWindow = nullptr;
}

void Window::idle() {
  cv::Mat frame = gWindow->_camera.getFrame();
  cv::cvtColor(frame, frame, cv::ColorConversionCodes::COLOR_BGR2RGB);
  cv::flip(frame, frame, 0);
  glBindTexture(GL_TEXTURE_2D, gWindow->_tex);

  glPixelStorei(GL_UNPACK_ALIGNMENT, (frame.step & 0b11) ? 1 : 4);
  glPixelStorei(GL_UNPACK_ROW_LENGTH, frame.step / frame.elemSize());

  glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);

  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, frame.cols, frame.rows, 0, GL_RGB,
               GL_UNSIGNED_BYTE, frame.data);
  glBindTexture(GL_TEXTURE_2D, 0);

  glutPostRedisplay();
}

void Window::resize(int w, int h) {
  double aspect = (double) gWindow->_camera.width / gWindow->_camera.height;

  glutReshapeWindow(w, h);
  if ((double) w / h > aspect)
    glViewport((int) (w - h * aspect) / 2, 0, (int) (h * aspect), h);
  else
    glViewport(0, (int) (h - w / aspect) / 2, w, (int) (w / aspect));
  gWindow->_width = w;
  gWindow->_height = h;
}

void Window::display() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Store current projection matrix and enable bits
  glPushAttrib(GL_ENABLE_BIT);
  glPushMatrix();
  glLoadIdentity();
  gluOrtho2D(0, 1, 0, 1);

  // Set Enable bits
  glEnable(GL_TEXTURE_2D);
  glDisable(GL_LIGHTING);
  glDisable(GL_DEPTH_TEST);

  // Draw Background
  glBindTexture(GL_TEXTURE_2D, gWindow->_tex);

  glBegin(GL_QUADS);
  glColor3i(0, 0, 0);
  glTexCoord2f(0.0f, 0.0f);
  glVertex2i(0, 0);
  glTexCoord2f(1.0f, 0.0f);
  glVertex2i(1, 0);
  glTexCoord2f(1.0f, 1.0f);
  glVertex2i(1, 1);
  glTexCoord2f(0.0f, 1.0f);
  glVertex2i(0, 1);
  glEnd();

  glPopMatrix();
  glPopAttrib();

  glutSwapBuffers();
}

Window* Window::gWindow = nullptr;

}  // namespace model_scanner
