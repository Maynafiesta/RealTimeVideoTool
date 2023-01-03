#include "video_encode.h"
#include <iostream>
#include <sys/stat.h>

static const char *const mpeg_extention = ".mp4";
static const char *const h264_extention = ".mkv";

uint8_t endcode[] = {0, 0, 1, 0xb7};
uint8_t headcode[] = {0, 0, 1, 0xb3};

video_encode::video_encode(codecs codec_obj,
                           AVFrame **frame_obj_param,
                           const uint16_t *frame_width_param,
                           const uint16_t *frame_height_param,
                           const uint8_t *frame_buffer_size_param,
                           const uint8_t *frame_rate_param,
                           const char *crf_val_param,
                           const char *crf_preset_param) {

    folder_path = "VideoClips/";
    file_name = "video_";
    codec_val = codec_obj;
    frame_width = *frame_width_param;
    frame_height = *frame_height_param;
    buffer_size = *frame_buffer_size_param;
    fps = *frame_rate_param;
    file_counter = 0;
    file_obj_status = false;
    frame_obj = frame_obj_param;
    crf_val_char = (char *) crf_val_param;
    crf_preset_val = (uint8_t) strtol(crf_preset_param, nullptr, 10);

    set_video_extension(codec_obj);
    set_avcontext();
    set_avframe();
    create_file_object();
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
        std::cout << "crf val : " << crf_val_char << "\n";
        std::cout << "crf preset : " << crf_preset_list[crf_preset_val] << "\n";
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

void video_encode::release_video_data() {
}

void video_encode::release_ffmpeg_tool() {
    fclose(file_obj);
    file_obj_status = false;
    avcodec_free_context(&codec_ctx_obj);
    av_frame_free(frame_obj);
    av_packet_free(&packet_obj);
}

void video_encode::create_file_object() {
    update_full_path_name();
    file_obj = fopen(full_file_path, "wb");
    if (!file_obj | file_obj_status) {
        std::cerr << "Could not open. Trying to create related top folder! " << full_file_path << "\n";
        int ret = mkdir(folder_path, 0777);
        if (ret) {
            std::cerr << "Error : " << strerror(errno) << " - Finally folder could not created!\n";
            exit(EXIT_FAILURE);
        }
        file_obj = fopen(full_file_path, "wb");
        if (!file_obj) {
            std::cerr << "Creating folder did not solve the problem!\n";
            exit(EXIT_FAILURE);
        }
    }
    file_obj_status = true;
}

void video_encode::add_head_code() {
    if (!file_obj_status) {
        std::cerr << stderr << "Can not open file!\n";
        exit(EXIT_FAILURE);
    }
    fwrite(headcode, 1, sizeof(headcode), file_obj);
}

void video_encode::add_end_code() {
    if (!file_obj_status) {
        std::cerr << stderr << "Can not open file!\n";
        exit(EXIT_FAILURE);
    }
    fwrite(endcode, 1, sizeof(endcode), file_obj);
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
    strcpy(full_file_path, folder_path);
    strcat(full_file_path, file_name);
//    sprintf(file_counter_char, "%d", file_counter);
//    strcat(full_file_path, file_counter_char);
    strcat(full_file_path, "Crf");
    strcat(full_file_path, crf_val_char);
    strcat(full_file_path, file_extension);
    file_counter += 1;
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
