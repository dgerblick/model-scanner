#include <model_scanner/Camera.h>

namespace model_scanner {

Camera::Camera(const std::string& deviceName,
               const std::string& calibrationFile)
  : _deviceName(deviceName),
    _calibrationFile(calibrationFile),
    _cap(deviceName) {
  cv::FileStorage fs(_calibrationFile, cv::FileStorage::Mode::READ);
  if (!fs.isOpened()) {
    _calibration.k = cv::Mat::eye(3, 3, CV_64F);
    _calibration.d = cv::Mat::zeros(1, 5, CV_64F);
  } else {
    fs["k"] >> _calibration.k;
    fs["d"] >> _calibration.d;
    fs.release();
  }
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
  cv::undistort(rawImage, undistorted, _calibration.k, _calibration.d);
  return undistorted;
}

void Camera::writeCameraInfo() {
  cv::FileStorage fs(_calibrationFile, cv::FileStorage::Mode::WRITE);
  fs << "k" << _calibration.k;
  fs << "d" << _calibration.d;
}

}  // namespace model_scanner
