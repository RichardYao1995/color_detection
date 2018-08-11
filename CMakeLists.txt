project(color_detection)
cmake_minimum_required(VERSION 2.8)

find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

include_directories(
  ${catkin_INCLUDE_DIRS}
)

add_executable(color_detection main.cpp)
target_link_libraries(color_detection ${catkin_LIBRARIES}
                                 ${OpenCV_LIBS})

