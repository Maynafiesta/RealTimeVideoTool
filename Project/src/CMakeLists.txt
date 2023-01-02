
include_directories(../../Libraries/include/video_utils)
include_directories(../../Libraries/include/camera_utils)

link_directories(../../Libraries/src)

set(EXECUTABLE_OUTPUT_PATH ../../)
add_executable(smirVideoTool main.cpp)

target_link_libraries(smirVideoTool camera_utils)
target_link_libraries(smirVideoTool video_utils)

