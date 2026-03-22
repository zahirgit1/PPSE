# 1. Set the minimum version of CMake required
cmake_minimum_required(VERSION 3.10)

# 2. Name your project and the language used
project(MyAwesomeProject C)

# 3. Specify the executable name and the source files to include
add_executable(test_fullstart_chain
    source_generateur.c
    codec_repetition_encode.c
    bpsk_mod.c
    main.c 

)
