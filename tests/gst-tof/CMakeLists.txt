set(PLUGIN_TEST_SRCS
    gtest-main.cc common/tofmeta.cc common/tofformat.cc tofparser/tofparser.cc
    raw2depth/raw2depth.cc gtest-main.cc)

set(PLUGIN_TEST_INCLUDES
    ${PROJECT_BINARY_DIR} ${PROJECT_SOURCE_DIR}/lib ${OPENCV_INCLUDE_DIRS}
    ${GST_INCLUDE_DIRS} ${GST_CHECH_INCLUDE_DIRS})

set(PLUGIN_TEST_LINK_LIBS GTest::gtest_main GTest::gmock_main ${GST_LIBRARIES}
                          ${GST_CHECK_LIBRARIES} tofplugin)

set(PLUGIN_TEST_LINK_DIRS ${GST_LIBRARY_DIRS} ${GST_CHECK_LIBRARY_DIRS})

add_executable(plugin_tests ${PLUGIN_TEST_SRCS})

target_include_directories(plugin_tests PUBLIC ${PLUGIN_TEST_INCLUDES})
target_link_directories(plugin_tests PUBLIC ${PLUGIN_TEST_LINK_DIRS})
target_link_libraries(plugin_tests ${PLUGIN_TEST_LINK_LIBS})

gtest_discover_tests(
  plugin_tests
  WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/tests
  PROPERTIES ENVIRONMENT GST_PLUGIN_PATH=${PROJECT_BINARY_DIR}/lib/gst-tof)
