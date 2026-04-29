cmake_minimum_required(VERSION 3.10)
project(PPSE)

# 1. Create a toggleable option (Default is OFF)
option(USE_STATS "Enable per-block performance statistics" OFF)

add_executable(simulator
    source_generateur.c
    codec_repetition_encode.c
    bpsk_mod.c
    noise_gen.c
    bpsk_demod.c
    soft_decode.c
    hard_decode.c
    monitor.c
    quantizer.c
    main.c 
)

# 2. Link the math library
target_link_libraries(simulator m)

# 3. Only add the definition if the option is ON
if(USE_STATS)
    target_compile_definitions(simulator PRIVATE ENABLE_STATS)
    message(STATUS "Performance statistics are ENABLED")
endif()

# (Your compare executable remains the same)
add_executable(compare compare_noise_main.c noise_gen.c)
target_link_libraries(compare m)