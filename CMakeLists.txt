cmake_minimum_required(VERSION 3.5)
project(A2)

set(A2_SOURCE_FILES
        definitions.h
        resources.h
        lib/vector.c
        lib/vector.h
        lib/queue.c
        lib/queue.h
        airport.c)

add_compile_options(-Wall -Werror -pthread)
add_executable(airport ${A2_SOURCE_FILES})
target_link_libraries(airport pthread)