#include <iostream>
#include <opencv2/opencv.hpp>
#include "video_convert.h"
#include "video_encode.h"
#include "camera_builtin_utils.h"

extern "C" {
#include <libswscale/swscale.h>
}

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

    const uint16_t frame_width = 640;
    const uint16_t frame_height = 480;
    const uint8_t frame_rate = 30;

    uint8_t port_num = 0;
    static char video_source_addr[200] = {};

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

    if (cameras::BUILT_IN == camera_select || cameras::FROM_FILE == camera_select) {

        auto camera_obj = new camera_builtin_utils(&frame_width,
                                                   &frame_height,
                                                   &frame_rate,
                                                   &port_num,
                                                   video_source_addr,
                                                   video_source);

        ret = camera_obj->set_video_capture();
        if (ret) {
            if (CAP_NOT_OPENED_ERROR == ret) {
                exit(EXIT_FAILURE);
            } else if (CAM_CAP_WRONG_FPS_RES_ERROR == ret) {
                std::cerr << "@Main\t\t:\tCamera opened but resolution or fps is not as you wish!\n";
            }
        }

        auto encode_obj = new video_encode(codec_select,
                                           &frame,
                                           &frame_width,
                                           &frame_height,
                                           &frame_rate,
                                           crf_val_char,
                                           crf_preset_char);

        ret = encode_obj->create_file_object();
        if (ret) {
            std::cerr << "@Main\t\t:\tCould not create file object!\n";
            exit(EXIT_FAILURE);
        }

        uint16_t count = 0;
        Mat color_image;

        std::chrono::steady_clock::time_point time_start = std::chrono::steady_clock::now();

        while (count < total_frame) {

            fflush(stdout);
            ret = camera_obj->get_rgb_frame(&color_image);
            if (ret) {
                break;
            }
            encode_obj->make_writable();
            cvmat_to_avframe_0(&color_image, &frame);
            frame->pts = count;
            encode_obj->encode(frame);
            count++;

            cv::imshow("Video", color_image);
            if (27 == cv::waitKey(1))
                break;
        }

        /* flush the encoder */
        encode_obj->encode(nullptr);
        std::cout << "@Main\t\t:\tTotal processed frame\t:\t[" << (unsigned) count << "]\n";

        std::chrono::steady_clock::time_point time_end = std::chrono::steady_clock::now();
        long time_diff = std::chrono::duration_cast<std::chrono::seconds>(time_end - time_start).count();

        std::cout << "@Main\t\t:\tTotal time \t:\t[" << time_diff << "] secs.\n";

        encode_obj->release_ffmpeg_tool();
        camera_obj->stop_camera_stream();
        delete camera_obj;
        delete encode_obj;
        exit(EXIT_SUCCESS);

    }
}
