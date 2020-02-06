#ifndef GAMEDEV_TERM_H
#define GAMEDEV_TERM_H

#include <iostream>
#ifdef __WIN32
#include <windows.h>
#endif

#ifdef linux
#include <sstream>
#endif

//https://stackoverflow.com/questions/47340610/c-overwrite-multiple-lines-that-were-previously-output-to-console

class term {
public:
#ifdef __WIN32
    static void clearScreen() {
        system("cls");
        SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), {0,0});
    }
    static void gotoRowColumn(int row, int column) {
        SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), {static_cast<SHORT>(column), static_cast<SHORT>(row)});
    }
#endif
#ifdef linux
    enum ANSI : int { ESC = 27 }; // escape character
    static std::string clearScreen() {
        std::stringstream ss;
        ss << static_cast<char>(ESC) << "[H";    //home
        ss << static_cast<char>(ESC) << "[23";   //clrbos
        return ss.str();
    }

    static std::string gotoRowColumn(int row, int column) {
        std::stringstream ss;
        ss << static_cast<char>(ESC) << '[' << (row + 1) << ';' << (column + 1) << 'H';
        return ss.str();
    }
#endif
};


#endif //GAMEDEV_TERM_H
