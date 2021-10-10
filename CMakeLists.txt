cmake_minimum_required(VERSION 3.18.0)

project(loop VERSION 0.1.0)

add_library(loop
src/pool.c
src/server.c
src/hashmap.c
src/loop.c
src/task.c
src/worker.c
)

add_executable(server
src/main.c
src/server.c
)

target_compile_options(loop PRIVATE -Wall -Werror)
target_include_directories(loop PRIVATE src)
target_include_directories(loop PUBLIC include)
target_link_libraries(loop PUBLIC pthread)


target_include_directories(server PRIVATE src)
target_link_libraries(server PRIVATE loop)