#include <iostream>
#include <opencv2/opencv.hpp>
#include "video_convert.h"
#include "video_encode.h"
#include "camera_builtin_utils.h"

#ifdef REALSENSE_SDK_SMIR
#include "camera_realsense_utils.h"
#endif

extern "C" {
#include <libavutil/opt.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}

#define DEFAULT_VIDEO_PATH "~/HRSampleVideos/3resting.avi_1.mp4"

enum cameras {
    BUILT_IN = 1,
    REALSENSE = 2,
    FROM_FILE = 3,
};

using namespace cv;

void print_usage(char *cmd) {
    char *cmd_name = basename(cmd);
    printf("Usage: %s [OPTION][OPTION]...\n"
           " -h (--help)            Display this help and exit\n"
           " -cs x (--cameraselect) Camera select\n"
           "                            1 : Builtin [default]\n"
           "                            2 : Realsense -> Requires compile configuration. See README.md\n"
           "                            3 : Logitech\n"
           "                            4 : From File + [Address]\n"
           " -p x (--port)          Select port number\n"
           "                            0 : [default]\n"
           " -cv x (--crfvalue)     Crf value, (0 to 51, 0 -> lossless)\n"
           "                            8 : [default]\n"
           " -cp x (--crfpreset)    Crf preset\n"
           "                            0 : ultrafast\n"
           "                            1 : superfast\n"
           "                            2 : veryfast\n"
           "                            3 : faster\n"
           "                            4 : fast\n"
           "                            5 : medium [default]\n"
           "                            6 : slow\n"
           "                            7 : slower\n"
           "                            8 : veryslow\n"
           "", cmd_name);
}

int main(int argc, char *argv[]) {
    size_t ret;
    bool port_num_given = false;
    bool video_path_given = false;
    bool camera_select_param_given = false;
    /*
     * todo argument alirken isnum kontrolu ekle
     */

    char crf_preset_char[5] = "5";
    char crf_val_char[3] = "8";

    unsigned long total_frames_encoded = 0;
//    const uint16_t frame_width = 1920;
//    const uint16_t frame_height = 1080;
    const uint16_t frame_width = 640;
    const uint16_t frame_height = 480;
    const uint8_t fps = 30;

    uint8_t port_num = 0;
    static char video_source_addr[200] = {};

    const uint8_t buffer_size = 15;
    cameras camera_select = cameras::BUILT_IN;
    Video_Sources video_source = Video_Sources::BUILT_IN_CAMERA;

    codecs codec_select = H264;
    AVFrame *frame;
    size_t total_frame = 450;

    // Argument parsing
    for (int i = 1; i < argc; ++i) {

        if ((0 == strcmp(argv[i], "-h")) || (0 == strcmp(argv[i], "--help"))) {
            print_usage(argv[0]);
            exit(EXIT_SUCCESS);
        } else if (((0 == strcmp(argv[i], "-p")) || (0 == strcmp(argv[i], "--port"))) && (i + 1 != argc)) {

            if (video_path_given) {
                std::cerr << "Both Video Path and Port Num given. "
                             "Decide Video Path or Camera Device with related Port Num!\n";
                exit(EXIT_FAILURE);
            }
            port_num = (uint8_t) strtol(argv[i + 1], nullptr, 10);
            port_num_given = true;
            i++;

        } else if (((0 == strcmp(argv[i], "-cv")) || (0 == strcmp(argv[i], "--crfvalue"))) && (i + 1 != argc)) {
            /*
             * todo kontrol ekle
             */
            strcpy(crf_val_char, argv[i + 1]);
            i++;
        } else if (((0 == strcmp(argv[i], "-cp")) || (0 == strcmp(argv[i], "--crfpreset"))) && (i + 1 != argc)) {
            /*
             * todo kontrol ekle
             */
            strcpy(crf_preset_char, argv[i + 1]);
            i++;
        } else if (((0 == strcmp(argv[i], "-cs")) || (0 == strcmp(argv[i], "--cameraselect"))) && (i + 1 != argc)) {

            if (camera_select_param_given) { // Check for double -cs param given.
                std::cerr << "Double -cs param detected. Its not acceptable!\n";
                exit(EXIT_FAILURE);
            }

            switch (std::stoi(argv[i + 1])) {
                case 1:
                    camera_select = cameras::BUILT_IN;
                    video_source = Video_Sources::BUILT_IN_CAMERA;
                    i++;
                    break;

                case 2:
                    camera_select = cameras::REALSENSE;
#ifndef REALSENSE_SDK_SMIR
                    std::cerr << "Project is not compiled with Realsense support! Check README.md.\n"
                                 "If you just want to get image through OpenCV use -p with related port num.\n"
                                 "This option requires access to Realsense SDK.\n";
                    exit(EXIT_FAILURE);
#endif
                    i++;
                    break;

                case 3:
                    camera_select = cameras::BUILT_IN;
                    video_source = Video_Sources::LOGITECH_CAMERA;
                    i++;
                    break;

                case 4:
                    camera_select = FROM_FILE;
                    video_source = Video_Sources::FROM_FILE_PATH;

                    if (i + 2 == argc || argv[i + 2][0] != '/') {
                        std::cerr << "Enter a valid video path!\n";
                        exit(EXIT_FAILURE);
                    }

                    if (port_num_given) {
                        std::cerr
                                << "Both Video Path and Port Num given. Decide video path or camera device with related port num.\n";
                        exit(EXIT_FAILURE);
                    }

                    video_path_given = true;
                    std::strcpy(video_source_addr, argv[i + 2]);
                    std::cout << "File Address Given: " << video_source_addr << "\n";
                    i++;
                    i++;
                    break;

                default:
                    std::cerr << "Invalid camera index!\n";
                    exit(EXIT_FAILURE);
            }
            camera_select_param_given = true; // Check for double -cs param given.
        } else {
            std::cerr << "Invalid argument, for help '-h' !\n";
            exit(EXIT_FAILURE);
        }
    }

    auto encode_obj = new video_encode(codec_select,
                                       &frame,
                                       &frame_width,
                                       &frame_height,
                                       &buffer_size,
                                       &fps,
                                       crf_val_char,
                                       crf_preset_char);

    if (cameras::BUILT_IN == camera_select || cameras::FROM_FILE == camera_select) {
        auto camera_obj = new camera_builtin_utils(&frame_width,
                                                   &frame_height,
                                                   &fps,
                                                   &port_num,
                                                   video_source_addr,
                                                   video_source);

        ret = camera_obj->set_video_capture();
        if (SUCCESS_VAL != ret) {
            if (CAP_NOT_OPENED_ERROR == ret) {
                exit(EXIT_FAILURE);
            }
            // Here represents the capture opened but with wrong resolution or fps.
        }
        uint16_t count = 0;
        Mat color_image;

        while (count < total_frame) {

            fflush(stdout);
            ret = camera_obj->get_rgb_frame(&color_image);
            if (ret) {
                total_frames_encoded = count;
                std::cout << "Total processed frame: " << (unsigned) total_frames_encoded << "\n";
                break;
            }
            encode_obj->make_writable();
            cvmat_to_avframe_0(&color_image, &frame);

            frame->pts = count;
            encode_obj->encode(frame);
            count++;

            cv::imshow("Video", color_image);
            if (27 == cv::waitKey(27))
                break;
        }

        /* flush the encoder */
        encode_obj->encode(nullptr);
        encode_obj->release_ffmpeg_tool();
        camera_obj->stop_camera_stream();
        delete camera_obj;
        delete encode_obj;
        exit(EXIT_SUCCESS);
#ifdef REALSENSE_SDK_SMIR
        } else if (cameras::REALSENSE == camera_select) {
            auto camera_obj = new camera_realsense_utils(&frame_width,
                                                         &frame_height,
                                                         &fps);
            uint16_t count = 0;
            Mat color_image;
            while (count++ < buffer_size) {
                fflush(stdout);
                camera_obj->get_rgb_frame(&color_image);
                encode_obj->make_writable();
                cvmat_to_avframe_0(&color_image, &frame);
                cv::imshow("Realsense Video", color_image);
                waitKey(25);
                frame->pts = count;
                encode_obj->encode(frame);
            }
            /* flush the encoder */
            encode_obj->encode(nullptr);
            encode_obj->release_ffmpeg_tool();
            camera_obj->stop_camera_stream();
            delete camera_obj;
            delete encode_obj;
            return 0;
#endif // REALSENSE_SDK_SMIR
    }
}
