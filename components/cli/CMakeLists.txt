idf_component_register(  SRC_DIRS "."
                         INCLUDE_DIRS "."
                         REQUIRES console Board_Data)

target_compile_options(${COMPONENT_LIB} PRIVATE "-Wno-format" "-Wall" "-Werror")
