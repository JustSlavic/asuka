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


struct test_pair
{
    os::filepath filename;
    acf correct;
};

struct test_stats
{
    uint32 successfull;
    uint32 failed;
};


bool run_acf_test(test_pair test)
{
    tprint("{}: ", test.filename);

    // @todo: Change it to tprint() or tformat() when they are ready!
    char filename_buffer[256];
    memory::set(filename_buffer, 0, sizeof(filename_buffer));
    sprintf(filename_buffer, "C:\\Projects\\asuka\\tests\\acf\\positive\\%s", test.filename.buffer.data);

    auto content = string::from(os::load_entire_file(filename_buffer));

    acf parsed_acf;
    b32 successfull = parse_acf(content, &parsed_acf);

    if (successfull)
    {
        osOutputDebugString("\n");
        tprint("{}\n", parsed_acf);
        osOutputDebugString("\n");

        parsed_acf.dispose();
        // successfull = (parsed_acf == test.correct);
    }

    if (successfull)
    {
        console::set_fg(0, 255, 0);
        printf("Ok\n");
        console::reset_formatting();
    }
    else
    {
        console::set_fg(255, 0, 0);
        printf("Fail\n");
        console::reset_formatting();
    }

    return successfull;
}


test_stats run_acf_tests()
{
    test_pair tests[] =
    {
        { os::filepath::from("001_empty_object.acf"), acf::from() },
        { os::filepath::from("002_null_value.acf"), acf::from() },
        { os::filepath::from("003_boolean_value.acf"), acf::from() },
        { os::filepath::from("004_integer_value.acf"), acf::from() },
        { os::filepath::from("005_floating_value.acf"), acf::from() },
        { os::filepath::from("006_string_value.acf"), acf::from() },
        { os::filepath::from("007_comments.acf"), acf::from() },
        { os::filepath::from("008_many_types.acf"), acf::from() },
        { os::filepath::from("009_optional_semicolons.acf"), acf::from() },
        { os::filepath::from("010_trailing_comma.acf"), acf::from() },
        { os::filepath::from("011_optional_commas.acf"), acf::from() },
        { os::filepath::from("012_optional_top_braces.acf"), acf::from() },
        { os::filepath::from("013_newtype_0_args.acf"), acf::from() },
        { os::filepath::from("014_newtype_1_null_arg.acf"), acf::from() },
        { os::filepath::from("015_newtype_1_bool_arg.acf"), acf::from() },
        { os::filepath::from("016_newtype_1_int_arg.acf"), acf::from() },
        { os::filepath::from("017_newtype_1_float_arg.acf"), acf::from() },
        { os::filepath::from("018_newtype_1_string_arg.acf"), acf::from() },
        { os::filepath::from("019_newtype_1_array_arg.acf"), acf::from() },
        { os::filepath::from("020_newtype_1_object_arg.acf"), acf::from() },
        { os::filepath::from("021_newtype_2_args.acf"), acf::from() },
        { os::filepath::from("022_newtype_3_args.acf"), acf::from() },
        { os::filepath::from("023_newtype_4_args.acf"), acf::from() },
        { os::filepath::from("024_type_value.acf"), acf::from() },
        { os::filepath::from("025_int_to_float_conversion.acf"), acf::from() },
        { os::filepath::from("026_type_type_value.acf"), acf::from() },
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

    return result;
}
