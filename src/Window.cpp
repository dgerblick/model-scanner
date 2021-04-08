#include <model_scanner/Window.h>
#include <fstream>
#include <sstream>

namespace model_scanner {

Window::Window(const std::string& deviceName,
               const std::string& calibrationFile, GLuint width, GLuint height,
               const std::string& winname)
  : _camera(deviceName, calibrationFile),
    _aprilTagDetector(_camera),
    _width(width),
    _height(height),
    _winname(winname) {
  if (gWindow != nullptr) {
    std::cerr << "Error: Global Window object already exists, nothing will be "
              << "done for window " << _winname << std::endl;
    return;
  }
  gWindow = this;

  AprilTagDetector::TagParams params = { .id = 0, .tagSize = TAG_SIZE };
  _aprilTagDetector.addTagParams(params);

  double fx = _camera.calibration.k.at<double>(0, 0);
  double fy = _camera.calibration.k.at<double>(1, 1);
  double cx = _camera.calibration.k.at<double>(0, 2);
  double cy = _camera.calibration.k.at<double>(1, 2);
  double zfar = 10.0;
  double znear = 0.01;

  _projMatrix[0][0] = 2.0 * fx / _camera.width;
  _projMatrix[0][1] = 0.0;
  _projMatrix[0][2] = 0.0;
  _projMatrix[0][3] = 0.0;

  _projMatrix[1][0] = 0.0;
  _projMatrix[1][1] = -2.0 * fy / _camera.height;
  _projMatrix[1][2] = 0.0;
  _projMatrix[1][3] = 0.0;

  _projMatrix[2][0] = 1.0 - 2.0 * cx / _camera.width;
  _projMatrix[2][1] = 2.0 * cy / _camera.height - 1.0;
  _projMatrix[2][2] = (zfar + znear) / (znear - zfar);
  _projMatrix[2][3] = -1.0;

  _projMatrix[3][0] = 0.0;
  _projMatrix[3][1] = 0.0;
  _projMatrix[3][2] = -2.0 * znear * zfar / (zfar - znear);
  _projMatrix[3][3] = 0.0;

  if (_width == 0)
    _width = _camera.width;
  if (_height == 0)
    _height = _camera.height;

  glutInitWindowSize(_width, _height);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

  _mainWindow = glutCreateWindow(_winname.c_str());
  GLenum err = glewInit();
  if (glewInit() != GLEW_OK) {
    std::cerr << "Error: " << glewGetErrorString(err) << std::endl;
    return;
  }
  glutIdleFunc(Window::idle);
  glutReshapeFunc(Window::resize);
  glutDisplayFunc(Window::display);

  glEnable(GL_DEPTH_TEST);

  glGenTextures(4, _tex);
  glGenFramebuffers(4, _frameBuffers);
  glGenRenderbuffers(4, _depthRenderBuffers);
  for (size_t i = 0; i < 4; ++i) {
    glBindTexture(GL_TEXTURE_2D, _tex[i]);
    glBindFramebuffer(GL_FRAMEBUFFER, _frameBuffers[i]);
    glBindRenderbuffer(GL_RENDERBUFFER, _depthRenderBuffers[i]);

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, _camera.width, _camera.height, 0,
                 GL_RGB, GL_UNSIGNED_BYTE, 0);

    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, _camera.width,
                          _camera.height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                              GL_RENDERBUFFER, _depthRenderBuffers[i]);

    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, _tex[i], 0);
  }
  glBindTexture(GL_TEXTURE_2D, 0);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glBindRenderbuffer(GL_RENDERBUFFER, 0);

  GLint status;

  std::ifstream maskShaderFile("shaders/mask.glsl");
  std::stringstream maskShaderSrc;
  maskShaderSrc << maskShaderFile.rdbuf();
  std::string maskShaderStr = maskShaderSrc.str();
  const char* maskShaderSrcCStr = maskShaderStr.c_str();

  GLuint maskShader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(maskShader, 1, &maskShaderSrcCStr, nullptr);
  glCompileShader(maskShader);
  glGetShaderiv(maskShader, GL_COMPILE_STATUS, &status);
  if (status != GL_TRUE) {
    char buffer[512];
    glGetShaderInfoLog(maskShader, 512, nullptr, buffer);
    std::cerr << "Error compiling shader: " << std::endl << buffer << std::endl;
    return;
  }

  _prog = glCreateProgram();
  glAttachShader(_prog, maskShader);
  glLinkProgram(_prog);
  glGetProgramiv(_prog, GL_LINK_STATUS, &status);
  if (status != GL_TRUE) {
    char buffer[512];
    glGetProgramInfoLog(_prog, 512, nullptr, buffer);
    std::cerr << "Error linking shader: " << std::endl << buffer << std::endl;
    return;
  }

  glDeleteShader(maskShader);

  _maskShaderTexLoc = glGetUniformLocation(_prog, "image");
  _maskShaderScreenSizeLoc = glGetUniformLocation(_prog, "screenSize");
}  // namespace model_scanner

Window::~Window() {
  if (gWindow == this)
    gWindow = nullptr;
}

void Window::render0() {
  glBindTexture(GL_TEXTURE_2D, _tex[0]);

  cv::Mat frame = _camera.getFrame();
  _aprilTagDetector.setFrame(frame);

  cv::cvtColor(frame, frame, cv::ColorConversionCodes::COLOR_BGR2RGB);
  cv::flip(frame, frame, 0);

  glPixelStorei(GL_UNPACK_ALIGNMENT, (frame.step & 0b11) ? 1 : 4);
  glPixelStorei(GL_UNPACK_ROW_LENGTH, frame.step / frame.elemSize());

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, frame.cols, frame.rows, 0, GL_RGB,
               GL_UNSIGNED_BYTE, frame.data);
  glBindTexture(GL_TEXTURE_2D, 0);
}

void Window::render1() {
  GLfloat verts[8][3]{ { MIN_X, MIN_Y, TAG_SIZE / 2 },
                       { MIN_X, MIN_Y, 0 },
                       { MIN_X, MAX_Y, 0 },
                       { MIN_X, MAX_Y, TAG_SIZE / 2 },
                       { MAX_X, MIN_Y, TAG_SIZE / 2 },
                       { MAX_X, MIN_Y, 0 },
                       { MAX_X, MAX_Y, 0 },
                       { MAX_X, MAX_Y, TAG_SIZE / 2 } };
  GLint faces[6][4]{ { 0, 1, 2, 3 }, { 3, 2, 6, 7 }, { 7, 6, 5, 4 },
                     { 4, 5, 1, 0 }, { 5, 6, 2, 1 }, { 7, 4, 0, 3 } };

  glBindFramebuffer(GL_FRAMEBUFFER, _frameBuffers[1]);

  glPushMatrix();
  glPushAttrib(GL_ENABLE_BIT);

  glLoadIdentity();
  gluOrtho2D(0, 1, 0, 1);

  glEnable(GL_TEXTURE_2D);
  glDisable(GL_DEPTH_TEST);

  glBindTexture(GL_TEXTURE_2D, _tex[0]);

  glBegin(GL_QUADS);
  glTexCoord2d(0.0, 0.0);
  glVertex2d(0.0, 0.0);
  glTexCoord2d(1.0, 0.0);
  glVertex2d(1.0, 0.0);
  glTexCoord2d(1.0, 1.0);
  glVertex2d(1.0, 1.0);
  glTexCoord2d(0.0, 1.0);
  glVertex2d(0.0, 1.0);
  glEnd();

  glBindTexture(GL_TEXTURE_2D, 0);

  glLoadMatrixf((GLfloat*) _projMatrix);

  glPopAttrib();
  cv::Mat modelView = _aprilTagDetector.getPose(0);
  if (!modelView.empty()) {
    glClear(GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf((GLfloat*) modelView.data);

    glBegin(GL_QUADS);
    for (int i = 0; i < 6; i++) {
      glColor3d(i / 6.0, (i % 2) / 2.0, (i % 3) / 3.0);
      for (int j = 0; j < 4; j++) {
        glVertex3fv(&verts[faces[i][j]][0]);
      }
    }
    glEnd();

    glLoadIdentity();
    glMatrixMode(GL_PROJECTION);
  }

  glPopMatrix();

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Window::render2() {
  glBindFramebuffer(GL_FRAMEBUFFER, _frameBuffers[2]);

  glPushMatrix();
  glPushAttrib(GL_ENABLE_BIT);

  glLoadIdentity();
  gluOrtho2D(0, 1, 0, 1);

  glDisable(GL_DEPTH_TEST);

  glBegin(GL_QUADS);
  glColor3f(0.0, 0.0, 0.0);
  glTexCoord2d(0.0, 0.0);
  glVertex2d(0.0, 0.0);
  glTexCoord2d(1.0, 0.0);
  glVertex2d(1.0, 0.0);
  glTexCoord2d(1.0, 1.0);
  glVertex2d(1.0, 1.0);
  glTexCoord2d(0.0, 1.0);
  glVertex2d(0.0, 1.0);
  glEnd();

  glLoadMatrixf((GLfloat*) _projMatrix);

  glPopAttrib();
  cv::Mat modelView = _aprilTagDetector.getPose(0);
  if (!modelView.empty()) {
    glClear(GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf((GLfloat*) modelView.data);

    glUseProgram(_prog);
    glBindTexture(GL_TEXTURE_2D, _tex[0]);
    glUniform1ui(_maskShaderTexLoc, 0);
    glUniform2f(_maskShaderScreenSizeLoc, _camera.width, _camera.height);

    glBegin(GL_QUADS);
    glVertex3d(MIN_X, MIN_Y, 0);
    glVertex3d(MIN_X, MAX_Y, 0);
    glVertex3d(MAX_X, MAX_Y, 0);
    glVertex3d(MAX_X, MIN_Y, 0);
    glEnd();

    glUseProgram(0);
    glBindTexture(GL_TEXTURE_2D, 0);

    glLoadIdentity();
    glMatrixMode(GL_PROJECTION);
  }

  glPopMatrix();

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Window::render3() {
}

void Window::idle() {
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

  glPushAttrib(GL_VIEWPORT_BIT);
  glViewport(0, 0, gWindow->_camera.width, gWindow->_camera.height);
  gWindow->render0();
  gWindow->render1();
  gWindow->render2();
  gWindow->render3();
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glPopAttrib();

  // Store current projection matrix and enable bits
  glPushAttrib(GL_ENABLE_BIT);
  glPushMatrix();
  glLoadIdentity();
  gluOrtho2D(0, 1, 0, 1);

  // Set Enable bits
  glEnable(GL_TEXTURE_2D);
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

  glutSwapBuffers();
}

Window* Window::gWindow = nullptr;

}  // namespace model_scanner
