#include "ConsoleUtils.h"
#include <iostream>
#include <sstream>
#include <string>

namespace ConsoleUtils {

int promptMenuChoice() {
    std::string line;
    std::getline(std::cin, line);
    std::istringstream iss(line);
    int choice{};
    if (iss >> choice)
        return choice;
    return -1;
}

} // namespace ConsoleUtils
