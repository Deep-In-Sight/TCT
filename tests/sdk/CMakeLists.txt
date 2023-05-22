set(SDK_TEST_SRCS
    gtest-main.cc
    inspector/inspector-queue.cc
    inspector/inspector-scanner.cc
    inspector/inspector-tracker.cc
    # inspector/inspector-histogram.cc
    core/pad.cc
    core/element.cc
    core/bases.cc
    core/queue.cc)

set(SDK_TEST_INCLUDES ${PROJECT_BINARY_DIR} ${PROJECT_SOURCE_DIR}/lib
                      ${OPENCV_INCLUDE_DIRS} ${GST_INCLUDE_DIRS})

set(SDK_TEST_LINK_LIBS
    m
    Threads::Threads
    GTest::gtest_main
    GTest::gmock_main
    ${OpenCV_LIBS}
    ${GST_LIBRARIES}
    sdk)

add_executable(sdk_tests ${SDK_TEST_SRCS})

target_include_directories(sdk_tests PUBLIC ${SDK_TEST_INCLUDES})
target_link_libraries(sdk_tests ${SDK_TEST_LINK_LIBS})

gtest_discover_tests(sdk_tests)
