#include <iostream>
#include <unistd.h>
#include <opencv2/opencv.hpp>
#include <apriltag/apriltag.h>
#include <apriltag/tagStandard41h12.h>

int main(int argc, char** argv) {
  std::string deviceName = "/dev/video0";

  int c;
  while ((c = getopt(argc, argv, "c:")) != -1) {
    switch (c) {
      case 'c':
        deviceName = optarg;
        break;
      default:
        return -1;
    }
  }

  cv::Mat frame;
  cv::Mat gray;
  cv::VideoCapture cap(deviceName, cv::CAP_V4L);
  if (!cap.isOpened()) {
    std::cerr << "Unable to open camera" << std::endl;
    return -1;
  }

  apriltag_family_t* tf = tagStandard41h12_create();
  apriltag_detector_t* td = apriltag_detector_create();
  apriltag_detector_add_family(td, tf);
  td->quad_decimate = 2.0;
  td->quad_sigma = 0.0;
  td->nthreads = 1;
  td->debug = 0;
  td->refine_edges = 1;

  while (1) {
    cap >> frame;
    if (frame.empty()) {
      std::cerr << "Frame is empty" << std::endl;
      break;
    }
    cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);

    image_u8_t im = { .width = gray.cols,
                      .height = gray.rows,
                      .stride = gray.cols,
                      .buf = gray.data };
    zarray_t* detections = apriltag_detector_detect(td, &im);

    for (int i = 0; i < zarray_size(detections); i++) {
      apriltag_detection_t* det;
      zarray_get(detections, i, &det);
      line(frame, cv::Point(det->p[0][0], det->p[0][1]),
           cv::Point(det->p[1][0], det->p[1][1]), cv::Scalar(0, 0xff, 0), 2);
      line(frame, cv::Point(det->p[0][0], det->p[0][1]),
           cv::Point(det->p[3][0], det->p[3][1]), cv::Scalar(0, 0, 0xff), 2);
      line(frame, cv::Point(det->p[1][0], det->p[1][1]),
           cv::Point(det->p[2][0], det->p[2][1]), cv::Scalar(0xff, 0, 0), 2);
      line(frame, cv::Point(det->p[2][0], det->p[2][1]),
           cv::Point(det->p[3][0], det->p[3][1]), cv::Scalar(0xff, 0, 0), 2);

      std::stringstream ss;
      ss << det->id;
      std::string text = ss.str();
      int fontface = cv::FONT_HERSHEY_SCRIPT_SIMPLEX;
      double fontscale = 1.0;
      int baseline;
      cv::Size textsize =
          cv::getTextSize(text, fontface, fontscale, 2, &baseline);
      cv::putText(frame, text,
                  cv::Point(det->c[0] - textsize.width / 2,
                            det->c[1] + textsize.height / 2),
                  fontface, fontscale, cv::Scalar(0xff, 0x99, 0), 2);
    }
    apriltag_detections_destroy(detections);

    cv::imshow("Webcam", frame);
    if (cv::waitKey(1) == 27) {
      std::cerr << "Exiting..." << std::endl;
      break;
    }
  }

  apriltag_detector_destroy(td);
  tagStandard41h12_destroy(tf);
  return 0;
}
