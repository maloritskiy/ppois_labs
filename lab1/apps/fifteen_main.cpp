#include <iostream>

#include "ui/fifteen_cli.hpp"

int main() {
    lab1::ui::FifteenCli cli(std::cin, std::cout);
    cli.run();
    return 0;
}
