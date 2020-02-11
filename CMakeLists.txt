cmake_minimum_required(VERSION 3.9)
project(generate)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(ASAN_CFLAGS -fsanitize=undefined -fsanitize=address -fstack-protector -ftrivial-auto-var-init=pattern)
    set(ASAN_LINKFLAGS ${ASAN_CFLAGS} -lstdc++)
endif()

add_executable(generate main.c)

target_compile_options(generate PRIVATE ${ASAN_CFLAGS} -Wall -fno-exceptions)

target_link_options(generate PRIVATE ${ASAN_LINKFLAGS})
