#ifndef MODEL_SCANNER_CAMERA_H
#define MODEL_SCANNER_CAMERA_H

#include <opencv2/opencv.hpp>

namespace model_scanner {

class Camera {
public:
  struct Calibration {
    cv::Mat k;
    cv::Mat d;
  } calibration;

  Camera(const std::string& deviceName, const std::string& calibrationFile);
  ~Camera();

  cv::Mat getFrame();

  int width;
  int height;
private:
  std::string _deviceName;
  std::string _calibrationFile;
  cv::VideoCapture _cap;
  cv::Mat _blankFrame;
};

}  // namespace model_scanner

#endif  // MODEL_SCANNER_CAMERA_H
