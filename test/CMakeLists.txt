include_directories("${PROJECT_SOURCE_DIR}/include")
include_directories("${PROJECT_SOURCE_DIR}/test")
include_directories("${PROJECT_BINARY_DIR}/include")

FILE(GLOB_RECURSE piojo_TEST_SOURCES ${PROJECT_SOURCE_DIR}/test/piojo_*_test.c)
FILE(GLOB_RECURSE piojo_TEST_SOURCESCPP ${PROJECT_SOURCE_DIR}/test/piojo_*_test.cpp)
set(piojo_TEST_SOURCES ${piojo_TEST_SOURCES} ${piojo_TEST_SOURCESCPP})

foreach(testsource ${piojo_TEST_SOURCES})
  get_filename_component(name ${testsource} NAME_WE)
  add_executable(${name} ${testsource} ${PROJECT_SOURCE_DIR}/test/piojo_test.c)
  target_link_libraries(${name} piojo)
  add_test(${name} ${name})
endforeach(testsource)

add_test(lcov lcov --capture --directory ${PROJECT_SOURCE_DIR} --output-file cov.info)
add_test(genhtml genhtml cov.info --output-directory html-coverage)
