#include "video_convert.h"

/**
 * Function to convert cv::Mat to AvFrame.
 * This function no needs the resolution info. It gets from the cv::Mat object.
 * @param image as cv::Mat*.
 * @param frame as AVFrame**. Make sure it's writable.
 * @return Returns same AVFrame pointer writen on frame parameter. Reason is adding flexibility.
 */
AVFrame *cvmat_to_avframe_0(cv::Mat *image, AVFrame **frame) {
    int frame_width = image->cols;
    int frame_height = image->rows;

    int cv_line_sizes[1];
//    int cv_line_sizes[1] = {3 * frame_width};

    cv_line_sizes[0] = (const int) image->step1();
    if (*frame == nullptr) {
        std::cout << "null frame" << std::endl;
        *frame = av_frame_alloc();
        av_image_alloc((*frame)->data, (*frame)->linesize, frame_width, frame_height, AVPixelFormat::AV_PIX_FMT_YUV420P,
                       1);
    }

    SwsContext *conversion = sws_getContext(frame_width, frame_height,
                                            AVPixelFormat::AV_PIX_FMT_BGR24,
                                            frame_width,
                                            frame_height,
                                            (AVPixelFormat) (*frame)->format,
                                            SWS_FAST_BILINEAR,
                                            NULL, NULL, NULL);

    sws_scale(conversion,
              &image->data,
              cv_line_sizes,
              0,
              frame_height,
              (*frame)->data,
              (*frame)->linesize);

    sws_freeContext(conversion);
    return *frame;
}
