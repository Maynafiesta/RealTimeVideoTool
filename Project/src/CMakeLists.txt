include_directories(../../Libraries/include/video_utils)
include_directories(../../Libraries/include/camera_utils)

link_directories(../../Libraries/src)

set(EXECUTABLE_OUTPUT_PATH ../../)

if (REALSENSE_SDK)
    add_compile_definitions(REALSENSE_SDK_SMIR)
endif (REALSENSE_SDK)

add_executable(smirVideoTool main.cpp)

target_link_libraries(smirVideoTool camera_utils)
target_link_libraries(smirVideoTool video_utils)

