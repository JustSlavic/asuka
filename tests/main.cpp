#include <stdio.h>
#include "acf/acf_tests.hpp"
#include "../common/tprint.hpp"


struct point2d
{
    float32 x, y;
};


template <>
int32 tprint_helper<point2d>(char const *fmt, point2d p)
{
    return tprint("({}, {})", p.x, p.y);
}


int main()
{
    point2d p = { 3.12f, 54.032f };

    tprint("FOO\n");
    tprint("{} {}\n", p, 2.0f);

    _set_printf_count_output(1);
#if 1
    test_pair test = {};
    test.filename = string::from("024_newtype_1_custom_arg.acf");

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
