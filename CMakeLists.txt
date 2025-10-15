cmake_minimum_required(VERSION 3.15)
project(AR_A4_Video LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenCV REQUIRED COMPONENTS core imgproc highgui calib3d)
find_package(PkgConfig REQUIRED)
pkg_check_modules(GLFW REQUIRED glfw3)
find_package(GLEW REQUIRED)

# GLM is header-only; if not found via package, vendor it or add include dir
find_path(GLM_INCLUDE_DIR glm/glm.hpp)
if(NOT GLM_INCLUDE_DIR)
  message(FATAL_ERROR "GLM not found. Install libglm-dev or provide include path.")
endif()

include_directories(
  ${OpenCV_INCLUDE_DIRS}
  ${GLEW_INCLUDE_DIRS}
  ${GLFW_INCLUDE_DIRS}
  ${GLM_INCLUDE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_executable(AR_A4_Video
  src/main.cpp
  src/ar/calib.cpp
  src/ar/pose.cpp
  src/detect/a4.cpp
  src/glx/mesh.cpp
  src/glx/shaders.cpp
  src/glx/texture.cpp
)

target_link_libraries(AR_A4_Video
  ${OpenCV_LIBS}
  GLEW::GLEW
  ${GLFW_LINK_LIBRARIES}
  GL
)

# On some systems, GLFW is a pkg-config-only dep; fallback to its libs
if (NOT GLFW_LINK_LIBRARIES)
  target_link_libraries(AR_A4_Video glfw)
endif()
