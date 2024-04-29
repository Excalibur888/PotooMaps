cmake_minimum_required(VERSION 3.25)
project(TPFinal C)

set(CMAKE_C_STANDARD 11)

include_directories(.)

add_executable(TPFinal
        cJSON.c
        cJSON.h
        dict.c
        dict.h
        municipalities.h
        graph.c
        graph.h
        graphList.c
        graphMat.c
        intList.c
        intList.h
        intTree.c
        intTree.h
        main.c
        path.c
        path.h
        poi.c
        poi.h
        settings.h
        uniStr.c
        uniStr.h poi.c)
