cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 11)
project("Glabor's Filter")

find_package(PkgConfig REQUIRED)
set(CMAKE_FIND_LIBRARY_SUFFIXES .so .a)
set(CMAKE_FIND_LIBRARY_PREFIXES "")
pkg_check_modules(GTK REQUIRED gtk+-3.0)
pkg_check_modules(GLIB REQUIRED glib-2.0)
include_directories(${GLIB_INCLUDE_DIRS})
include_directories(${GTK_INCLUDE_DIRS})



add_executable(GlaborEx "main.cpp")
target_link_libraries(GlaborEx ${GTK_LIBRARIES})
target_link_libraries(GlaborEx ${GLIB_LIBRARIES})
install(TARGETS GlaborEx DESTINATION ${CMAKE_SOURCE_DIR}/src/)



