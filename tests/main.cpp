#include <stdio.h>
#include "acf/acf_tests.hpp"
#include "../common/tprint.hpp"
#include <math/quaternion.hpp>
#include <math/complex.hpp>

#if ASUKA_OS_WINDOWS
#include <windows.h>

// this line is only for older versions of windows headers (pre Win 10):
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
void EnableVTCodes()
{
    // enable ANSI sequences for windows 10:
    HANDLE Console = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD ConsoleMode;
    GetConsoleMode(Console, &ConsoleMode);
    ConsoleMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(Console, ConsoleMode);
}
#endif // ASUKA_OS_WINDOWS


int main()
{
#if ASUKA_OS_WINDOWS
    EnableVTCodes();
#endif // ASUKA_OS_WINDOWS

#if 1
    test_pair test = {};
    test.filename = os::filepath::from("026_type_type_value.acf");

    bool success = run_acf_test(test);
    printf("%s\n", success ? "Success!" : "Failure!");
#else
    auto tests_result = run_acf_tests();
    printf("Successfull tests: %d\n"
           "Failed tests:      %d\n",
           tests_result.successfull,
           tests_result.failed);
#endif

    return 0;
}
