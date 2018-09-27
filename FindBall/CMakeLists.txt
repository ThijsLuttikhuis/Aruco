cmake_minimum_required(VERSION 2.8)
project( FindBall )
find_package( OpenCV 3 REQUIRED )

set(CMAKE_CXX_FLAGS "-O3 -Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g -Wall -Wextra")

include_directories( ${OpenCV_INCLUDE_DIRS} )
add_executable( FindBall arucoDetector.cpp)
target_link_libraries( FindBall ${OpenCV_LIBS} )