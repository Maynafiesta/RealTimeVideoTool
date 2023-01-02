find_package(OpenCV REQUIRED)
find_package(realsense2 REQUIRED)

include_directories(../../include/video_utils)

add_library(video_utils video_encode.cpp video_decode.cpp video_convert.cpp)


target_link_libraries(video_utils avcodec)
target_link_libraries(video_utils avutil)
target_link_libraries(video_utils avdevice)
target_link_libraries(video_utils avfilter)
target_link_libraries(video_utils avformat)
target_link_libraries(video_utils postproc)
target_link_libraries(video_utils swresample)
target_link_libraries(video_utils swscale)

target_link_libraries(video_utils vdpau)
target_link_libraries(video_utils x264)
target_link_libraries(video_utils X11)
target_link_libraries(video_utils va)
target_link_libraries(video_utils z)
target_link_libraries(video_utils va-drm)
target_link_libraries(video_utils va-x11)
target_link_libraries(video_utils lzma)

target_link_libraries(video_utils opencv_videoio)
target_link_libraries(video_utils opencv_core)
target_link_libraries(video_utils ${OpenCV_LIBS})
target_link_libraries(video_utils  ${realsense2_LIBRARY})
