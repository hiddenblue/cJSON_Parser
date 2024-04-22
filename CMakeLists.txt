cmake_minimum_required(VERSION 3.10.0)



project(cJSON)

add_executable(main src/main.c src/cjson.c src/errorPrint.c)

target_compile_options("-g -Wall -O0 -fno-inline")

target_include_directories("include")

target_link_libraries()