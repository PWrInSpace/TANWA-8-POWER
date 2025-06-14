idf_component_register(
    SRC_DIRS "."
    INCLUDE_DIRS "."
    REQUIRES config can control12V control24V control24V_SOL Board_Data cli
)


target_compile_options(${COMPONENT_LIB} PRIVATE "-Wno-format" "-Wall" "-Werror")
