cmake_minimum_required (VERSION 2.6)
project(piojo)

include(CTest)

set(piojo_VERSION_MAJOR 0)
set(piojo_VERSION_MINOR 9)
set(piojo_VERSION_PATCH 4)

option(PIOJO_DEBUG "Build with debugging support" OFF)
option(BUILD_SHARED_LIBS "Build library as shared" ON)
option(BUILD_DOCUMENTATION "Use Doxygen to create the HTML based API documentation" ON)

if(BUILD_DOCUMENTATION)
  find_package(Doxygen REQUIRED)
  #-- Configure the Template Doxyfile for our specific project
  configure_file(${PROJECT_SOURCE_DIR}/doc/Doxyfile.in
                 ${PROJECT_BINARY_DIR}/Doxyfile  @ONLY IMMEDIATE)
  #-- Add a custom target to run Doxygen when ever the project is built
  add_custom_target (doc
                     COMMAND ${DOXYGEN_EXECUTABLE} ${PROJECT_BINARY_DIR}/Doxyfile
                     SOURCES ${PROJECT_BINARY_DIR}/Doxyfile)
endif(BUILD_DOCUMENTATION)

set(EXTRA_LIBS ${EXTRA_LIBS} m)

if(CMAKE_BUILD_TYPE STREQUAL "Testing")
  set(EXTRA_LIBS ${EXTRA_LIBS} gcov)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -W -Wall -pedantic -pg -fprofile-arcs -ftest-coverage")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -W -Wall -pedantic -pg -fprofile-arcs -ftest-coverage")
endif(CMAKE_BUILD_TYPE STREQUAL "Testing")

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -W -Wall -pedantic")
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -W -Wall -pedantic")
endif(CMAKE_BUILD_TYPE STREQUAL "Debug")

#SET(CMAKE_SHARED_LINKER_FLAGS="-fprofile-arcs -ftest-coverage")
#SET(CMAKE_EXE_LINKER_FLAGS="-fprofile-arcs -ftest-coverage")

# FOR DEPENDENCIES....
#find_package(Curses REQUIRED)
#find_package(Curses)
#if (CURSES_FOUND)
#    include_directories(${CURSES_INCLUDE_DIR})
#    set(EXTRA_LIBS ${EXTRA_LIBS} ${CURSES_LIBRARIES})
#endif(CURSES_FOUND)

add_subdirectory(src)
add_subdirectory(test)

# build a CPack driven installer package
include(InstallRequiredSystemLibraries)
set(CPACK_RESOURCE_FILE_LICENSE "${PROJECT_SOURCE_DIR}/LICENSE")
set(CPACK_PACKAGE_DESCRIPTION_FILE "${PROJECT_SOURCE_DIR}/README")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Small C library")
#set(CPACK_PACKAGE_INSTALL_DIRECTORY "${PROJECT_NAME}-${piojo_VERSION_MAJOR}.${piojo_VERSION_MINOR}")
set(CPACK_PACKAGE_VENDOR "Elian Gidoni")
set(CPACK_PACKAGE_CONTACT "https://github.com/eliangidoni")
set(CPACK_PACKAGE_VERSION_MAJOR "${piojo_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${piojo_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${piojo_VERSION_PATCH}")
include(CPack)
