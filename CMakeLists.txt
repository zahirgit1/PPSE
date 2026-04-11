# 1. Set the minimum version of CMake required
cmake_minimum_required(VERSION 3.10)

# 2. Name your project and the language used
project(MyAwesomeProject C)

# 3. Specify the executable name and the source files to include
add_executable(simulator
    source_generateur.c
    codec_repetition_encode.c
    bpsk_mod.c
    noise_gen.c
    bpsk_demod.c
    soft_decode.c
    hard_decode.c
    monitor.c
    main.c 
)
target_link_libraries(simulator m)
add_executable(compare compare_noise_main.c
    noise_gen.c)
target_link_libraries(compare m)
