cmake_minimum_required(VERSION 3.17)
project(csocket C)

set(CMAKE_C_COMPILER "clang")
set(CMAKE_C_FLAGS "-std=c99 -Weverything -Wno-reserved-id-macro -Wno-switch-enum -Wno-format-nonliteral -Wno-padded -Wno-used-but-marked-unused")
set(CMAKE_C_FLAGS_DEBUG "-g -D_CSOCKET_DEBUG")
set(CMAKE_C_FLAGS_RELEASE "-O2")
set(CMAKE_EXE_LINKER_FLAGS_RELEASE "-Wl,-gc-sections -s")

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif ()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)

add_executable(${PROJECT_NAME} src/main.c src/types.h src/log.c src/log.h src/common.h src/rh/server.c src/rh/server.h src/rh/client.c src/rh/client.h src/client.c src/client.h src/server.c src/server.h)
add_custom_command(TARGET ${PROJECT_NAME} PRE_BUILD COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

find_package(Threads REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE Threads::Threads m)
