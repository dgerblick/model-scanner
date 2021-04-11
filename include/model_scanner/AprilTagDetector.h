#ifndef MODEL_SCANNER_APRIL_TAG_DETECTOR_H
#define MODEL_SCANNER_APRIL_TAG_DETECTOR_H

#include <vector>
#include <map>
#include <glm/matrix.hpp>
#include <opencv2/opencv.hpp>
#include <apriltag/tagStandard41h12.h>
#include <apriltag/apriltag.h>
#include <apriltag/apriltag_pose.h>
#include <model_scanner/Camera.h>

namespace model_scanner {

class AprilTagDetector {
public:
  struct TagParams {
    int id;
    double tagSize;
  };

  AprilTagDetector(const Camera& camera);
  ~AprilTagDetector();

  void addTagParams(TagParams params);
  void setFrame(const cv::Mat& frame);
  glm::mat4 getPose(int id);

private:
  apriltag_detector_t* _td;
  apriltag_family_t* _tf;
  std::map<int, double> _tagSizes;

  std::map<int, glm::mat4> _lastFrameTagPos;
  apriltag_detection_info_t _info;
};

}  // namespace model_scanner

#endif  // MODEL_SCANNER_APRIL_TAG_DETECTOR_H
