#include <iostream>
#include <opencv2/opencv.hpp>
#include <dirent.h>
#include "video_convert.h"
#include "video_encode.h"
#include "camera_realsense_utils.h"
#include "camera_builtin_utils.h"

extern "C" {
#include <libavutil/opt.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}

#define MAX_PATH_SIZE 200
#define MAX_FILE_NUMBER 200

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
           "                            2 : Realsense\n"
           "                            3 : Logitech\n"
           "                            4 : From File + [Address]\n"
           "                            5 : From Folder + [Address]\n"
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
//    bool from_folder_given = false;
    bool from_folder_given = true;
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

    uint8_t port_num = 2;
    static char video_source_addr[200] = {};
    static char video_folder_addr[200] = {"/home/smartirtaric/Videos/fpsTestVideos/noCompress/"};

    const uint8_t buffer_size = 15;
    cameras camera_select = cameras::BUILT_IN;
    Video_Sources video_source = Video_Sources::FROM_FILE_PATH;

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
                std::cerr
                        << "Both Video Path and Port Num given. Decide Video Path or camera device with related Port Num!\n";
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

                case 5:
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

                    from_folder_given = true;
                    std::strcpy(video_folder_addr, argv[i + 2]);
                    std::cout << "Folder Address Given: " << video_folder_addr << "\n";
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


    if ((cameras::BUILT_IN == camera_select || cameras::FROM_FILE == camera_select) && !from_folder_given) {

        auto encode_obj = new video_encode(codec_select,
                                           &frame,
                                           &frame_width,
                                           &frame_height,
                                           &buffer_size,
                                           &fps,
                                           crf_val_char,
                                           crf_preset_char,
                                           "deneme1.avi");

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

    } else if (from_folder_given) {

        int duration_in_sec = 15;

        char video_extension[10] = {};
        char crf_values_arr[][5] = {"20", "25", "30", "35", "40", "45"};

        size_t total_file_in_folder = 0;

        std::cout << "@Test\t:\tFolder Directory\t:\t" << video_folder_addr << "\n";

        DIR *dir;
        struct dirent *ent;

        // File name array allocation.
        char **all_files_in_folder = (char **) malloc(sizeof(char *) * MAX_FILE_NUMBER);

        // File names allocations one by one.
        for (size_t i = 0; i < MAX_FILE_NUMBER; ++i) {
            all_files_in_folder[i] = (char *) malloc(sizeof(char) * MAX_PATH_SIZE);
            if (!all_files_in_folder[i]) {
                std::cerr << "Insufficient memory!\n";
                exit(EXIT_FAILURE);
            }
        }

        //  Filling file names into the array.
        if ((dir = opendir(video_folder_addr)) != nullptr) {
            size_t file_counter = 0;
            for (size_t i; ((ent = readdir(dir)) != nullptr) && (file_counter <= MAX_FILE_NUMBER); i++) {
                strcpy(all_files_in_folder[file_counter], ent->d_name);
                std::cout << "Path [" << i << "]\t:\t" << all_files_in_folder[file_counter] << "\n";
                file_counter++;
            }

            if (MAX_FILE_NUMBER <= file_counter) { // Warning for folder missing. Check related macro.
                std::cerr << "Max file number is achieved! Some files could not be processed!\n";
            }

            total_file_in_folder = file_counter; // Keep total file counter in this local variable.
            closedir(dir);

        } else {
            /* could not open directory */
            std::cerr << "Error: " << strerror(errno) << " - Can not open directory!\n";
            perror("");
            return EXIT_FAILURE;
        }

        char file_path_chr[MAX_PATH_SIZE] = {};

        std::cout << "@Test\t:\tTotal File Number\t:\t" << total_file_in_folder << "\n";

        size_t crf_list_size = sizeof(crf_values_arr) / sizeof(crf_values_arr[0]);
        std::cout << "@Test\t:\tNumber of CRF values in list\t:\t" << crf_list_size << "\n";

        for (size_t i = 0; i < total_file_in_folder; ++i) { // Folder handler loop
            std::cout << "@Test\t:\tWorking on file\t:\t[" << all_files_in_folder[i] << "]\n";

            uint8_t clip_fps;
            char *fps_addr;
            fps_addr = strstr(all_files_in_folder[i], "fps");
            if (!fps_addr) {
                std::cerr << "@Test\t:\tCould not parse fps val in file name!\n";
                continue;
            }
            clip_fps = (uint8_t) strtol(fps_addr + 3, nullptr, 10);
            std::cout << "@Test\t:\tInput file FPS val\t:\t" << (unsigned) clip_fps << "\n";

            strcpy(file_path_chr, video_folder_addr);
            strcat(file_path_chr, all_files_in_folder[i]);

            int total_frame_folder = duration_in_sec * clip_fps;

            for (int j = 0; j < crf_list_size; ++j) {   // CRF loop

                std::cout << "@Test\t:\tInput File Path\t:\t[" << file_path_chr << "]\n";

                auto folder_encode_obj = new video_encode(codec_select,
                                                          &frame,
                                                          &frame_width,
                                                          &frame_height,
                                                          &buffer_size,
                                                          &clip_fps,
                                                          crf_values_arr[j],
                                                          crf_preset_char,
                                                          all_files_in_folder[i]);

                auto camera_obj = new camera_builtin_utils(&frame_width,
                                                           &frame_height,
                                                           &clip_fps,
                                                           &port_num,
                                                           file_path_chr,
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

                while (count < total_frame_folder) {

                    fflush(stdout);
                    ret = camera_obj->get_rgb_frame(&color_image);
                    if (ret) {
                        total_frames_encoded = count;
                        std::cout << "@Test\t:\tTotal processed frame\t:\t" << (unsigned) total_frames_encoded << "\n";
                        break;
                    }
                    folder_encode_obj->make_writable();
                    cvmat_to_avframe_0(&color_image, &frame);

                    frame->pts = count;
                    folder_encode_obj->encode(frame);
                    count++;

                    cv::imshow("Video", color_image);
                    if (27 == cv::waitKey(1))
                        break;
                }

                /* flush the encoder */
                folder_encode_obj->encode(nullptr);
                folder_encode_obj->release_ffmpeg_tool();
                camera_obj->stop_camera_stream();

                free(camera_obj);
                free(folder_encode_obj);

            } // CRF loop
        } // Folder handler loop

        for (size_t i = 0; i < MAX_FILE_NUMBER; ++i) {
            free(all_files_in_folder[i]);
        }
        free(all_files_in_folder);
        exit(EXIT_SUCCESS);

    } else if (cameras::REALSENSE == camera_select) {
        auto encode_obj = new video_encode(codec_select,
                                           &frame,
                                           &frame_width,
                                           &frame_height,
                                           &buffer_size,
                                           &fps,
                                           crf_val_char,
                                           crf_preset_char,
                                           "deneme1.avi");

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
    }
}
