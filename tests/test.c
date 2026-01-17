#include <stdio.h>
#include <unistd.h>

#include "../threadpool.h"
#include "../lock.h"
#include "../spin.h"

#include "schedule_and_yield.c"

const char* current_test_group;
int n_failed = 0;
int pass = 0;
int all_passing = 1;

#define PASS pass = 1; return;

void
start_test(const char* test_group)
{
    current_test_group = test_group;
    printf("---+ %s +---\n", test_group);
    n_failed = 0;
}

void
run_test(int (*test)(void), const char* test_name)
{
    if (test()) {
        printf(" ~ (F) %s\n", test_name);
        n_failed += 1;
        all_passing = 0;
    }
}

void
end_test(void)
{
    if (n_failed != 0)
        printf("%s: %d tests failed!\n", current_test_group, n_failed);

    else
        printf("%s: all test passed!\n", current_test_group);

}

#define RUN_TEST(test) run_test(test, #test);

int
main(void)
{
    thpl_init(-1);

    start_test("Schedule & Yield");
    RUN_TEST(say1);
    RUN_TEST(say2);
    RUN_TEST(say3);
    RUN_TEST(say4);
    RUN_TEST(say5);
    RUN_TEST(say6)
    end_test();

    return all_passing ? 0 : -1;
}
