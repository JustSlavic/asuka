#include <stdio.h>

#include <windows.h>


int main() {
    int i = 0;
    while(true) {
        printf("Hello, World! %d\n", i++);

        Sleep(100);
    }

    return 0;
}
