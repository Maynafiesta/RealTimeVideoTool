#ifndef FFMPEG_VIDEO_STREAM_ENCODE_CAMERA_BUILTIN_UTILS_H
#define FFMPEG_VIDEO_STREAM_ENCODE_CAMERA_BUILTIN_UTILS_H

#include <opencv4/opencv2/opencv.hpp>
#include <iostream>
#include "camera_errors.h"

typedef enum {
    BUILT_IN_CAMERA = 1,
    FROM_FILE_PATH = 2,
    LOGITECH_CAMERA = 3,
} Video_Sources;

class camera_builtin_utils {
public:
    camera_builtin_utils(const uint16_t *frame_width_param,
                         const uint16_t *frame_height_param,
                         const uint8_t *fps_param,
                         const uint8_t *port_num_param,
                         const char *source_addr_param,
                         Video_Sources video_source_param);

    ~camera_builtin_utils();

    void stop_camera_stream();

    size_t get_rgb_frame(cv::Mat *dest);
    uint16_t get_camera_frame_width();
    uint16_t get_camera_frame_height();
    uint8_t get_camera_frame_rate();

    size_t set_video_capture();

    size_t check_resolution_fps();

    cv::VideoCapture cap;
    uint8_t port_num;
    uint16_t frame_width;
    uint16_t frame_height;
    uint8_t frame_rate;
    bool camera_work_flag;
    char *file_addr;
    Video_Sources video_source;
};

#endif //FFMPEG_VIDEO_STREAM_ENCODE_CAMERA_BUILTIN_UTILS_H
