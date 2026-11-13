#include <cstdlib>
#include <ctime>
#include <iostream>

#include "fifteen_menu.h"

int main() {
    std::srand(static_cast<unsigned int>(std::time(nullptr)));
    FifteenMenu menu(std::cin, std::cout);
    menu.run();
    return 0;
}
