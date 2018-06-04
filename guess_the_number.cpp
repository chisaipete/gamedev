#include <iostream>
#include <cstdlib>
#include <ctime>

int main(int argc, char **argv) {
    srand(time(NULL));

    bool game_on = true;

    int answer;
    int guess;
    char g_answer;

    while(game_on) {

        answer = rand() % 100;
        guess = 0;
        std::cout << "I'm thinking of a number between 1 and 100, take a guess." << std::endl;

        while (guess != answer) {

            std::cin >> guess;
        
            if (guess < answer) {
                std::cout << "A little higher..." << std::endl;
            } else if (guess > answer) {
                std::cout << "A little lower..." << std::endl;
            }
        }

        std::cout << "Great job! " << answer << " was my number!" << std::endl;
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
