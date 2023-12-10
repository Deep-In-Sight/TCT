cmake_minimum_required(VERSION 3.5)
project(tct VERSION 0.1)

set(CMAKE_CXX_STANDARD 14)

option(USE_CONFIG_H "Use config.h" ON)
if(USE_CONFIG_H)
  configure_file(config.h.in config.h)
endif()

# threads
find_package(Threads REQUIRED)

# add OpenCV
find_package(OpenCV REQUIRED)
if(OpenCV_NOTFOUND)
  message(FATAL "OpenCV not found")
endif()

# spdlog
find_package(spdlog REQUIRED)
if(spdlog_NOTFOUND)
  message(FATAL "spdlog not found")
endif()

# Open3D
set(Open3D_DIR /home/linh/Open3d/lib/cmake/Open3D)
find_package(Open3D REQUIRED)

# put this out here so vscode can find it
enable_testing()

add_subdirectory(lib)
add_subdirectory(tests)
add_subdirectory(apps)
add_subdirectory(3rdparty)
