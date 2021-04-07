#include <model_scanner/Camera.h>

namespace model_scanner {

Camera::Camera(const std::string& deviceName,
               const std::string& calibrationFile)
  : _deviceName(deviceName),
    _calibrationFile(calibrationFile),
    _cap(deviceName) {
  cv::FileStorage fs(_calibrationFile, cv::FileStorage::Mode::READ);
  if (!fs.isOpened()) {
    calibration.k = cv::Mat::eye(3, 3, CV_64F);
    calibration.d = cv::Mat::zeros(1, 5, CV_64F);
  } else {
    fs["k"] >> calibration.k;
    fs["d"] >> calibration.d;
    fs.release();
  }

  width = _cap.get(cv::VideoCaptureProperties::CAP_PROP_FRAME_WIDTH);
  height = _cap.get(cv::VideoCaptureProperties::CAP_PROP_FRAME_HEIGHT);
}

Camera::~Camera() {
  _cap.release();
}

cv::Mat Camera::getFrame() {
  cv::Mat rawImage;
  cv::Mat undistorted;
  _cap >> rawImage;
  if (rawImage.empty())
    return rawImage;
  cv::undistort(rawImage, undistorted, calibration.k, calibration.d);
  return undistorted;
}

void Camera::writeCameraInfo() {
  cv::FileStorage fs(_calibrationFile, cv::FileStorage::Mode::WRITE);
  fs << "k" << calibration.k;
  fs << "d" << calibration.d;
}

}  // namespace model_scanner
