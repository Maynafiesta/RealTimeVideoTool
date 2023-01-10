#ifndef SMIRVIDEOTOOL_VIDEO_ENCODE_H
#define SMIRVIDEOTOOL_VIDEO_ENCODE_H

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
                 const uint8_t *frame_rate_param,
                 const char *crf_val_param,
                 const char *crf_preset_param);

    ~video_encode();

    void encode(AVFrame *);

    void update_full_path_name();

    void set_video_extension(codecs);

    void set_avcontext();

    void set_avframe();

    size_t create_file_object();

    void release_ffmpeg_tool();

    void make_writable();

    void frame_alloc();

    /*
     * MPEG desteği ve container dosya sekli icin bu yapilar eklenebilir.
    void add_head_code();
    void add_end_code();
     */

private:
    const char *folder_path;
    const char *file_name;
    const char *file_extension;
    char full_file_path[50] = {};
    char *crf_val_char;
    uint8_t crf_preset_val;
    codecs codec_val;
    uint16_t file_counter;
    uint16_t frame_width;
    uint16_t frame_height;
    uint8_t fps;
    FILE *file_obj = nullptr;
    bool file_obj_status;
    AVCodec *codec = nullptr;
    AVFrame **frame_obj;
    AVCodecContext *codec_ctx_obj = nullptr;
    AVPacket *packet_obj = nullptr;
    AVCodecID codec_id_obj = AV_CODEC_ID_H264;
};

#endif //SMIRVIDEOTOOL_VIDEO_ENCODE_H
