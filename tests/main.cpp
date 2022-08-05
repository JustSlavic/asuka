#include <stdio.h>
#include "acf/acf_tests.hpp"
#include "../common/tprint.hpp"


struct point2d
{
    float32 x, y;
};


template <>
void tprint_helper<point2d>(point2d p)
{
    tprint("({}, {})", p.x, p.y);
}


int main()
{
#if 0
    point2d p = { 3.12f, 54.032f };

    int8  a = 12;
    int16 b = 256;
    int32 c = 204234;
    int64 d = 3000000000;

    float32 f = 0.4123f;
    float64 ff = 0.4123;

    uint8  g = (uint8)  -1;
    uint16 h = (uint16) -1;
    uint32 i = (uint32) -1;
    uint64 j = (uint64) -1;

    tprint(" int8:  {}\n int16: {}\n int32: {}\n int64: {}\n", a, b, c, d);
    tprint("uint8:  {}\nuint16: {}\nuint32: {}\nuint64: {}\n", g, h, i, j);
    tprint("float32: {}\nfloat64: {}\n", f, ff);

    tprint("FOO\n");
    tprint("{} {}\n", p, 2.0f);

    _set_printf_count_output(1);
    test_pair test = {};
    test.filename = os::filepath::from("024_type_value.acf");

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
