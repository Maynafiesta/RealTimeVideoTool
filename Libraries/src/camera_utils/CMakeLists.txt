find_package(OpenCV REQUIRED)

if(REALSENSE_SDK)
    find_package(realsense2 REQUIRED)
endif(REALSENSE_SDK)

include_directories(../../include/camera_utils)

if(REALSENSE_SDK)
    add_library(camera_utils camera_realsense_utils.cpp camera_builtin_utils.cpp)
else()
    add_library(camera_utils camera_builtin_utils.cpp)
endif(REALSENSE_SDK)

target_link_libraries(camera_utils opencv_videoio)
target_link_libraries(camera_utils opencv_core)
target_link_libraries(camera_utils ${OpenCV_LIBS})

if(REALSENSE_SDK)
    target_link_libraries(camera_utils  ${realsense2_LIBRARY})
endif(REALSENSE_SDK)
