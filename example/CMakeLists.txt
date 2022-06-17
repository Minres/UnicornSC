#
# Copyright (c) 2022 MINRES Technolgies GmbH
#
# SPDX-License-Identifier: Apache-2.0
#
cmake_minimum_required(VERSION 3.12)

project(unicorn-sc-example)

find_package(systemc-cci)
find_package(elfio)

add_executable(unicorn-sc-ex sc_main.cpp testbench.cpp uart.cpp)
target_link_libraries(unicorn-sc-ex unicorn-sc scc systemc-cci::systemc-cci elfio::elfio)