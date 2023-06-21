find_package(
  Qt5
  COMPONENTS Core Widgets
  REQUIRED)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(APP_SRCS
    main.cpp
    PlaybackSourceNode.cpp
    PlaybackSettingWidget.cpp
    CameraSourceNode.cpp
    CameraSourceSettingWidget.cpp
    VideoSinkNode.cpp
    ImageInspectorWidget.cpp
    DepthCalcNode.cpp
    DepthCalcConfigWidget.cpp
    MovingAverageNode.cpp
    FishEyeSettingWidget.cpp
    FishEyeNode.cpp
    PipelineModel.cpp
    NodeBase.cpp
    TopLevelNodeEditorWidget.cpp)

set(APP_HEADERS
    PlaybackSourceNode.hpp
    PlaybackSettingWidget.hpp
    CameraSourceNode.hpp
    CameraSourceSettingWidget.hpp
    VideoSinkNode.hpp
    ImageInspectorWidget.hpp
    DepthCalcNode.hpp
    DepthCalcConfigWidget.hpp
    MovingAverageNode.hpp
    FishEyeSettingWidget.hpp
    FishEyeNode.hpp
    PipelineModel.hpp
    NodeBase.hpp
    ToFDataTypes.hpp
    TopLevelNodeEditorWidget.hpp)

set(RESOURCE_FILES resource/resource.qrc)

add_executable(ek640r ${APP_SRCS} ${APP_HEADERS} ${RESOURCE_FILES})

target_link_libraries(ek640r QtNodes sdk)
