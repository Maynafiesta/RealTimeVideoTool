#ifndef SMIRVIDEOTOOL_CAMERA_REALSENSE_UTILS_H
#define SMIRVIDEOTOOL_CAMERA_REALSENSE_UTILS_H

#include <librealsense2/rs.hpp>
#include <opencv2/opencv.hpp>
#include <iostream>

class camera_realsense_utils {
public:
    camera_realsense_utils(const uint16_t *,
                           const uint16_t *,
                           const uint8_t *);

    ~camera_realsense_utils();

    void stop_camera_stream();

    void get_rgb_frame(cv::Mat *dest);

private:
    uint16_t frame_width;
    uint16_t frame_height;
    uint8_t fps;
    rs2::pipeline pipe;
    rs2::config cfg;
    rs2::frameset frame;
    rs2::frame color_frame;
    bool camera_stop_flag;

};

#endif //SMIRVIDEOTOOL_CAMERA_REALSENSE_UTILS_H
