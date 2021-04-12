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
      glm::mat4 modelView(1.0);

      modelView[0][0] = pose.R->data[0];
      modelView[0][1] = -pose.R->data[3];
      modelView[0][2] = -pose.R->data[6];
      modelView[0][3] = 0.0;

      modelView[1][0] = -pose.R->data[1];
      modelView[1][1] = pose.R->data[4];
      modelView[1][2] = pose.R->data[7];
      modelView[1][3] = 0.0;

      modelView[2][0] = -pose.R->data[2];
      modelView[2][1] = pose.R->data[5];
      modelView[2][2] = pose.R->data[8];
      modelView[2][3] = 0.0;

      modelView[3][0] = pose.t->data[0];
      modelView[3][1] = -pose.t->data[1];
      modelView[3][2] = -pose.t->data[2];
      modelView[3][3] = 1.0;

      _lastFrameTagPos[det->id] = modelView;
    } else {
      std::cout << "Warning: No size known for tag " << det->id << std::endl;
    }
  }
  apriltag_detections_destroy(detections);
}

glm::mat4 AprilTagDetector::getPose(int id) {
  return _lastFrameTagPos[id];
}

}  // namespace model_scanner
