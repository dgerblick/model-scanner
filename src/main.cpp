#include <iostream>
#include <getopt.h>
#include <opencv2/opencv.hpp>
#include <GL/glut.h>
#include <GL/gl.h>
#include <model_scanner/Window.h>

int main(int argc, char** argv) {
  glutInit(&argc, argv);

  std::string deviceName = "/dev/video0";
  std::string outputDir = "untitled";
  std::string cameraInfo = "camera_info.yml";

  static struct option longopts[] = {
    { "device", required_argument, nullptr, 'd' },
    { "output", required_argument, nullptr, 'o' },
    { "camera_info", required_argument, nullptr, 'c' },
    { 0, 0, 0, 0 }
  };

  int longind = 0;
  int opt;
  while ((opt = getopt_long(argc, argv, "d:o:c:", longopts, &longind)) != -1) {
    switch (opt) {
      case 'd':
        deviceName = optarg;
        break;
      case 'o':
        outputDir = optarg;
        break;
      case 'c':
        cameraInfo = optarg;
        break;
      default:
        break;
    }
  }

  model_scanner::Window window(deviceName, cameraInfo);
  glutMainLoop();
  return 0;
}
