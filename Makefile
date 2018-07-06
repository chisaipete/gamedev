#OBJS specifies which files to compile as part of the project
# OBJS = guess_the_number.cpp
# OBJS = hangman.cpp
# OBJS = tic_tac_toe.cpp
# OBJS = rock_paper_scissors.cpp
# OBJS = breakout.cpp
# OBJS = tetris.cpp
# OBJS = asteroids.cpp
# OBJS = dungeon.cpp
OBJS = leaper.cpp

#CC specifies which compiler we're using
CC = g++

#INCLUDE_PATHS specifies the additional include paths we'll need
INCLUDE_PATHS = -IC:\MinGW\include\SDL2

#LIBRARY_PATHS specifies the additional library paths we'll need
LIBRARY_PATHS = -LC:\MinGW\lib

#COMPILER_FLAGS specifies the additional compilation options we're using
# -g debugging symbols
# -w suppresses all warnings
# -Wl,-subsystem,windows gets rid of the console window
# COMPILER_FLAGS = -w -Wl,-subsystem,windows
COMPILER_FLAGS = -g

#LINKER_FLAGS specifies the libraries we're linking against
LINKER_FLAGS = -lmingw32 -lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf

#OBJ_NAME specifies the name of our exectuable
# OBJ_NAME = guess_the_number
# OBJ_NAME = hangman
# OBJ_NAME = tic_tac_toe
# OBJ_NAME = rock_paper_scissors
# OBJ_NAME = breakout
# OBJ_NAME = tetris
# OBJ_NAME = asteroids
# OBJ_NAME = dungeon
OBJ_NAME = leaper

#This is the target that compiles our executable
all : $(OBJS)
	$(CC) $(OBJS) $(INCLUDE_PATHS) $(LIBRARY_PATHS) $(COMPILER_FLAGS) $(LINKER_FLAGS) -o $(OBJ_NAME)