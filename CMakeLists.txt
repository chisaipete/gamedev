cmake_minimum_required(VERSION 3.13)
project(gamedev)

#set(CMAKE_CXX_STANDARD 14)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR})

set(SDL2_INCLUDE_DIR C:\\MinGW\\include\\SDL2)
set(GL_INCLUDE_DIR C:\\MinGW\\include\\GL)
set(LIBRARY_DIR C:\\MinGW\\lib)

include_directories(${SDL2_INCLUDE_DIR})
include_directories(${GL_INCLUDE_DIR})
link_directories(${LIBRARY_DIR})

set(SDL2_LINK mingw32 SDL2main SDL2 SDL2_image SDL2_ttf)
set(GL_LINK OpenGL32 glu32 glew32)

add_executable(gtn guess_the_number.cpp)
add_executable(hangman hangman.cpp)
add_executable(ttt tic_tac_toe.cpp)
add_executable(rps rock_paper_scissors.cpp)
add_executable(breakout breakout.cpp)
add_executable(tetris tetris.cpp)
add_executable(asteroids asteroids.cpp)
add_executable(dungeon dungeon.cpp)
add_executable(leaper leaper.cpp)
add_executable(tilemap tilemap.cpp)
add_executable(tiny tiny.cpp)
add_executable(tinyprof tiny.cpp)
add_executable(opengl opengl.cpp)
add_executable(opengl_dbug opengl.cpp)
target_link_libraries(ttt ${SDL2_LINK})
target_link_libraries(rps ${SDL2_LINK})
target_link_libraries(breakout ${SDL2_LINK})
target_link_libraries(tetris ${SDL2_LINK})
target_link_libraries(asteroids ${SDL2_LINK})
target_link_libraries(dungeon ${SDL2_LINK})
target_link_libraries(leaper ${SDL2_LINK})
target_link_libraries(tilemap ${SDL2_LINK})
target_link_libraries(tiny ${SDL2_LINK})
target_link_libraries(tinyprof ${SDL2_LINK})
target_link_libraries(opengl ${SDL2_LINK} ${GL_LINK})
target_link_libraries(opengl_dbug ${SDL2_LINK} ${GL_LINK})

#add_executable(gamedev
#        asteroids.cpp
#        asteroids.h
#        breakout.cpp
#        breakout.h
#        cute_tiled.h
#        dungeon.cpp
#        dungeon.h
#        guess_the_number.cpp
#        hangman.cpp
#        leaper.cpp
#        leaper.h
#        opengl.cpp
#        opengl.h
#        rock_paper_scissors.cpp
#        tetris.cpp
#        tetris.h
#        tic_tac_toe.cpp
#        tilemap.cpp
#        tiny.cpp
#        tiny.h
#        tinygl.h)
