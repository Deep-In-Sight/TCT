set(SDK_SRCS
    inspector/inspector.cc inspector/inspector-client.cc
    inspector/inspector-scanner.cc inspector/inspector-tracker.cc
    inspector/inspector-histogram.cc inspector/inspector-bitmap.cc)

set(SDK_INCLUDES ${PROJECT_BINARY_DIR} ${PROJECT_SOURCE_DIR}/lib
                 ${GST_INCLUDE_DIRS})

set(SDK_LINK_LIBS m Threads::Threads ${GST_LIBRARIES})

add_library(sdk SHARED ${SDK_SRCS})
target_include_directories(sdk PUBLIC ${SDK_INCLUDES})
target_link_libraries(sdk PUBLIC ${SDK_LINK_LIBS})
