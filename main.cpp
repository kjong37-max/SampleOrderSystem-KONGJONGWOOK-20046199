#define NOMINMAX
#include <Windows.h>
#include <gtest/gtest.h>
#include "Controller/MainController.h"

// 실행 방법:
//   SampleOrderSystem.exe          → 애플리케이션 실행
//   SampleOrderSystem.exe --test   → gMock 단위 테스트 실행
int main(int argc, char** argv) {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    for (int i = 1; i < argc; ++i) {
        if (std::string(argv[i]) == "--test") {
            ::testing::InitGoogleTest(&argc, argv);
            return RUN_ALL_TESTS();
        }
    }

    MainController controller;
    controller.run();
    return 0;
}
