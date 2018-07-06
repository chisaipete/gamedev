cmake_minimum_required(VERSION 3.7)

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

# set(CMAKE_PREFIX_PATH "${CMAKE_PREFIX_PATH} ${CMAKE_CURRENT_LIST_DIR}/SDL2.0.8_x86_64/lib")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake")

#SDL2
set(SDL2 "C:/SDL2-2.0.8")
find_package(SDL2 REQUIRED)
message(STATUS "Include directory: " ${SDL2_INCLUDE_DIR})
message(STATUS "Libraries: " ${SDL2_LIBRARY})
include_directories(${SDL2_INCLUDE_DIR})

#SDL2_image
set(SDL2_IMAGE "C:/SDL2_image-2.0.3")
find_package(SDL2_image REQUIRED)
message(STATUS "Include directory: " ${SDL2_IMAGE_INCLUDE_DIR})
message(STATUS "Libraries: " ${SDL2_IMAGE_LIBRARY})
include_directories(${SDL2_IMAGE_INCLUDE_DIR})

#SDL2_ttf
set(SDL2_TTF "C:/SDL2_ttf-2.0.14")
find_package(SDL2_ttf REQUIRED)
message(STATUS "Include directory: " ${SDL2_TTF_INCLUDE_DIR})
message(STATUS "Libraries: " ${SDL2_TTF_LIBRARY})
include_directories(${SDL2_TTF_INCLUDE_DIR})

# add_executable(guess_the_number guess_the_number.cpp)

# add_executable(hangman hangman.cpp)

# set(SOURCE_FILES tic_tac_toe.cpp)
# add_executable(tic_tac_toe ${SOURCE_FILES})
# target_link_libraries(tic_tac_toe ${SDL2_LIBRARY} ${SDL2_IMAGE_LIBRARY} ${SDL2_TTF_LIBRARY})

# set(SOURCE_FILES rock_paper_scissors.cpp)
# add_executable(rock_paper_scissors ${SOURCE_FILES})
# target_link_libraries(rock_paper_scissors ${SDL2_LIBRARY} ${SDL2_IMAGE_LIBRARY} ${SDL2_TTF_LIBRARY})

# set(SOURCE_FILES breakout.cpp breakout.h)
# add_executable(breakout ${SOURCE_FILES})
# target_link_libraries(breakout ${SDL2_LIBRARY} ${SDL2_IMAGE_LIBRARY} ${SDL2_TTF_LIBRARY})

# set(SOURCE_FILES tetris.cpp tetris.h)
# add_executable(tetris ${SOURCE_FILES})
# target_link_libraries(tetris ${SDL2_LIBRARY} ${SDL2_IMAGE_LIBRARY} ${SDL2_TTF_LIBRARY})

# set(SOURCE_FILES asteroids.cpp asteroids.h)
# add_executable(asteroids ${SOURCE_FILES})
# target_link_libraries(asteroids ${SDL2_LIBRARY} ${SDL2_IMAGE_LIBRARY} ${SDL2_TTF_LIBRARY})

# set(SOURCE_FILES dungeon.cpp dungeon.h)
# add_executable(dungeon ${SOURCE_FILES})
# target_link_libraries(dungeon ${SDL2_LIBRARY} ${SDL2_IMAGE_LIBRARY} ${SDL2_TTF_LIBRARY})

set(SOURCE_FILES leaper.cpp leaper.h)
add_executable(leaper ${SOURCE_FILES})
target_link_libraries(leaper ${SDL2_LIBRARY} ${SDL2_IMAGE_LIBRARY} ${SDL2_TTF_LIBRARY})

