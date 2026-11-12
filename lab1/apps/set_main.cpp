#include <iostream>

#include "ui/set_cli.hpp"

int main() {
    lab1::ui::SetCli cli(std::cin, std::cout);
    cli.run();
    return 0;
}
