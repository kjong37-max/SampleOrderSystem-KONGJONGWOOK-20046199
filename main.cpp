#define NOMINMAX
#include <Windows.h>
#include "Controller/MainController.h"

int main() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    MainController controller;
    controller.run();
    return 0;
}
