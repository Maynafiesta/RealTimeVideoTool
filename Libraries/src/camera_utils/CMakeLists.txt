find_package(OpenCV REQUIRED)
find_package(realsense2 REQUIRED)

include_directories(../../include/camera_utils)

add_library(camera_utils camera_builtin_utils.cpp camera_realsense_utils.cpp)

target_link_libraries(camera_utils opencv_videoio)
target_link_libraries(camera_utils opencv_core)
target_link_libraries(camera_utils ${OpenCV_LIBS})
target_link_libraries(camera_utils  ${realsense2_LIBRARY})
