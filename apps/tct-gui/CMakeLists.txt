# IMGUI source and header files
set(IMGUI_DIR ${CMAKE_SOURCE_DIR}/3rdparty/imgui)
set(IMGUI_SRCS
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    # ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp)
set(IMGUI_INCLUDE_DIRS ${IMGUI_DIR} ${IMGUI_DIR}/backends)

# IMPLOT source and header files
set(IMPLOT_DIR ${CMAKE_SOURCE_DIR}/3rdparty/implot)
set(IMPLOT_SRCS ${IMPLOT_DIR}/implot.cpp ${IMPLOT_DIR}/implot_items.cpp
                ${IMPLOT_DIR}/implot_demo.cpp)
set(IMPLOT_INCLUDE_DIRS ${IMPLOT_DIR})

# IMGUI Node Editor source and header files
set(IMGUI_NODE_DIR ${CMAKE_SOURCE_DIR}/3rdparty/imgui-node-editor)
set(IMGUI_NODE_SRCS
    ${IMGUI_NODE_DIR}/imgui_node_editor.cpp
    ${IMGUI_NODE_DIR}/imgui_node_editor_api.cpp
    ${IMGUI_NODE_DIR}/imgui_canvas.cpp ${IMGUI_NODE_DIR}/crude_json.cpp)
set(IMGUI_NODE_INCLUDE_DIRS ${IMGUI_NODE_DIR})

# TCT source and header files
set(TCT_SRCS
    imgui_impl_glfw.cpp
    main.cc
    application.cc
    window.cc
    node-editor.cc
    inspector-bitmap-view.cc
    utility.cc
    graphics-item.cc
    graphics-layout.cc
    graphics-item-impl.cc
    graphics-view.cc
    transform.cc
    inspector-graphics-view.cc
    inspector-plot-view.cc)

add_executable(tct)

target_sources(tct PRIVATE ${TCT_SRCS} ${IMGUI_SRCS} ${IMGUI_NODE_SRCS}
                           ${IMPLOT_SRCS})
target_link_libraries(
  tct
  sdk
  nlohmann_json::nlohmann_json
  glfw
  dl
  GL
  ${OpenCV_LIBRARIES})
target_include_directories(
  tct
  PRIVATE ${IMGUI_INCLUDE_DIRS} ${IMGUI_NODE_INCLUDE_DIRS}
          ${IMPLOT_INCLUDE_DIRS} ${OpenCV_INCLUDE_DIRS}
          ${PROJECT_SOURCE_DIR}/lib)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/data
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/)
