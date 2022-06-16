add_custom_target(fw-hello-world
    COMMAND make
    WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/hello-world
    COMMENT "Building directed firmware to test hello-world"
    BYPRODUCTS hello-world/hello
    USES_TERMINAL
)
