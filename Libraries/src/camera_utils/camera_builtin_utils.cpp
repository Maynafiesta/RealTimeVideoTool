#include "camera_builtin_utils.h"

static char logitech_camera_full_path[] = "/dev/v4l/by-id/usb-046d_HD_Pro_Webcam_C920_E24A993F-video-index0";

camera_builtin_utils::camera_builtin_utils(const uint16_t *frame_width_param,
                                           const uint16_t *frame_height_param,
                                           const uint8_t *fps_param,
                                           const uint8_t *port_num_param,
                                           const char *source_addr_param,
                                           Video_Sources video_source_param) {

    this->camera_work_flag = true;
    this->file_addr = (char *) source_addr_param;
    this->video_source = video_source_param;
    this->port_num = *port_num_param;
    this->frame_height = *frame_height_param;
    this->frame_width = *frame_width_param;
    this->frame_rate = *fps_param;
}

camera_builtin_utils::~camera_builtin_utils() = default;

size_t camera_builtin_utils::set_video_capture() {
    switch (this->video_source) {
        case BUILT_IN_CAMERA:
            this->cap = cv::VideoCapture(this->port_num, cv::CAP_V4L2);
            break;

        case FROM_FILE_PATH:
            this->cap = cv::VideoCapture(this->file_addr);
            break;

        case LOGITECH_CAMERA:
            this->cap = cv::VideoCapture(logitech_camera_full_path, cv::CAP_V4L2);
            break;
    }

    this->cap.set(cv::CAP_PROP_AUTO_EXPOSURE, 1);
    this->cap.set(cv::CAP_PROP_FPS, this->frame_rate);
    if (!this->cap.isOpened()) {
        std::cerr << "Can not Open Video Capture!\n";
        return CAM_CAP_NOT_OPENED_ERROR;
    }

    if (this->check_resolution_fps()) {
        return CAM_CAP_WRONG_FPS_RES_ERROR;
    }
    return CAM_SUCCESS_VAL;
}

void camera_builtin_utils::stop_camera_stream() {
    this->camera_work_flag = false;
    this->cap.release();
}

/**
 * Function to get RGB frame with OpenCV.
 * @param dest Destination address to write cv::Mat
 */
size_t camera_builtin_utils::get_rgb_frame(cv::Mat *dest) {
    this->cap >> *dest;
    if ((*dest).empty()) {
        std::cerr << "Empty frame!\n";
        return CAM_EMPTY_FRAME_ERROR;
    }
//    (*dest).copyTo(*dest);
    return CAM_SUCCESS_VAL;
}

/**
 * Check and set the resolution and fps.
 * @param frame_width_param
 * @param frame_height_param
 * @param fps_param
 * @return true if smt. wrong. false if conversion is done or not applied.
 */

static bool resize_failure_flag = false;

size_t camera_builtin_utils::check_resolution_fps() {

    double current_frame_width = this->cap.get(cv::CAP_PROP_FRAME_WIDTH);
    double current_frame_height = this->cap.get(cv::CAP_PROP_FRAME_HEIGHT);
    double current_fps = this->cap.get(cv::CAP_PROP_FPS);

    if (current_frame_width != this->frame_width) {
        std::cerr << "Frame width will be changed.\n";
        this->cap.set(cv::CAP_PROP_FRAME_WIDTH, this->frame_width);
        double reseted_frame_width = this->cap.get(cv::CAP_PROP_FRAME_WIDTH);
        if (reseted_frame_width != this->frame_width) {
            std::cerr << "Frame width can not be changed to " << (unsigned) this->frame_width <<
                      ". Current width " << reseted_frame_width << "\n";
            resize_failure_flag = true;
        }
    }

    if (current_frame_height != this->frame_height) {
        std::cerr << "Frame height will be changed.\n";
        this->cap.set(cv::CAP_PROP_FRAME_HEIGHT, this->frame_height);
        double reseted_frame_height = this->cap.get(cv::CAP_PROP_FRAME_HEIGHT);
        if (reseted_frame_height != this->frame_height) {
            std::cerr << "Frame height can not be changed to " << (unsigned) this->frame_height <<
                      ". Current height " << reseted_frame_height << "\n";
            resize_failure_flag = true;
        }
    }
    if (current_fps != this->frame_rate) {
        std::cerr << "FPS will be changed.\n";
        this->cap.set(cv::CAP_PROP_FPS, this->frame_rate);
        double reseted_fps = this->cap.get(cv::CAP_PROP_FPS);
        if (reseted_fps != this->frame_rate) {
            std::cerr << "FPS can not be changed to " << (unsigned) this->frame_rate <<
                      ". Current FPS " << reseted_fps << "\n";
            resize_failure_flag = true;
        }
    }
    return resize_failure_flag;
}

uint16_t camera_builtin_utils::get_camera_frame_height() {
    return (uint8_t) this->cap.get(cv::CAP_PROP_FRAME_HEIGHT);
}

uint16_t camera_builtin_utils::get_camera_frame_width() {
    return (uint8_t) this->cap.get(cv::CAP_PROP_FRAME_WIDTH);
}

uint8_t camera_builtin_utils::get_camera_frame_rate() {
    return (uint8_t) this->cap.get(cv::CAP_PROP_FPS);
}

