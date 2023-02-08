#ifndef SMIRVIDEOTOOL_VIDEO_CONVERT_H
#define SMIRVIDEOTOOL_VIDEO_CONVERT_H

#include <opencv2/opencv.hpp>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
}

AVFrame *cvmat_to_avframe_0(cv::Mat *, AVFrame **);
//AVFrame *cvmat_to_avframe_0(cv::Mat *, AVFrame *);

#endif //SMIRVIDEOTOOL_VIDEO_CONVERT_H
