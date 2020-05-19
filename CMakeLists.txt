# CMakeList.txt: 顶层 CMake 项目文件，在此处执行全局配置
# 并包含子项目。
#
cmake_minimum_required (VERSION 3.8)

MESSAGE(STATUS "operation system is ${CMAKE_SYSTEM}")

# Options. Turn on with 'cmake -Dmyvarname=ON'.
option(GAPING_SECURITY_HOLE "nc with -e option" ON)
option(TELNET "nc with -t option" ON)
option(test "Build all tests." OFF) # Makes boolean 'test' available.

# Check platforms
if (CMAKE_HOST_WIN32)
    set(WIN32 1)
elseif(CMAKE_HOST_APPLE)
    set(MACOS 1)
elseif(CMAKE_HOST_UNIX)
    set(LINUX 1)
endif()

project ("lcx" C)

# force static runtime libraries for msvc builds
if(MSVC)
    add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
    add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")
  add_definitions(-D_CRT_SECURE_NO_WARNINGS -DGAPING_SECURITY_HOLE -DTELNET)
  set(variables 
    CMAKE_CXX_FLAGS_DEBUG
    CMAKE_CXX_FLAGS_RELEASE
	CMAKE_CXX_FLAGS_RELWITHDEBINFO
	CMAKE_CXX_FLAGS_MINSIZEREL
  )
  foreach(variable ${variables})
	if(${variable} MATCHES "/MD")
	  string(REGEX REPLACE "/MD" "/MT" ${variable} "${${variable}}")
	endif()
  endforeach()
endif()

# The version number.
set (LCX_VERSION_MAJOR 0)
set (LCX_VERSION_MINOR 1)
set (LCX_VERSION "1.0")

# configure a header file to pass some of the CMake settings
# to the source code
configure_file (
  "${PROJECT_SOURCE_DIR}/src/LcxConfig.h.in"
  "${PROJECT_BINARY_DIR}/LcxConfig.h"
  )

# add the binary tree to the search path for include files
# so that we will find TutorialConfig.h
include_directories("${PROJECT_BINARY_DIR}")

file(GLOB LCX_SOURCES src/*.c)
file(GLOB_RECURSE LIB_SOURCES lib/*.c)
file(GLOB_RECURSE LCX_HEADERS src/*.h lib/*.h)

set (LCX_INCLUDE_DIRS "")
foreach (_headerFile ${LCX_HEADERS})
    get_filename_component(_dir ${_headerFile} PATH)
    list (APPEND LCX_INCLUDE_DIRS ${_dir})
endforeach()
list(REMOVE_DUPLICATES LCX_INCLUDE_DIRS)

MESSAGE(STATUS "LCX_INCLUDE_DIRS is ${LCX_INCLUDE_DIRS}")
MESSAGE(STATUS "LCX_SOURCES is ${LCX_SOURCES}")

add_executable (lcx ${LCX_SOURCES} ${LIB_SOURCES})
target_include_directories(lcx PRIVATE ${LCX_INCLUDE_DIRS})

################################
# Testing
################################
if (test)
  if (APPLE)
    add_definitions(-DGTEST_USE_OWN_TR1_TUPLE)
    add_definitions(-D__GLIBCXX__)
  endif (APPLE)

  # Options. Turn on with 'cmake -Dmyvarname=ON'.
  option(BUILD_GMOCK "Build with gmock." ON) # Makes boolean 'gmock' available.
  #option(BUILD_SHARED_LIBS "Build shared libraries (DLLs)." OFF)

  add_subdirectory(third-party/gtest/)

  # 'project(gtest)' above.
  include_directories("third-party/gtest/googletest/include" "third-party/gtest/googlemock/include" ${gtest_SOURCE_DIR})

  ##############
  # Unit Tests
  ##############
  file(GLOB_RECURSE TEST_SOURCES "test/*.cpp" "test/*.c")
  add_executable(runUnitTests ${TEST_SOURCES})

  # Standard linking to gtest stuff.
  target_link_libraries(runUnitTests gtest gtest_main gmock)


  # This is so you can do 'make test' to see all your tests run, instead of
  # manually running the executable runUnitTests to see those specific tests.
  add_test(NAME that-test-I-made COMMAND runUnitTests)

  # You can also omit NAME and COMMAND. The second argument could be some other
  # test executable.
  add_test(that-other-test-I-made runUnitTests)
endif()