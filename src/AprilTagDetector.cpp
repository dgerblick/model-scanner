#include <model_scanner/AprilTagDetector.h>

namespace model_scanner {

AprilTagDetector::AprilTagDetector(const Camera& camera) {
  _td = apriltag_detector_create();
  _tf = tagStandard41h12_create();

  apriltag_detector_add_family(_td, _tf);
  _td->quad_decimate = 2.0;
  _td->quad_sigma = 0.0;
  _td->nthreads = 1;
  _td->debug = 0;
  _td->refine_edges = 1;

  _info.det = nullptr;
  _info.tagsize = -1.0;
  _info.fx = camera.calibration.k.at<double>(0, 0);
  _info.fy = camera.calibration.k.at<double>(1, 1);
  _info.cx = camera.calibration.k.at<double>(0, 2);
  _info.cy = camera.calibration.k.at<double>(1, 2);
}

AprilTagDetector::~AprilTagDetector() {
  apriltag_detector_destroy(_td);
  tagStandard41h12_destroy(_tf);
}

void AprilTagDetector::addTagParams(TagParams params) {
  _tagSizes[params.id] = params.tagSize;
}

void AprilTagDetector::setFrame(const cv::Mat& frame) {
  cv::Mat grey;
  cv::cvtColor(frame, grey, cv::COLOR_BGR2GRAY);

  image_u8_t image = { .width = grey.cols,
                       .height = grey.rows,
                       .stride = grey.cols,
                       .buf = grey.data };

  _lastFrameTagPos.clear();
  zarray_t* detections = apriltag_detector_detect(_td, &image);
  for (int i = 0; i < zarray_size(detections); ++i) {
    apriltag_detection_t* det;
    zarray_get(detections, i, &det);

    if (_tagSizes.contains(det->id)) {
      _info.det = det;
      _info.tagsize = _tagSizes[det->id];
      apriltag_pose_t pose;
      estimate_tag_pose(&_info, &pose);
      //_lastFrameTagPos[det->id] = cv::Mat(4, 4, CV_32F);
      //_lastFrameTagPos[det->id].at(0, 0) = 0;
    } else {
      std::cout << "Warning: No size known for tag " << det->id << std::endl;
    }
  }
  apriltag_detections_destroy(detections);
}

cv::Mat AprilTagDetector::getPose(int id) {
  cv::Mat mat;
  return mat;
}

}  // namespace model_scanner
