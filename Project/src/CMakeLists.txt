include_directories(../../Libraries/include/video_utils)
include_directories(../../Libraries/include/camera_utils)
include_directories(../../Libraries/include/meta_data_utils)

link_directories(../../Libraries/src)

set(EXECUTABLE_OUTPUT_PATH ../../)

if (REALSENSE_SDK)
    add_compile_definitions(REALSENSE_SDK_SMIR)
endif (REALSENSE_SDK)


add_executable(${PROJECT_NAME} main.cpp)

target_link_libraries(${PROJECT_NAME}  camera_utils)
target_link_libraries(${PROJECT_NAME}  video_utils)
target_link_libraries(${PROJECT_NAME} meta_data_utils)

