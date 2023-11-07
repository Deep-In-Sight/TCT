set(IMGUI_DIR ${CMAKE_SOURCE_DIR}/3rdparty/imgui)
set(IMGUI_SRCS
    ${IMGUI_DIR}/imgui.cpp ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/imgui_draw.cpp ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp)
set(IMGUI_INCLUDE_DIRS ${IMGUI_DIR} ${IMGUI_DIR}/backends)

set(TCT_DIR ${CMAKE_SOURCE_DIR}/apps/tct-gui)
set(TCT_SRCS ${TCT_DIR}/graphics-item.cc ${TCT_DIR}/transform.cc
             ${TCT_DIR}/graphics-item-impl.cc ${TCT_DIR}/utility.cc)

set(TCT_TEST_SRCS gtest-main.cc graphics-item.cc transform.cc ${TCT_SRCS}
                  ${IMGUI_SRCS})

set(TCT_TEST_INCLUDES ${PROJECT_BINARY_DIR} ${PROJECT_SOURCE_DIR}/apps/tct-gui
                      ${OPENCV_INCLUDE_DIRS} ${IMGUI_INCLUDE_DIRS})

set(TCT_TEST_LINK_LIBS GTest::gtest_main GTest::gmock_main ${OpenCV_LIBS} glfw
                       GL)

if(DEFINED TCT_SRCS)
  message(STATUS "TCT_SRCS: ${TCT_SRCS}")
else()
  message(STATUS "TCT_SRCS: NOT DEFINED")
endif()

add_executable(tct_tests ${TCT_TEST_SRCS} ${TCT_SRCS})

target_include_directories(tct_tests PUBLIC ${TCT_TEST_INCLUDES})
target_link_libraries(tct_tests ${TCT_TEST_LINK_LIBS})

gtest_discover_tests(tct_tests WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/tests)
