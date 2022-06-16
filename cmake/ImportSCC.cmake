#
# Copyright (c) 20221 MINRES Technolgies GmbH
#
# SPDX-License-Identifier: Apache-2.0
#
include(ExternalProject)
ExternalProject_Add(
	scc_git
    GIT_REPOSITORY "https://github.com/Minres/SystemC-Components.git"
    GIT_TAG        "origin/develop"
    GIT_SHALLOW    ON
  	UPDATE_DISCONNECTED NOT ${UPDATE_UNICORN_SRC} # When enabled, this option causes the update step to be skipped.
  	CMAKE_ARGS    -DCMAKE_INSTALL_PREFIX:PATH=install -DBUILD_SCC_DOCUMENTATION=OFF
)

ExternalProject_Get_Property(scc_git install_dir)
message(STATUS "SCC is ${install_dir}")
set(scc_ROOT ${install_dir}/src/scc_git)
set(scc_INSTALL ${install_dir}/src/scc_git-build/install)

add_library (scc-util STATIC IMPORTED)
add_dependencies( scc-util scc_git)
set_target_properties( scc-util PROPERTIES IMPORTED_LOCATION 	${scc_INSTALL}/lib/static/libscc-util.a)

add_library (scc-sysc STATIC IMPORTED)
add_dependencies( scc-sysc scc_git)
set_target_properties( scc-sysc PROPERTIES IMPORTED_LOCATION 	${scc_INSTALL}/lib/static/libscc-sysc.a)

add_library (scv-tr STATIC IMPORTED)
add_dependencies( scv-tr scc_git)
set_target_properties( scv-tr PROPERTIES IMPORTED_LOCATION 	${scc_INSTALL}/lib/static/libscv-tr.a)

add_library(scc INTERFACE)
if(HAVE_GETENV)
    target_compile_definitions(scc INTERFACE HAVE_GETENV)
endif()
set_target_properties(scc PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${scc_INSTALL}/include)
target_link_libraries(scc INTERFACE scc-util scc-sysc scv-tr pthread)
