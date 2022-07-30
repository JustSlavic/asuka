#include <stdio.h>
#include "acf/acf_tests.hpp"


int main()
{
#if 0
    bool success = run_acf_test(string::from("positive\\008_many_types.acf"));
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
