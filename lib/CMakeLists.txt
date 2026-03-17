add_library(argparser argparser.cpp memory_management.cpp parsing.cpp validation.cpp)
add_library(nargparse::argparser ALIAS argparser)

target_include_directories(argparser PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})