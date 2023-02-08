find_package( OpenCV REQUIRED )

set(CMAKE_CXX_FLAGS -lX11)
#set(CMAKE_CXX_FLAGS -lopenblas)
set(CMAKE_CXX_FLAGS -ggdb)
set(CMAKE_CXX_FLAGS -lpthread)
set(CMAKE_CXX_FLAGS -pthread)


include_directories( ${OpenCV_INCLUDE_DIRS} )
include_directories( /home/smartirtaric/Libraries/dlib-19.24 )
include_directories(../../include/meta_data_utils)
#include_directories( /home/serkan/dlib-19.20/dlib/all)
add_library(meta_data_utils MetaDataExtractor.cpp)

target_link_libraries( meta_data_utils ${OpenCV_LIBS} )
target_link_libraries( meta_data_utils /home/smartirtaric/Libraries/dlib-19.24/build/dlib/libdlib.a)