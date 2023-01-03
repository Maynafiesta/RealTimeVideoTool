#ifndef SMIRVIDEOTOOL_VIDEO_DECODE_H
#define SMIRVIDEOTOOL_VIDEO_DECODE_H

#include <iostream>
#include <string>
#include <cstdlib>

extern "C" {
#include <libavcodec/avcodec.h>
};

class video_decode {
public:
    video_decode();

    ~video_decode();

    static void pgm_save(unsigned char *buf, int wrap, int xsize, int ysize, char *filename);

    static void decode(AVCodecContext *dec_ctx, AVFrame *frame, AVPacket *pkt, const char *filename);

private:
    const AVCodec *codec;
    const char *filename, *out_filename;
    AVCodecParserContext *parser_context;
    AVCodecContext *codec_ctx = nullptr;
    FILE *file;
    AVFrame *av_frame;
    AVPacket *pkt;
};

#endif //SMIRVIDEOTOOL_VIDEO_DECODE_H
