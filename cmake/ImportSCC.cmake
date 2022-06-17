#
# Copyright (c) 20221 MINRES Technolgies GmbH
#
# SPDX-License-Identifier: Apache-2.0
#
cmake_minimum_required(VERSION 3.14)   # The FetchModule is only available since CMake 3.11.

option(UPDATE_SCC_SRC "Always update the SCC library" OFF)

include(FetchContent)
FetchContent_Declare(
    scc_git
    GIT_REPOSITORY "https://github.com/Minres/SystemC-Components.git"
    GIT_TAG        "origin/develop"
    GIT_SHALLOW    ON
  	UPDATE_DISCONNECTED NOT ${UPDATE_SCC_SRC} # When enabled, this option causes the update step to be skipped.
)
FetchContent_Populate(scc_git)
add_subdirectory(${scc_git_SOURCE_DIR} )
