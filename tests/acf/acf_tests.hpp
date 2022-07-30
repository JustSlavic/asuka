#pragma once

// Project specific headers
#include <defines.hpp>
#include <os.hpp>

// ACF implementation
#define ACF_LIB_IMPLEMENTATION
#include <acf/acf.hpp>

// Standard headers
#include <stdio.h>

// Windows
#include <windows.h>


GLOBAL WORD TextFormatting_Regular = 7;
GLOBAL WORD TextFormatting_Red = 4;
GLOBAL WORD TextFormatting_Green = 2;


void SetTextFormatting(WORD Formatting)
{
    PERSIST HANDLE Console = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(Console, Formatting);
}


struct test_stats
{
    uint32 successfull;
    uint32 failed;
};


bool run_acf_test(string filename)
{
    printf("File %-30s:  ", filename.data);

    // @todo: Change it to tprint() or tformat() when they are ready!
    char filename_buffer[256];
    memory::set(filename_buffer, 0, sizeof(filename_buffer));
    sprintf(filename_buffer, "C:\\Projects\\asuka\\tests\\acf\\%s", filename.data);

    auto content = os::load_entire_file(filename_buffer);

    acf parsed_acf;
    b32 successfull = parse_acf(string::from(content), &parsed_acf);
    if (successfull)
    {
        SetTextFormatting(TextFormatting_Green);
        printf("Successfull!\n");
        SetTextFormatting(TextFormatting_Regular);

        acf_print(parsed_acf);
        osOutputDebugString("\n");

        return true;
    }
    else
    {
        SetTextFormatting(TextFormatting_Red);
        printf("Failed!\n");
        SetTextFormatting(TextFormatting_Regular);
        return false;
    }
}


test_stats run_acf_tests()
{
    test_stats result = {};

    WIN32_FIND_DATAA FoundFile = {};
    HANDLE FileHandle = FindFirstFile("C:\\Projects\\asuka\\tests\\acf\\positive\\*", &FoundFile);

    if (FileHandle == INVALID_HANDLE_VALUE)
    {
        printf("Filepath is wrong.\n");
    }
    else
    {
        do {
            auto FileAttributes = FoundFile.dwFileAttributes;
            if (FileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                // Skip directories.
            }
            else
            {
                char filename_buffer[256];
                memory::set(filename_buffer, 0, sizeof(filename_buffer));
                sprintf(filename_buffer, "positive\\%s", FoundFile.cFileName);

                auto filename = string::from(filename_buffer);
                if ((filename[filename.size - 4] != '.') ||
                    (filename[filename.size - 3] != 'a') ||
                    (filename[filename.size - 2] != 'c') ||
                    (filename[filename.size - 1] != 'f'))
                {
                    continue;
                }

                if (run_acf_test(filename))
                {
                    result.successfull += 1;
                }
                else
                {
                    result.failed += 1;
                }
            }
        } while (FindNextFile(FileHandle, &FoundFile));

        if (GetLastError() != ERROR_NO_MORE_FILES)
        {
            printf("Could not real whole directory for some reason.\n");
        }

        FindClose(FileHandle);
    }
    return result;
}
