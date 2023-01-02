//
// Created by taric on 16.08.2022.
//

#ifndef FFMPEG_VIDEO_STREAM_ENCODE_VIDEO_CONVERT_H
#define FFMPEG_VIDEO_STREAM_ENCODE_VIDEO_CONVERT_H

#include <opencv2/opencv.hpp>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
}

AVFrame* cvmat_to_avframe_0(cv::Mat *, AVFrame **);
//AVFrame *cvmat_to_avframe_0(cv::Mat *, AVFrame *);

#endif //FFMPEG_VIDEO_STREAM_ENCODE_VIDEO_CONVERT_H
