#include <iostream>

#include "set_menu.h"

int main() {
    SetMenu menu(std::cin, std::cout);
    menu.run();
    return 0;
}
