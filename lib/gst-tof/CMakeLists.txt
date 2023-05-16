set(PLUGIN_SRCS common/tofmeta.cc common/tofformat.cc tofparser/tofparser.cc
                raw2depth/raw2depth.cc common/plugin.cc)

set(PLUGIN_INCLUDES ${PROJECT_BINARY_DIR} ${PROJECT_SOURCE_DIR}/lib
                    ${GST_INCLUDE_DIRS} ${GST_BASE_INCLUDE_DIRS})

set(PLUGIN_LINK_LIBS m Threads::Threads ${GST_LIBRARIES} ${GST_BASE_LIBRARIES})

add_library(tofplugin SHARED ${PLUGIN_SRCS})
target_include_directories(tofplugin PUBLIC ${PLUGIN_INCLUDES})
target_link_libraries(tofplugin ${PLUGIN_LINK_LIBS})
