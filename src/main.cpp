#include <iostream>
#include <getopt.h>
#include <opencv2/opencv.hpp>
#include <GL/glew.h>
#include <GL/glut.h>
#include <model_scanner/Window.h>

int main(int argc, char** argv) {
  glutInit(&argc, argv);

  std::string source = "/dev/video0";
  std::string outputFile = "model.stl";
  std::string cameraInfo = "";
  uint octreeDepth = 4;

  static struct option longopts[] = {
    { "octree-depth", required_argument, nullptr, 'd' },
    { "output", required_argument, nullptr, 'o' },
    { "camera-info", required_argument, nullptr, 'c' },
    { "source", required_argument, nullptr, 's' },
    { 0, 0, 0, 0 }
  };

  int longind = 0;
  int opt;
  while ((opt = getopt_long(argc, argv, "d:o:c:s:", longopts, &longind)) !=
         -1) {
    switch (opt) {
      case 'd': {
        std::stringstream ss(optarg);
        ss >> octreeDepth;
        break;
      }
      case 'o':
        outputFile = optarg;
        break;
      case 'c':
        cameraInfo = optarg;
        break;
      case 's':
        source = optarg;
        break;
      default:
        break;
    }
  }

  model_scanner::Window window(source, cameraInfo, outputFile, octreeDepth);
  glutMainLoop();
  return 0;
}
