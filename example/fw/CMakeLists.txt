if(NOT EXISTS "${CMAKE_CURRENT_LIST_DIR}/coremark/cm")
    find_package(Git)
    execute_process(
        COMMAND             ${GIT_EXECUTABLE} clone https://github.com/eembc/coremark.git --recursive ${CMAKE_CURRENT_LIST_DIR}/coremark/cm
        RESULT_VARIABLE     git_result
        OUTPUT_VARIABLE     git_output)
    if(NOT git_result EQUAL "0")
        set(${CLONE_RESULT} FALSE CACHE INTERNAL "" FORCE)
        if(NOT PARGS_QUIET)
            message(WARNING "Could not clone https://github.com/eembc/coremark.git, CoreMark will be disabled")  #ToDo: maybe FATAL_ERROR?
        endif()
    else()
        add_custom_target(fw-coremark
            COMMAND make PORT_DIR=../tgc
            WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/coremark/cm
            COMMENT "Building coremark firmware"
            BYPRODUCTS coremark/cm/coremark.elf
            USES_TERMINAL
        )        
    endif()
else()     
    add_custom_target(fw-coremark
        COMMAND make PORT_DIR=../tgc
        WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/coremark/cm
        COMMENT "Building coremark firmware"
        BYPRODUCTS coremark/cm/coremark.elf
        USES_TERMINAL
    )        
endif()
add_custom_target(fw-dhrystone
    COMMAND make
    WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/dhrystone
    COMMENT "Building dhrystone firmware"
    BYPRODUCTS dhrystone/dhrystone
    USES_TERMINAL
)
add_custom_target(fw-directed-test
    COMMAND make
    WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/directed/test
    COMMENT "Building directed test firmware"
    BYPRODUCTS directed/test/firmware.elf
    USES_TERMINAL
)
add_custom_target(fw-directed-pmp
    COMMAND make
    WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/directed/pmp
    COMMENT "Building directed firmware to test PMP"
    BYPRODUCTS directed/test/firmware.elf
    USES_TERMINAL
)
add_custom_target(fw-directed-wfi
    COMMAND make
    WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/directed/wfi
    COMMENT "Building directed firmware to test WFI"
    BYPRODUCTS directed/wfi/firmware.elf
    USES_TERMINAL
)
add_custom_target(fw-directed-zol
    COMMAND make
    WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/directed/hwl
    COMMENT "Building directed firmware to test hwl"
    BYPRODUCTS directed/hwl/firmware.elf
    USES_TERMINAL
)
add_custom_target(fw-directed-ldst
    COMMAND make
    WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/directed/ld_st
    COMMENT "Building directed firmware to test ld_st"
    BYPRODUCTS directed/ld_st/firmware.elf
    USES_TERMINAL
)
if(DEFINED ENV{LLVM_TOOL_DIR})
add_custom_target(fw-hwl-factorial
    COMMAND make
    WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/hwl-factorial
    COMMENT "Building directed firmware to test hwl-factorial"
    BYPRODUCTS hwl-factorial/build/app
    USES_TERMINAL
)
add_custom_target(fw-hwl-fir
    COMMAND make
    WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/hwl-fir
    COMMENT "Building directed firmware to test hwl-fir"
    BYPRODUCTS hwl-fir/build/app
    USES_TERMINAL
)
endif()