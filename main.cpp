#define NOMINMAX
#include <Windows.h>
#include "Controller/MainController.h"

#ifdef _DEBUG
#include <gmock/gmock.h>
#endif

int main(int argc, char** argv) {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

#ifdef _DEBUG
    ::testing::InitGoogleMock();
    return RUN_ALL_TESTS();
#else
    MainController controller;
    controller.run();
    return 0;
#endif
}
