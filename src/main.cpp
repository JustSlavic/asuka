#include <windows.h>


int WINAPI
WinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR lpCmdLine,
    int nCmdShow)
{
    MessageBoxA(0, "Hello world!", "GL2 Message box", MB_OK);

    return 0;
}
