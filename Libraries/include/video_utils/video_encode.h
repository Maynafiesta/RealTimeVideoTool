//
// Created by taric on 16.08.2022.
//

#ifndef FFMPEG_VIDEO_STREAM_ENCODE_VIDEO_ENCODE_H
#define FFMPEG_VIDEO_STREAM_ENCODE_VIDEO_ENCODE_H

#define USER_DATA_START_CODE    0x000001b2
#define SEQUENCE_HEADER_CODE    0x000001b3
//#define SEQUENCE_HEADER_CODE    {0, 0, 1, 0xb3}
#define SEQUENCE_ERROR_CODE     0x000001b4
#define SEQUENCE_END_CODE       0x000001b7

#include <cstring>
#include <vector>
#include <cstdio>
#include "video_errors.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavutil/opt.h>
#include <libswscale/swscale.h>
}

static const char *const crf_preset_list[] = {
        "ultrafast",
        "superfast",
        "veryfast",
        "faster",
        "fast",
        "medium", // – default preset
        "slow",
        "slower",
        "veryslow"};

enum codecs {
    MPEG = AV_CODEC_ID_MPEG2VIDEO,
    H264 = AV_CODEC_ID_H264,
};


class video_encode {
public:
    video_encode(codecs codec_obj,
                 AVFrame **frame_obj_param,
                 const uint16_t *frame_width_param,
                 const uint16_t *frame_height_param,
                 const uint8_t *frame_buffer_size_param,
                 const uint8_t *frame_rate_param,
                 const char *crf_val_param,
                 const char *crf_preset_param);

    ~video_encode();

    void encode(AVFrame *);

    void update_full_path_name();

    void set_video_extension(codecs);

    void set_avcontext();

    void set_avframe();

    void add_head_code();

    void add_end_code();

    void create_file_object();

    void release_video_data();

    void release_ffmpeg_tool();

    void make_writable();

    void frame_alloc();

private:
    const char *folder_path;
    const char *file_name;
    const char *file_extension;
    char full_file_path[50];
    char *crf_val_char;
    uint8_t crf_preset_val;
    codecs codec_val;
    uint16_t file_counter;
    uint16_t frame_width;
    uint16_t frame_height;
    uint8_t fps;
    uint8_t buffer_size;
    uint16_t frame_counter;
    FILE *file_obj;
    bool file_obj_status;
    const AVCodec *codec;
    AVFrame **frame_obj;
    AVCodecContext *codec_ctx_obj;
    AVPacket *packet_obj;
    AVCodecID codec_id_obj;

};


#endif //FFMPEG_VIDEO_STREAM_ENCODE_VIDEO_ENCODE_H
