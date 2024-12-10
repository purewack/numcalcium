# This file was automatically generated for projects
# without default 'CMakeLists.txt' file.

# FILE(GLOB_RECURSE app_sources ${CMAKE_SOURCE_DIR}/src/*.*)

idf_component_register(SRCS 
    # ${app_sources} 
    "main.c"
    INCLUDE_DIRS ""
    REQUIRES driver soc ulp)

set(ulp_app_name ulp_main)
set(ulp_s_sources "../ulp/main.S" )
set(ulp_exp_dep_srcs "main.c")

ulp_embed_binary(${ulp_app_name} ${ulp_s_sources} ${ulp_exp_dep_srcs})
