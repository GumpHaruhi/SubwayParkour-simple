cmake_minimum_required(VERSION 3.23)
project(untitled22 C)

set(CMAKE_C_STANDARD 99)

add_executable(main main.c)
target_link_libraries(main mingw32)
target_link_libraries(main SDL2main)
target_link_libraries(main SDL2)
target_link_libraries(main SDL2_image)
target_link_libraries(main SDL2_ttf)

