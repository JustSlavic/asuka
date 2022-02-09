#include <stdio.h>
#include <stdlib.h>

#include <defines.hpp>

#include <windows.h>
#include <psapi.h>


int main() {
#if 1
    {
        DWORD processes[2048] {};
        DWORD processes_size = 0; // in bytes

        if (EnumProcesses(processes, sizeof(processes) * sizeof(DWORD), &processes_size)) {
            // Success.

            u32 processes_count = processes_size / sizeof(DWORD);

            for (uint32 i = 0; i < processes_count; i++) {
                DWORD pid = processes[i];

                char process_name[MAX_PATH] = TEXT("<unknown>");
                HANDLE process = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
                if (process != NULL) {
                    defer { CloseHandle(process); };
                    HMODULE modules[1024] {};
                    DWORD modules_size = 0; // in bytes
                    u32 modules_count = 0;

                    if (EnumProcessModules(process, modules, sizeof(modules) * sizeof(HMODULE), &modules_size)) {
                        modules_count = modules_size / sizeof(HMODULE);

                        GetModuleBaseName(process, modules[0], process_name, sizeof(process_name));
                    }


                    if (strcmp(process_name, "debuggee.exe") == 0) {
                        printf("%30s  (PID: %u) (%d modules)\n", process_name, pid, modules_count);

                        if (DebugActiveProcess(pid)) {
                            printf("Debugging PID: %u\n", pid);

                            // DebugActiveProcessStop(pid);

                            Sleep(10000);
                        }
                    }
                }
            }
        } else {
            printf("Failed to retrieve process list.\n");
            return 1;
        }
    }
#endif

    

    return 0;
}
