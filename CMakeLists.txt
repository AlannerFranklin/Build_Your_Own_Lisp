cmake_minimum_required(VERSION 3.10)
project(lispy C)

set(CMAKE_C_STANDARD 99)

add_executable(lispy
    parsing.c
    lval.c
    lenv.c
    builtins.c
    mpc.c
)

if(UNIX)
    target_link_libraries(lispy m edit readline)
endif()
