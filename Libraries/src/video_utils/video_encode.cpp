#include "video_encode.h"
#include <iostream>
#include <sys/stat.h>

#define MAX_FILE_NUMBER 400

static const char *const mpeg_extention = ".mp4";
static const char *const h264_extention = ".mkv";

video_encode::video_encode(codecs codec_obj,
                           AVFrame **frame_obj_param,
                           const uint16_t *frame_width_param,
                           const uint16_t *frame_height_param,
                           const uint8_t *frame_rate_param,
                           const char *crf_val_param,
                           const char *crf_preset_param,
                           const char *file_name_param) {

    folder_path = "VideoClips/";
    file_name = (char*)file_name_param;
//    strcpy(this->file_name, file_name_param);
    file_extension = "";
    codec_val = codec_obj;
    frame_width = *frame_width_param;
    frame_height = *frame_height_param;
    fps = *frame_rate_param;
    file_counter = 0;
    file_obj_status = false;
    frame_obj = frame_obj_param;
    crf_val_char = (char *) crf_val_param;
    crf_preset_val = (uint8_t) strtol(crf_preset_param, nullptr, 10);

    set_video_extension(codec_obj);
    set_avcontext();
    set_avframe();
}

void video_encode::encode(AVFrame *frame_param) {

//    printf("Send frame %3" PRId64 "\n", (*frame_obj)->pts);
    int ret = avcodec_send_frame(codec_ctx_obj, frame_param);
    if (AVERROR(EAGAIN) == ret || AVERROR_EOF == ret)
        std::cerr << "Error sending a frame for encoding!\n";
    if (ret < 0) {
        std::cerr << stderr << "Error sending a frame for encoding!\n";
        exit(EXIT_FAILURE);
    }
    while (true) {
        ret = avcodec_receive_packet(codec_ctx_obj, packet_obj);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
            return;
        else if (ret < 0) {
            std::cerr << stderr << "Error during encoding!\n";
            exit(EXIT_FAILURE);
        }

//        printf("Write packet %3" PRId64" (size=%5d)\n", (packet_obj)->pts, (packet_obj)->size);
        fwrite(packet_obj->data, 1, packet_obj->size, file_obj);
        av_packet_unref(packet_obj);
    }
}

video_encode::~video_encode() = default;

void video_encode::frame_alloc() {
    *frame_obj = av_frame_alloc();
    if (!*frame_obj) {
        std::cerr << "Could not allocate video frame!\n";
        exit(EXIT_FAILURE);
    }
}

void video_encode::set_avframe() {
    frame_alloc();
    int my_qscale = 31;
    (*frame_obj)->format = codec_ctx_obj->pix_fmt;
    (*frame_obj)->width = codec_ctx_obj->width;
    (*frame_obj)->height = codec_ctx_obj->height;
    (*frame_obj)->flags |= codec_ctx_obj->flags;
    (*frame_obj)->sample_rate = codec_ctx_obj->sample_rate;
    if (MPEG == codec_val)
        (*frame_obj)->quality = FF_QP2LAMBDA * my_qscale;

    int ret = av_frame_get_buffer(*frame_obj, 0);
    if (ret < 0) {
        std::cerr << "Could not allocate the video frame data\n";
        exit(EXIT_FAILURE);
    }
}

void video_encode::set_avcontext() {
    codec_ctx_obj = nullptr;
    codec_id_obj = static_cast<AVCodecID>(codec_val);
    codec = avcodec_find_encoder(codec_id_obj);
    if (!codec) {
        std::cerr << stderr << " - Codec " << (unsigned) codec_id_obj << " not found!\n";
        exit(EXIT_FAILURE);
    }
    codec_ctx_obj = avcodec_alloc_context3(codec);
    if (!codec_ctx_obj) {
        std::cerr << "Could not allocate video codec context!\n";
        exit(EXIT_FAILURE);
    }

    packet_obj = av_packet_alloc();
    if (!packet_obj) {
        std::cerr << "Could not allocate packet object!\n";
        exit(EXIT_FAILURE);
    }
    if (MPEG == codec_val) {
        codec_ctx_obj->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
        codec_ctx_obj->flags |= AV_CODEC_FLAG_QSCALE;
    }

    if (codec->id == AV_CODEC_ID_H264) {
        std::cout << "@Encoder\t:\tcrf val\t:\t" << crf_val_char << "\n";
        std::cout << "@Encoder\t:\tcrf pre\t:\t" << crf_preset_list[crf_preset_val] << "\n";
        av_opt_set(codec_ctx_obj->priv_data, "preset", crf_preset_list[crf_preset_val], 0);
        av_opt_set(codec_ctx_obj->priv_data, "crf", crf_val_char, 0);
    }
//    codec_ctx_obj->bit_rate = 8000000;      //  7372800
    codec_ctx_obj->width = frame_width;
    codec_ctx_obj->height = frame_height;
    codec_ctx_obj->time_base = (AVRational) {1, fps};
    codec_ctx_obj->framerate = (AVRational) {fps, 1};
    codec_ctx_obj->gop_size = 3;
    codec_ctx_obj->max_b_frames = 1;
    codec_ctx_obj->pix_fmt = AV_PIX_FMT_YUV420P;

    int ret = avcodec_open2(codec_ctx_obj, codec, nullptr);
    if (ret < 0) {
        std::cerr << stderr << " - Could not open codec!\n";
        exit(EXIT_FAILURE);
    }
}

void video_encode::release_ffmpeg_tool() {
    fclose(file_obj);
    file_obj_status = false;
    avcodec_free_context(&codec_ctx_obj);
    av_frame_free(frame_obj);
    av_packet_free(&packet_obj);
}

size_t video_encode::create_file_object() {
    if (file_obj_status) {
        std::cerr << "@Encoder\t:\tAnother FILE* object is already created. "
                     "To avoid leakage and data loss, please first release it!\n";
        return FILE_ALREADY_OPEN;
    }
    short local_file_counter_increment = 0;
    static char file_open_mode[] = "wx";

    int ret = mkdir(folder_path, 0777);
    if (ret && EEXIST != errno) {
        std::cerr << "@Encoder\t:\tError str\t:\t" << strerror(errno) << "\n";
        std::cerr << "@Encoder\t:\tCreation of related top folder \t:\t[" << folder_path << "] is failed\n";
        exit(EXIT_FAILURE);
    }
    bool file_exists_warning_writen_flag = false;
    while (local_file_counter_increment < MAX_FILE_NUMBER) {
        update_full_path_name();

        file_obj = fopen(full_file_path, file_open_mode);
        if (!file_obj) {
            // Note to myself, if file not created, no need to call fclose().
            if (EEXIST == errno) {
                if (!file_exists_warning_writen_flag) {
                    std::cerr << "@Encoder\t:\tFile exists, retrying creating file!\n";
                    file_exists_warning_writen_flag = true;
                }
                local_file_counter_increment++;
                continue;
            }
            std::cerr << "@Encoder\t:\tFailed to create file!\n"
                      << "@Encoder\t:\tError desc.\t:\t[" << strerror(errno) << "]\n";
            return FILE_COULD_NOT_OPEN;
        }
        std::cout << "@Encoder\t:\tFile created\t:\t[" << full_file_path << "]\n";
        file_obj_status = true;
        return SUCCESS_VAL;  // Successfully file created.
    }

    std::cerr << "@Encoder\t:\tMaximum file counter ["
              << local_file_counter_increment << "] achieved. Handle storage issue first!\n";
    return MAX_FILE_COUNTER_ACHIEVED;
}

void video_encode::make_writable() {
    int writable_val = av_frame_make_writable(*frame_obj);
    if (writable_val < 0) {
        std::cerr << stderr << "Could not make writable av frame!\n";
        exit(EXIT_FAILURE);
    }
}

void video_encode::update_full_path_name() {
    char file_counter_char[10];
    char file_fps_char[10];
    strcpy(full_file_path, folder_path);
    strcat(full_file_path, file_name);
    strcat(full_file_path, "_Crf_");
    strcat(full_file_path, crf_val_char);
    strcat(full_file_path, "Fps_");
    sprintf(file_fps_char, "%d", fps);
    strcat(full_file_path, file_fps_char);
    file_counter += 1;
    sprintf(file_counter_char, "_%d", file_counter);
    strcat(full_file_path, file_counter_char);
    strcat(full_file_path, file_extension);
}

void video_encode::set_video_extension(codecs codec_obj) {
    if (codec_obj == MPEG) {
        file_extension = mpeg_extention;
    } else if (codec_obj == H264) {
        file_extension = h264_extention;
    } else {
        std::cerr << "Only MPEG and H264 are implemented yet...!!!\n";
        exit(EXIT_FAILURE);
    }
}
