cmake_minimum_required(VERSION 2.8.9)

# this is supposedly bad...and will break?
# https://gitlab.kitware.com/cmake/community/wikis/FAQ#how-do-i-use-a-different-compiler
# set(CMAKE_C_COMPILER "x86_64-w64-mingw32-gcc")
# project (knr)
# set(CMAKE_BUILD_TYPE Release)
# # set(GCC_COMPILE_FLAGS "-mno-cygwin")
# # set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${GCC_COMPILE_FLAGS}")
# add_executable(knr knr.c)

set(CMAKE_CXX_COMPILER "x86_64-w64-mingw32-g++")
project (gamedev)
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libgcc -static-libstdc++")
# add to PATH: C:\cygwin64\usr\x86_64-w64-mingw32\sys-root\mingw\bin


add_executable(guess_the_number guess_the_number.cpp)
add_executable(hangman hangman.cpp)

