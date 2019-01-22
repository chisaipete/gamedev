#OBJS specifies which files to compile as part of the project
# OBJS = guess_the_number.cpp
# OBJS = hangman.cpp
# OBJS = tic_tac_toe.cpp
# OBJS = rock_paper_scissors.cpp
# OBJS = breakout.cpp
# OBJS = tetris.cpp
# OBJS = asteroids.cpp
# OBJS = dungeon.cpp
# OBJS = leaper.cpp

#CC specifies which compiler we're using
CC = g++

#INCLUDE_PATHS specifies the additional include paths we'll need
INCLUDE_PATHS = -IC:\MinGW\include\SDL2
INCLUDE_PATHS_GL = -IC:\MinGW\include\GL

#LIBRARY_PATHS specifies the additional library paths we'll need
LIBRARY_PATHS = -LC:\MinGW\lib

#COMPILER_FLAGS specifies the additional compilation options we're using
# -g debugging symbols
# -w suppresses all warnings
# -Wl,-subsystem,windows gets rid of the console window
# COMPILER_FLAGS = -w -Wl,-subsystem,windows
COMPILER_FLAGS = -g
COMPILER_FLAGS_PROF = -ggdb -g -pg -O0
COMPILER_FLAGS_DBUG = -g -o

#LINKER_FLAGS specifies the libraries we're linking against
LINKER_FLAGS = -lmingw32 -lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf
LINKER_FLAGS_GL = -lOpenGL32 -lglu32

#OBJ_NAME specifies the name of our exectuable
# OBJ_NAME = guess_the_number
# OBJ_NAME = hangman
# OBJ_NAME = tic_tac_toe
# OBJ_NAME = rock_paper_scissors
# OBJ_NAME = breakout
# OBJ_NAME = tetris
# OBJ_NAME = asteroids
# OBJ_NAME = dungeon
# OBJ_NAME = leaper

#This is the target that compiles our executable
all: gtn hangman ttt rps breakout tetris asteroids dungeon leaper tilemap

gtn : 
	$(CC) guess_the_number.cpp $(COMPILER_FLAGS) -o guess_the_number

hangman:
	$(CC) hangman.cpp $(COMPILER_FLAGS) -o hangman

ttt:
	$(CC) tic_tac_toe.cpp $(INCLUDE_PATHS) $(LIBRARY_PATHS) $(COMPILER_FLAGS) $(LINKER_FLAGS) -o tic_tac_toe

rps:
	$(CC) rock_paper_scissors.cpp $(INCLUDE_PATHS) $(LIBRARY_PATHS) $(COMPILER_FLAGS) $(LINKER_FLAGS) -o rock_paper_scissors

breakout:
	$(CC) breakout.cpp $(INCLUDE_PATHS) $(LIBRARY_PATHS) $(COMPILER_FLAGS) $(LINKER_FLAGS) -o breakout

tetris:
	$(CC) tetris.cpp $(INCLUDE_PATHS) $(LIBRARY_PATHS) $(COMPILER_FLAGS) $(LINKER_FLAGS) -o tetris

asteroids:
	$(CC) asteroids.cpp $(INCLUDE_PATHS) $(LIBRARY_PATHS) $(COMPILER_FLAGS) $(LINKER_FLAGS) -o asteroids

dungeon:
	$(CC) dungeon.cpp $(INCLUDE_PATHS) $(LIBRARY_PATHS) $(COMPILER_FLAGS) $(LINKER_FLAGS) -o dungeon

leaper:
	$(CC) leaper.cpp $(INCLUDE_PATHS) $(LIBRARY_PATHS) $(COMPILER_FLAGS) $(LINKER_FLAGS) -o leaper

tilemap:
	$(CC) tilemap.cpp $(INCLUDE_PATHS) $(LIBRARY_PATHS) $(COMPILER_FLAGS) $(LINKER_FLAGS) -o tilemap

tiny:
	$(CC) tiny.cpp $(INCLUDE_PATHS) $(LIBRARY_PATHS) $(COMPILER_FLAGS) $(LINKER_FLAGS) -o tiny

tinyprof:
	$(CC) tiny.cpp $(INCLUDE_PATHS) $(LIBRARY_PATHS) $(COMPILER_FLAGS_PROF) $(LINKER_FLAGS) -o tiny

opengl:
	$(CC) opengl.cpp $(INCLUDE_PATHS) $(INCLUDE_PATHS_GL) $(LIBRARY_PATHS) $(COMPILER_FLAGS) $(LINKER_FLAGS) $(LINKER_FLAGS_GL) -o opengl

opengl_dbug:
	$(CC) opengl.cpp $(INCLUDE_PATHS) $(INCLUDE_PATHS_GL) $(LIBRARY_PATHS) $(COMPILER_FLAGS_DBUG) $(LINKER_FLAGS) $(LINKER_FLAGS_GL) -o opengl
