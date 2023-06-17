find_package(
  Qt5
  COMPONENTS Core Widgets
  REQUIRED)

set(CMAKE_AUTOMOC ON)

set(APP_SRCS main.cpp PlaybackSourceNode.cpp PlaybackSettingWidget.cpp
             FakeSinkNode.cpp PipelineModel.cpp)

set(APP_HEADERS PlaybackSourceNode.hpp PlaybackSettingWidget.hpp
                FakeSinkNode.hpp PipelineModel.hpp)

add_executable(ek640r ${APP_SRCS} ${APP_HEADERS})

target_link_libraries(ek640r QtNodes sdk)
