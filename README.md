Tsan Projektarbeit.

Requirements: 
  CMAKE 3.13.4 or higher
  ONE OF THE FOLLOWING OS:  
      Android aarch64, x86_64
      Darwin arm64, x86_64
      FreeBSD
      Linux aarch64, x86_64, powerpc64, powerpc64le
      NetBSD
 Only 64 Bit Architectures are supported



How to build:
  1. clone project
  2. mkdir build && cd build
  3. cmake -DLLVM_ENABLE_PROJECTS="clang;compiler-rt;" DCMAKE_BUILD_TYPE=Release ../llvm_source
     This will build clang and the runtime libraries including the Threadsanitizer project
     For debug puprposes use "CMAKE_BUILD_TYPE=Debug"
  4. "make"  to build
      "make check-tsan" to build and run tests for tsan
 
 More Requirements for llvm and flags can be found here https://llvm.org/docs/GettingStarted.html#requirements
 
 
How to use:

Option1:
To test your code for Data races do the following:
1. Set the flag to run with tsan and use the right compiler
    path_to_your_llvm_project/build/bin/clang++ path_to_your_program -fsanitize=thread -fPIE -pie -g -O1
2. To run    ./a.out 2> log.txt
3. This will write the output of tsan in the log.txt file.

Option2:

1. Add the following to the cmake file of your program:

    set(CMAKE_CXX_COMPILER "/home/martin/CLionProjects/llvm-project-2/build/bin/clang++")

    # add_compile_options(-fsanitize=thread)
    add_link_options(-fsanitize=thread)

    add_executable(your_name_to_call_without_flags your_program.cc)
    add_executable(your_name_to_call_with_flags  your_programm.cc)
    set_target_properties(your_name_to_call_with_flags PROPERTIES COMPILE_FLAGS "-fsanitize=thread -fPIE -pie -g -O1")
3. mkdir build && cd build
4. cmake path_to_source
5. build your programm with "make"
6. Run your programm with ./your_name_to_call_with_flags 2>log.txt
7. This will write the output of tsan in the log.txt file.

What events are tracked.

-Read Write operations.
-Vector clocks (not yet finished)












 
 




