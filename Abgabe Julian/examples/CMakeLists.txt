cmake_minimum_required(VERSION 3.5)
set(CMAKE_CXX_COMPILER "/Users/I575211/Development/llvm-project/build_cmake/bin/clang++")
project(examples)

# add_compile_options(-fsanitize=thread)
add_link_options(-fsanitize=thread)

add_executable(tiny_race tiny_race.cc)
add_executable(tiny_race_sanitized tiny_race.cc)
set_target_properties(tiny_race_sanitized PROPERTIES COMPILE_FLAGS "-fsanitize=thread -g -O0")

add_executable(vptr_race vptr_race.cc)
add_executable(vptr_race_sanitized vptr_race.cc)
set_target_properties(vptr_race_sanitized PROPERTIES COMPILE_FLAGS "-fsanitize=thread -g -O0")

add_executable(mutex_test mutex_test.cc)
add_executable(mutex_test_sanitized mutex_test.cc)
set_target_properties(mutex_test_sanitized PROPERTIES COMPILE_FLAGS "-fsanitize=thread -g -O0")

add_executable(djit_example djit_example.cc)
add_executable(djit_example_sanitized djit_example.cc)
set_target_properties(djit_example_sanitized PROPERTIES COMPILE_FLAGS "-fsanitize=thread -g -O0")