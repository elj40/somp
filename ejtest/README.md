## ejtest
An extremely simple testing tool that I made to use for test driven development

## Example
```c
#include "ejtest.h"
void testDynamicArrayAppend()
{
    // Initialize a bool for ejtest to use
    // Make sure it is set to true
    bool R = true;
    Ints ints = {0};
    for (int i = 0; i < 100; i++)
    {
        DynamicArrayAppend(&ints, i);
        // Checks if values are equal, if not outputs message and updates R
        ejtest_expect_bool(&R, ints.count <= ints.capacity, true);
        ejtest_expect_int( &R, ints.items[i], i);
    }

    free(ints.items);
    // Prints status of the test
    ejtest_print_result("testDynamicArrayAppend", R);
}
```

```
Example output:
somp_tester.c,229:: Expected equal: |0|; |1|
== testFloatComparison           : FAILURE ==
== testLinkedLists               : success ==
somp_tester.c,127:: Expected nearly equal: |6.000000|; |6.100000|
== testWallReaction              : FAILURE ==
== testDynamicArrayAppend        : success ==
```
