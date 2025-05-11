idf_component_register(SRCS "mbuslora.cpp"
                    INCLUDE_DIRS ".")
                    add_compile_options(-Wno-error=format)
                    set_target_properties(${COMPONENT_LIB} PROPERTIES COMPILE_FLAGS -Wno-errors)
