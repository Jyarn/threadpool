#include "../threadpool.h"

/*
 * Checks if function calls are still supported
 */
int
say1(void)
{
    return 0;
}

/*
 * Tests yield with nothing in queue. Expected to not crash and burn
 */
int
say2(void)
{
    thpl_yield();
    return 0;
}

/*
 * Test pushing, yielding, and executing a task. Expected to set passed value
 * to 0
 */
void
_say3(int* r)
{
    *r = 0;
}

int
say3(void)
{
    int r = -1;
    thpl_push((Task) _say3, &r);
    thpl_yield();
    return r;
}

/*
 * Test that main thread is resumed iff there are not other threads to execute
 * Expected order of execution:
 *  1. say4
 *  2. _say4, and exits
 *  3. _say3
 *  4. say4
 */
void
_say4(int* _r)
{
    thpl_push((Task) _say3, _r);
}

int
say4(void)
{
    int r = -1;
    thpl_push((Task) _say4, &r);
    thpl_yield();
    return r;
}

/*
 * Test that thread scheduler is FILO.
 * Expected order of execution:
 *  1. say5
 *  2. _say5
 *  3. say4
 *  4. _say4
 *  5. _say3
 *  6. say5
 */
void
_say5(int* r)
{
    *r = say4();
}

int
say5(void)
{
    int r = -1;
    thpl_push((Task) _say5, &r);
    thpl_yield();
    return r == 0;
}

/*
 * Same as say5
 */
void
_say61(int* r)
{
    *r = 1;
}

void
_say62(int* r)
{
    *r = 2;
}

int
say6(void)
{
    int r = 0;
    thpl_push((Task) _say61, &r);
    thpl_push((Task) _say62, &r);
    thpl_yield();
    return r == 1;
}
