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

# gstreamer - To be removed!!
find_package(PkgConfig REQUIRED)
pkg_check_modules(GST REQUIRED gstreamer-1.0)
if(GST_NOTFOUND)
  message(FATAL "gstreamer not found")
endif()
pkg_check_modules(GST_BASE REQUIRED gstreamer-base-1.0)
if(GST_BASE_NOTFOUND)
  message(FATAL "gstreamer base not found")
endif()
pkg_check_modules(GST_CHECK REQUIRED gstreamer-check-1.0)
if(GST_CHECK_NOTFOUND)
  message(FATAL "gstreamer check not found")
endif()

# put this out here so vscode can find it
enable_testing()

add_subdirectory(lib)
add_subdirectory(tests)
