#ifndef MODEL_SCANNER_CAMERA_H
#define MODEL_SCANNER_CAMERA_H

#include <opencv2/opencv.hpp>

namespace model_scanner {

class Camera {
public:
  struct Calibration {
    cv::Mat k;
    cv::Mat d;
  };

  Camera(const std::string& deviceName = "/dev/video0",
         const std::string& calibrationFile = "camera_info.yml");
  ~Camera();

  cv::Mat getFrame();
  void writeCameraInfo();

private:
  std::string _deviceName;
  std::string _calibrationFile;
  cv::VideoCapture _cap;
  Calibration _calibration;
};

}  // namespace model_scanner

#endif  // MODEL_SCANNER_CAMERA_H