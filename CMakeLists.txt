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

add_library(myCP)
target_sources(myCP
    PRIVATE
        src/cp/calc.c
    PUBLIC
        src/cp/calc.h
)

add_library(myNP)
target_sources(myNP
    PRIVATE
        src/np/naming_proxy.c
    PUBLIC
        src/np/naming_proxy.h
)

add_library(myR)
target_sources(myR
    PRIVATE
        src/r/requestor.c
    PUBLIC
        src/r/requestor.h
)

add_library(myM)
target_sources(myM
    PRIVATE
        src/m/data.c
        src/m/marshaller.c
    PUBLIC
        src/m/data.h
        src/m/marshaller.h
)

add_library(myRH)
target_sources(myRH
    PRIVATE
        src/rh/client.c
        src/rh/server.c
    PUBLIC
        src/rh/types.h
        src/rh/client.h
        src/rh/server.h
)

add_library(myLog)
target_sources(myLog
    PRIVATE
        src/log.c
    PUBLIC
        src/log.h
)

add_executable(${PROJECT_NAME} src/main.c src/types/primitive.h src/client.c src/client.h src/server.c src/server.h)

include_directories(src)
find_package(Threads REQUIRED)
target_link_libraries(myCP PRIVATE myNP myR)
target_link_libraries(myR PRIVATE myM myRH)
target_link_libraries(${PROJECT_NAME} PRIVATE myLog myCP myNP Threads::Threads m)
add_custom_command(TARGET ${PROJECT_NAME} PRE_BUILD COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
