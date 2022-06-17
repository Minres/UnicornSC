#
# Copyright (c) 20221 MINRES Technolgies GmbH
#
# SPDX-License-Identifier: Apache-2.0
#
cmake_minimum_required(VERSION 3.12)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

project(unicorn-sc VERSION 1.0.0)

set(UNICORN_ROOT "" CACHE PATH "Install location of the Unicorn Engine")

if(CMAKE_PROJECT_NAME STREQUAL "unicorn-sc")
    message(STATUS "Building Unicorn-SC in standalone mode")
	list(APPEND CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR})
	list(APPEND CMAKE_PREFIX_PATH ${CMAKE_BINARY_DIR})
    include(GNUInstallDirs)
    set(CMAKE_CXX_STANDARD 11)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
	set(CONAN_BOOST_OPTIONS
		boost:fPIC=True
		boost:shared=True
		boost:header_only=False 
		boost:without_contract=True
		boost:without_graph=True
		boost:without_graph_parallel=True
		boost:without_iostreams=True
		boost:without_json=True
		boost:without_locale=True
		boost:without_log=True
		boost:without_math=True
		boost:without_mpi=True
		boost:without_nowide=True
		boost:without_python=True
		boost:without_random=True
		boost:without_regex=True
		boost:without_stacktrace=True
		boost:without_test=True
		boost:without_timer=True
		boost:without_type_erasure=True
		boost:without_wave=True
	)

	if(NOT EXISTS "${CMAKE_BINARY_DIR}/conan.cmake")
	  message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
	  file(DOWNLOAD "https://raw.githubusercontent.com/conan-io/cmake-conan/0.18.1/conan.cmake"
	                "${CMAKE_BINARY_DIR}/conan.cmake"
	                TLS_VERIFY ON)
	endif()
	
	include(${CMAKE_BINARY_DIR}/conan.cmake)
	conan_cmake_configure(REQUIRES systemc/2.3.3 systemc-cci/1.0.0 fmt/8.1.1 elfio/3.8 boost/1.75.0
	                      GENERATORS cmake_find_package)
	
	conan_cmake_autodetect(settings BUILD_TYPE "None")
	
	conan_cmake_install(PATH_OR_REFERENCE .
	                    BUILD missing
	                    REMOTE conancenter
	                    SETTINGS ${settings}
	                    OPTIONS systemc-cci:shared=False fmt:header_only=True ${CONAN_BOOST_OPTIONS})
endif()

find_package(SystemCLanguage)
find_package(fmt)

if(NOT UNICORN_ROOT)
	include(ImportUnicorn)
else()
	add_library (unicorn INTERFACE) #SHARED IMPORTED)
	target_include_directories(unicorn INTERFACE ${UNICORN_ROOT}/include)
	target_link_libraries(unicorn INTERFACE ${UNICORN_ROOT}/lib/libunicorn.so)
endif()

include(ImportSCC)

add_library(${PROJECT_NAME}	src/unicorn_sc.cpp)
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_LIST_DIR}/src)
target_link_directories(${PROJECT_NAME} PUBLIC  ${CMAKE_BINARY_DIR})
target_link_libraries(${PROJECT_NAME} PUBLIC unicorn SystemC::systemc)

add_subdirectory(example)
