#
# Copyright (c) 20221 MINRES Technolgies GmbH
#
# SPDX-License-Identifier: Apache-2.0
#
cmake_minimum_required(VERSION 3.12)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

project(unicorn-sc VERSION 1.0.0)

option(UPDATE_UNICORN_SRC "Always update the unicorn engine library" OFF)

if(CMAKE_PROJECT_NAME STREQUAL "unicorn-sc")
    message(STATUS "Building Unicorn-SC in standalone mode")
	list(APPEND CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR})
	list(APPEND CMAKE_PREFIX_PATH ${CMAKE_BINARY_DIR})
    include(GNUInstallDirs)
    set(CMAKE_CXX_STANDARD 11)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)

	if(NOT EXISTS "${CMAKE_BINARY_DIR}/conan.cmake")
	  message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
	  file(DOWNLOAD "https://raw.githubusercontent.com/conan-io/cmake-conan/0.18.1/conan.cmake"
	                "${CMAKE_BINARY_DIR}/conan.cmake"
	                TLS_VERIFY ON)
	endif()
	
	include(${CMAKE_BINARY_DIR}/conan.cmake)
	conan_cmake_configure(REQUIRES systemc/2.3.3 systemc-cci/1.0.0 elfio/3.8
	                      GENERATORS cmake_find_package)
	
	conan_cmake_autodetect(settings BUILD_TYPE "None")
	
	conan_cmake_install(PATH_OR_REFERENCE .
	                    BUILD missing
	                    REMOTE conancenter
	                    SETTINGS ${settings}
	                    OPTIONS systemc-cci:shared=False)
endif()

include(ImportUnicorn)

find_package(SystemCLanguage)

add_library(${PROJECT_NAME}
	src/unicorn_sc.cpp
)
message(STATUS "Linking ${UNICORN_LIB_NAME}")

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_LIST_DIR}/src)
target_include_directories(${PROJECT_NAME} PUBLIC ${install_dir}/include)
target_link_directories(${PROJECT_NAME} PUBLIC  ${CMAKE_BINARY_DIR})
target_link_libraries(${PROJECT_NAME} PUBLIC libunicorn SystemC::systemc)

add_subdirectory(example)
