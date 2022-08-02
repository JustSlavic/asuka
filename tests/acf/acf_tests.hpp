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


struct test_pair
{
    string filename;
    acf correct;
};

struct test_stats
{
    uint32 successfull;
    uint32 failed;
};


bool run_acf_test(test_pair test)
{
    printf("%.*s: ", (int) test.filename.size, test.filename.data);

    // @todo: Change it to tprint() or tformat() when they are ready!
    char filename_buffer[256];
    memory::set(filename_buffer, 0, sizeof(filename_buffer));
    sprintf(filename_buffer, "C:\\Projects\\asuka\\tests\\acf\\positive\\%s", test.filename.data);

    auto content = string::from(os::load_entire_file(filename_buffer));

    acf parsed_acf;
    b32 successfull = parse_acf(content, &parsed_acf);
    if (successfull)
    {
        acf_print(parsed_acf);
        osOutputDebugString("\n");
        // successfull = (parsed_acf == test.correct);
    }

    if (successfull)
    {
        SetTextFormatting(TextFormatting_Green);
        printf("Ok\n");
        SetTextFormatting(TextFormatting_Regular);
    }
    else
    {
        SetTextFormatting(TextFormatting_Red);
        printf("Fail\n");
        SetTextFormatting(TextFormatting_Regular);
    }

    return successfull;
}


test_stats run_acf_tests()
{


    test_pair tests[] =
    {
        { string::from("001_empty_object.acf"), acf_null() },
        { string::from("002_null_value.acf"), acf_null() },
        { string::from("003_boolean_value.acf"), acf_null() },
        { string::from("004_integer_value.acf"), acf_null() },
        { string::from("005_floating_value.acf"), acf_null() },
        { string::from("006_string_value.acf"), acf_null() },
        { string::from("007_comments.acf"), acf_null() },
        { string::from("008_many_types.acf"), acf_null() },
        { string::from("009_optional_semicolons.acf"), acf_null() },
        { string::from("010_trailing_comma.acf"), acf_null() },
        { string::from("011_optional_commas.acf"), acf_null() },
        { string::from("012_optional_top_braces.acf"), acf_null() },
        { string::from("013_newtype_0_args.acf"), acf_null() },
        { string::from("014_newtype_1_null_arg.acf"), acf_null() },
        { string::from("015_newtype_1_bool_arg.acf"), acf_null() },
        { string::from("016_newtype_1_int_arg.acf"), acf_null() },
        { string::from("017_newtype_1_float_arg.acf"), acf_null() },
        { string::from("018_newtype_1_string_arg.acf"), acf_null() },
        { string::from("019_newtype_1_array_arg.acf"), acf_null() },
        { string::from("020_newtype_1_object_arg.acf"), acf_null() },
        { string::from("021_newtype_2_args.acf"), acf_null() },
        { string::from("022_newtype_3_args.acf"), acf_null() },
        { string::from("023_newtype_4_args.acf"), acf_null() },
        { string::from("024_newtype_1_custom_arg.acf"), acf_null() },
    };

    test_stats result = {};
    for (int test_index = 0; test_index < ARRAY_COUNT(tests); test_index++)
    {
        if (run_acf_test(tests[test_index]))
        {
            result.successfull += 1;
        }
        else
        {
            result.failed += 1;
        }
    }

    // WIN32_FIND_DATAA FoundFile = {};
    // HANDLE FileHandle = FindFirstFile("C:\\Projects\\asuka\\tests\\acf\\positive\\*", &FoundFile);

    // if (FileHandle == INVALID_HANDLE_VALUE)
    // {
    //     printf("Filepath is wrong.\n");
    // }
    // else
    // {
    //     do {
    //         auto FileAttributes = FoundFile.dwFileAttributes;
    //         if (FileAttributes & FILE_ATTRIBUTE_DIRECTORY)
    //         {
    //             // Skip directories.
    //         }
    //         else
    //         {
    //             char filename_buffer[256];
    //             memory::set(filename_buffer, 0, sizeof(filename_buffer));
    //             sprintf(filename_buffer, "positive\\%s", FoundFile.cFileName);

    //             auto filename = string::from(filename_buffer);
    //             if ((filename[filename.size - 4] != '.') ||
    //                 (filename[filename.size - 3] != 'a') ||
    //                 (filename[filename.size - 2] != 'c') ||
    //                 (filename[filename.size - 1] != 'f'))
    //             {
    //                 continue;
    //             }

    //             if (run_acf_test(filename))
    //             {
    //                 result.successfull += 1;
    //             }
    //             else
    //             {
    //                 result.failed += 1;
    //             }
    //         }
    //     } while (FindNextFile(FileHandle, &FoundFile));

    //     if (GetLastError() != ERROR_NO_MORE_FILES)
    //     {
    //         printf("Could not real whole directory for some reason.\n");
    //     }

    //     FindClose(FileHandle);
    // }
    return result;
}
