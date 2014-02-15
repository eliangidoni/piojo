if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(PIOJO_DEBUG ON)
endif(CMAKE_BUILD_TYPE STREQUAL "Debug")

configure_file ("${PROJECT_SOURCE_DIR}/include/piojo_config.h.in"
                "${PROJECT_BINARY_DIR}/include/piojo/piojo_config.h")

set(piojo_SOURCES
    piojo.c
    piojo_alloc.c
    piojo_array.c
    piojo_bitset.c
    piojo_buffer.c
    piojo_diset.c
    piojo_graph.c
    piojo_hash.c
    piojo_heap.c
    piojo_list.c
    piojo_queue.c
    piojo_stack.c
    piojo_tree.c)

include_directories("${PROJECT_SOURCE_DIR}/include")
include_directories("${PROJECT_BINARY_DIR}/include")

add_library(piojo ${piojo_SOURCES})

set_target_properties(piojo PROPERTIES
                      VERSION "${piojo_VERSION_MAJOR}.${piojo_VERSION_MINOR}.${piojo_VERSION_PATCH}"
                      SOVERSION "${piojo_VERSION_MAJOR}")

target_link_libraries(piojo ${EXTRA_LIBS})

install(TARGETS piojo DESTINATION lib)
#install(TARGETS test DESTINATION bin)

install(DIRECTORY "${PROJECT_SOURCE_DIR}/include/piojo" DESTINATION include)
install(DIRECTORY "${PROJECT_BINARY_DIR}/include/piojo" DESTINATION include)
