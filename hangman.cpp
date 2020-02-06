#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <ctime>
#include <ctype.h>
#include "term.h"
// #include <libtcod.hpp>

// https://stackoverflow.com/questions/7617587/is-there-an-alternative-to-using-time-to-seed-a-random-number-generation
unsigned long long rdtsc(){
    unsigned int lo,hi;
    __asm__ __volatile__ ("rdtsc" : "=a" (lo), "=d" (hi));
    return ((unsigned long long)hi << 32) | lo;
}

std::string get_word() {
    std::string filename = "res/words.txt";
    std::ifstream istrm(filename);
    std::string word;
    int num_words = 29245; //magic number, length of file @ one word per line
    if (!istrm.is_open()) {
        std::cout << "couldn't open " << filename << std::endl;
    } else {
        int answer = rand() % num_words;
        for (int i = 0; i < answer; i++) {
            istrm >> word;
        }
        istrm.close();
    }
    return word;
}

void draw_hangman(int guesses) {
    switch (guesses) {
        case 0:
            std::cout << "   __ \n  |  |\n     |\n     |\n     |\n     |\n ____|\n|____|" << std::endl;
            break;
        case 1:
            std::cout << "   __ \n  |  |\n  O  |\n     |\n     |\n     |\n ____|\n|____|" << std::endl;
            break;
        case 2:
            std::cout << "   __ \n  |  |\n  O  |\n  |  |\n  |  |\n     |\n ____|\n|____|" << std::endl;
            break;
        case 3:
            std::cout << "   __ \n  |  |\n  O  |\n /|  |\n  |  |\n     |\n ____|\n|____|" << std::endl;
            break;
        case 4:
            std::cout << "   __ \n  |  |\n  O  |\n /|\\ |\n  |  |\n     |\n ____|\n|____|" << std::endl;
            break;
        case 5:
            std::cout << "   __ \n  |  |\n  O  |\n /|\\ |\n  |  |\n /   |\n ____|\n|____|" << std::endl;
            break;
        case 6:
            std::cout << "   __ \n  |  |\n  O  |\n /|\\ |\n  |  |\n / \\ |\n ____|\n|____|" << std::endl;
            break;
        default:
            std::cout << "      \n      \n      \n      \n      \n      \n ____ \n|____|" << std::endl;
    }
}

bool draw_word(std::string answer, std::string guessed) {
    bool has_blanks = false;
    for (int i=0; i < answer.length(); i++) {
        if ( isalpha(answer[i]) ) {
            if (guessed.find(answer[i]) != std::string::npos) {
                std::cout << answer[i];
            } else {
                std::cout << '_';
                has_blanks = true;
            }
        } else {
            std::cout << answer[i];
        }
    }
    std::cout << std::endl;
    return has_blanks;
}

int main(int argc, char **argv){
    // srand(time(NULL));
    srand(rdtsc());
    bool game_on = true;
    char g_answer;
    bool not_dead;
    int allowed_bad_guesses = 6;

    // TCODConsole::initRoot(80,50,"libtcod C++ tutorial",false);
    // while ( !TCODConsole::isWindowClosed() ) {
    //     TCOD_key_t key;
    //     TCODSystem::checkForEvent(TCOD_EVENT_KEY_PRESS,&key,NULL);
    //     TCODConsole::root->clear();
    //     TCODConsole::root->putChar(40,25,'@');
    //     TCODConsole::flush();
    // }

    while (game_on) {
        //loop
        std::string answer = get_word();
        not_dead = true;
        bool not_valid_guess;
        int bad_guesses = 0;
        bool unguessed_letters;
        char guess;
        std::string guessed;

        while (not_dead) {
            term::clearScreen();
            draw_hangman(bad_guesses);
            unguessed_letters = draw_word(answer, guessed);
            std::cout << "Guesses: " << guessed << std::endl;

            // std::cout << answer << std::endl;

            if (!unguessed_letters) {
                std::cout << "Congratulations, you've saved our man!" << std::endl;
                not_dead = false;
                break;
            }

            if (bad_guesses >= allowed_bad_guesses) {
                std::cout << "Oops, looks like our man's dead!" << std::endl;
                std::cout << "The word you were looking for was: " << answer << std::endl;
                not_dead = false;
                break;
            }

            not_valid_guess = true;
            while (not_valid_guess) {
                std::cin >> guess;
                if (isalpha(guess) && guessed.find(guess) == std::string::npos) {
                    guessed += guess;
                    not_valid_guess = false;
                } else {
                    std::cout << "You've tried that already, or that's not a letter. Guess again!" << std::endl;
                }
            }

            if (answer.find(guess) == std::string::npos) {
                bad_guesses++;
            }
        }

        do {
            std::cout << "Do you want to play again? [y/n]" << std::endl;
            std::cin >> g_answer;
        } while(g_answer != 'y' && g_answer != 'n');

        if (g_answer == 'n') {
            game_on = false;
            std::cout << "Thanks for playing!" << std::endl;
        }
    }
}
