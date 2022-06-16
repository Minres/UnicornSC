#
# Copyright (c) 2022 MINRES Technolgies GmbH
#
# SPDX-License-Identifier: Apache-2.0
#
include(ExternalProject)
ExternalProject_Add(
	unicorn
    GIT_REPOSITORY "https://github.com/unicorn-engine/unicorn.git"
    GIT_TAG        "origin/master"
    GIT_SHALLOW    ON
  	UPDATE_DISCONNECTED NOT ${UPDATE_UNICORN_SRC} # When enabled, this option causes the update step to be skipped.
  	CMAKE_ARGS    -DCMAKE_INSTALL_PREFIX:PATH=install -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
)

ExternalProject_Get_Property(unicorn install_dir)
message(STATUS "Unicorn is ${install_dir}")
set(unicorn_ROOT ${install_dir}/src/unicorn)
set(unicorn_INSTALL ${install_dir}/src/unicorn-build/install)
add_library (libunicorn SHARED IMPORTED)
add_dependencies(libunicorn unicorn)
set_target_properties(libunicorn PROPERTIES IMPORTED_LOCATION ${unicorn_INSTALL}/lib/libunicorn.so)
set_target_properties(libunicorn PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${unicorn_INSTALL}/include)
