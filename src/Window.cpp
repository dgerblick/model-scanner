#include <model_scanner/Window.h>

namespace model_scanner {

Window::Window(const std::string& deviceName,
               const std::string& calibrationFile, GLuint width, GLuint height,
               const std::string& winname)
  : _camera(deviceName, calibrationFile),
    _aprilTagDetector(_camera),
    _width(width),
    _height(height),
    _winname(winname),
    _projMatrix(4, 4, CV_64F) {
  if (gWindow != nullptr) {
    std::cerr << "Error: Global Window object already exists, nothing will be "
              << "done for window " << _winname << std::endl;
    return;
  }
  gWindow = this;

  AprilTagDetector::TagParams params = { .id = 0, .tagSize = 0.08333333333 };
  _aprilTagDetector.addTagParams(params);

  double fx = _camera.calibration.k.at<double>(0, 0);
  double fy = _camera.calibration.k.at<double>(1, 1);
  double cx = _camera.calibration.k.at<double>(0, 2);
  double cy = _camera.calibration.k.at<double>(1, 2);
  double znear = 0.01;
  double zfar = 10.0;

  _projMatrix.at<double>(0, 0) = 2.0 * fx / _camera.width;
  _projMatrix.at<double>(0, 1) = 0.0;
  _projMatrix.at<double>(0, 2) = 0.0;
  _projMatrix.at<double>(0, 3) = 0.0;

  _projMatrix.at<double>(1, 0) = 0.0;
  _projMatrix.at<double>(1, 1) = -2.0 * fy / _camera.height;
  _projMatrix.at<double>(1, 2) = 0.0;
  _projMatrix.at<double>(1, 3) = 0.0;

  _projMatrix.at<double>(2, 0) = 1.0 - 2.0 * cx / _camera.width;
  _projMatrix.at<double>(2, 1) = 2.0 * cy / _camera.height - 1.0;
  _projMatrix.at<double>(2, 2) = (znear + zfar) / (znear - zfar);
  _projMatrix.at<double>(2, 3) = -1.0;

  _projMatrix.at<double>(4, 0) = 0.0;
  _projMatrix.at<double>(4, 1) = 0.0;
  _projMatrix.at<double>(4, 2) = 2.0 * znear * zfar / (znear - zfar);
  _projMatrix.at<double>(4, 3) = 0.0;

  if (_width == 0) {
    _width = _camera.width;
  }
  if (_height == 0) {
    _height = _camera.height;
  }

  glutInitWindowSize(_width, _height);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

  _mainWindow = glutCreateWindow(_winname.c_str());
  glutIdleFunc(Window::idle);
  glutReshapeFunc(Window::resize);
  glutDisplayFunc(Window::display);

  glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
  glEnable(GL_LIGHT0);
  glEnable(GL_LIGHTING);
  glEnable(GL_COLOR_MATERIAL);
  glEnable(GL_DEPTH_TEST);

  glGenTextures(4, _tex);
  // glGenFramebuffers(4, _frameBuffers);
  for (size_t i = 0; i < 4; ++i) {
    glBindTexture(GL_TEXTURE_2D, _tex[i]);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, _camera.width, _camera.height, 0,
                 GL_RGB, GL_UNSIGNED_BYTE, 0);
  }
  glBindTexture(GL_TEXTURE_2D, 0);
}

Window::~Window() {
  if (gWindow == this)
    gWindow = nullptr;
}

void Window::render0() {
  glBindTexture(GL_TEXTURE_2D, gWindow->_tex[0]);

  cv::Mat frame = gWindow->_camera.getFrame();
  gWindow->_aprilTagDetector.setFrame(frame);

  cv::cvtColor(frame, frame, cv::ColorConversionCodes::COLOR_BGR2RGB);
  cv::flip(frame, frame, 0);

  glPixelStorei(GL_UNPACK_ALIGNMENT, (frame.step & 0b11) ? 1 : 4);
  glPixelStorei(GL_UNPACK_ROW_LENGTH, frame.step / frame.elemSize());

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, frame.cols, frame.rows, 0, GL_RGB,
               GL_UNSIGNED_BYTE, frame.data);

  glBindTexture(GL_TEXTURE_2D, 0);
}

void Window::render1() {
}

void Window::render2() {
}

void Window::render3() {
}

void Window::idle() {
  gWindow->render0();
  gWindow->render1();
  gWindow->render2();
  gWindow->render3();

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

  // Render Quadrants
  for (size_t i = 0; i < 4; ++i) {
    double startX = (i / 2) / 2.0;
    double startY = (i % 2) / 2.0;

    glBindTexture(GL_TEXTURE_2D, gWindow->_tex[i]);

    glBegin(GL_QUADS);
    glTexCoord2d(0.0, 0.0);
    glVertex2d(startX, startY);
    glTexCoord2d(1.0, 0.0);
    glVertex2d(startX + 0.5, startY);
    glTexCoord2d(1.0, 1.0);
    glVertex2d(startX + 0.5, startY + 0.5);
    glTexCoord2d(0.0, 1.0);
    glVertex2d(startX, startY + 0.5);
    glEnd();
  }
  glBindTexture(GL_TEXTURE_2D, 0);

  // Restore Everything
  glPopMatrix();
  glPopAttrib();

  /*
    glPushMatrix();
    glLoadIdentity();
    glLoadMatrixd((GLdouble*) gWindow->_projMatrix.data);

    glBindTexture(GL_TEXTURE_2D, 0);
    glBegin(GL_QUADS);
    glColor3i(1, 0, 0);
    glVertex3f(-1.0, -1.0, -1.0);
    glVertex3f(1.0, -1.0, -1.0);
    glVertex3f(-1.0, 1.0, -1.0);
    glVertex3f(1.0, 1.0, -1.0);
    glEnd();

    glPopMatrix();
  */

  glutSwapBuffers();
}

Window* Window::gWindow = nullptr;

}  // namespace model_scanner
