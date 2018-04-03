cmake_minimum_required(VERSION 3.9)
project(FIRMWARE)

set(CMAKE_CXX_STANDARD 11)

add_executable(FIRMWARE
        common/comm.c
        common/comm.h
        common/neuron.c
        common/neuron.h
        src/flags.h
        src/HAL.c
        src/HAL.h
        src/main.c)
