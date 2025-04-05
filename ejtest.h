#ifndef EJ_TEST_H
#define EJ_TEST_H

#define EJ_TEST_VERSION 1.0.0

#include <stdio.h>
#include <stdbool.h>

#define ejtest_expect_bool(p, a, b) ejtest_expect_bool_file_line((p), (a), (b), __FILE__, __LINE__)
#define ejtest_expect_int(p, a, b) ejtest_expect_int_file_line((p), (a), (b), __FILE__, __LINE__)
#define ejtest_expect_char(p, a, b) ejtest_expect_char_file_line((p), (a), (b), __FILE__, __LINE__)
#define ejtest_expect_float(p, a, b) ejtest_expect_float_file_line((p), (a), (b), __FILE__, __LINE__)

#define ejtest_print_result(name, result) printf("== %-30s: %s ==\n", (name), (result) ? "success" : "FAILURE");

void ejtest_expect_bool_file_line(bool * p, bool a, bool b, const char * file, int line)
{
    if (a == b) {
        *p = *p && true;
        return;
    }
    printf("%s,%d:: Expected equal: |%d|; |%d|\n", file, line, a, b);
    *p = *p && false;
}
void ejtest_expect_int_file_line(bool * p, int a, int b, const char * file, int line)
{
    if (a == b) {
        *p = *p && true;
        return;
    }
    printf("%s,%d:: Expected equal: |%d|; |%d|\n", file, line, a, b);
    *p = false;
}

void ejtest_expect_char_file_line(bool * p, char a, char b, const char * file, int line)
{
    if (a == b) {
        *p = *p && true;
        return;
    }
    printf("%s,%d:: Expected equal: |%c|; |%c|\n", file, line, a, b);
    *p = false;
}
void ejtest_expect_float_file_line(bool * p, float a, float b, const char * file, int line)
{
#define EJ_TEST_EPSILON (1.0/4096.0)
    bool equal = true;
    if (a > b && a - b > EJ_TEST_EPSILON) equal = false;
    else if (b > a && b - a > EJ_TEST_EPSILON) equal = false;

    if (equal) {
        *p = *p && true;
        return;
    }
    printf("%s,%d:: Expected nearly equal: |%f|; |%f|\n", file, line, a, b);
    *p  = false;
}

#endif // EJ_TEST_H
