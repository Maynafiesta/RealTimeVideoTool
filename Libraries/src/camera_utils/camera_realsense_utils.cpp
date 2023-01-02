//
// Created by taric on 16.08.2022.
//

#include "camera_realsense_utils.h"

using namespace cv;

camera_realsense_utils::camera_realsense_utils(const uint16_t *frame_width_param,
                                               const uint16_t *frame_height_param,
                                               const uint8_t *fps_param) {

    this->frame_width = *frame_width_param;
    this->frame_height = *frame_height_param;
    this->fps = *fps_param;
    this->camera_stop_flag = false;
    this->cfg.enable_stream(RS2_STREAM_COLOR,
                            frame_width,
                            frame_height,
                            RS2_FORMAT_BGR8,
                            fps);
    this->pipe.start(this->cfg);
}

camera_realsense_utils::~camera_realsense_utils() = default;

void camera_realsense_utils::get_rgb_frame(cv::Mat *dest) {

    frame = this->pipe.wait_for_frames();
    color_frame = frame.get_color_frame();
    Mat color_image(Size(this->frame_width,
                         this->frame_height), CV_8UC3, (void *) color_frame.get_data(), Mat::AUTO_STEP);
    *dest = color_image;
}

void camera_realsense_utils::stop_camera_stream() {
    this->camera_stop_flag = true;
    this->pipe.stop();
}

