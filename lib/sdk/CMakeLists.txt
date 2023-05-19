set(SDK_SRCS
    inspector/inspector-queue.cc
    # inspector/inspector-client.cc inspector/inspector-scanner.cc
    # inspector/inspector-tracker.cc inspector/inspector-histogram.cc
    # inspector/inspector-bitmap.cc
    core/pad.cc
    core/element.cc
    core/base-src.cc
    core/base-sink.cc
    core/base-transform.cc
    core/queue.cc)

set(SDK_INCLUDES ${PROJECT_BINARY_DIR} ${PROJECT_SOURCE_DIR}/lib
                 ${GST_INCLUDE_DIRS} ${OpenCV_INCLUDE_DIRS})

set(SDK_LINK_LIBS m Threads::Threads ${GST_LIBRARIES} ${OpenCV_LIBRARIES})

add_library(sdk SHARED ${SDK_SRCS})
target_include_directories(sdk PUBLIC ${SDK_INCLUDES})
target_link_libraries(sdk PUBLIC ${SDK_LINK_LIBS})
