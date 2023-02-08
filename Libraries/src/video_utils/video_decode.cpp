#include "video_decode.h"

video_decode::video_decode() {

}

video_decode::~video_decode() = default;


void video_decode::pgm_save(unsigned char *buf, int wrap, int xsize, int ysize, char *filename) {
    FILE *f;
    int i;
    f = fopen(filename, "w");
    fprintf(f, "P5\n%d %d\n%d\n", xsize, ysize, 255);
    for (i = 0; i < ysize; i++)
        fwrite(buf + i * wrap, 1, xsize, f);
    fclose(f);
}

void video_decode::decode(AVCodecContext *dec_ctx, AVFrame *frame, AVPacket *pkt,
                          const char *filename) {
    char buf[1024];
    int ret;
    ret = avcodec_send_packet(dec_ctx, pkt);
    if (ret < 0) {
        std::cerr << stderr << "Error sending a packet for decoding!\n";
        exit(EXIT_FAILURE);
    }
    while (true) {
        ret = avcodec_receive_frame(dec_ctx, frame);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
            return;
        else if (ret < 0) {
            std::cerr << stderr << "Error during decoding\n";
            exit(EXIT_FAILURE);
        }
//        std::cout << "saving frame " << dec_ctx->frame_number << "\n";
        fflush(stdout);
        snprintf(buf, sizeof(buf), "%s-%d", filename, dec_ctx->frame_number);
        pgm_save(frame->data[0], frame->linesize[0],
                 frame->width, frame->height, buf);
    }
}
