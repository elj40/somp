#ifndef EJ_TEST_H
#define EJ_TEST_H

#define EJ_TEST_VERSION 1.0.0

#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#define ejtest_expect_bool(p, a, b) ejtest_expect_bool_file_line((p), (a), (b), __FILE__, __LINE__)
#define ejtest_expect_int(p, a, b) ejtest_expect_int_file_line((p), (a), (b), __FILE__, __LINE__)
#define ejtest_expect_char(p, a, b) ejtest_expect_char_file_line((p), (a), (b), __FILE__, __LINE__)
#define ejtest_expect_float(p, a, b) ejtest_expect_float_file_line((p), (a), (b), __FILE__, __LINE__)
#define ejtest_expect_struct(p, a, b, cmp) ejtest_expect_struct_file_line((p), &(a), &(b), sizeof(a), sizeof(b), (cmp), __FILE__, __LINE__)

#define ejtest_print_result(name, result) printf("== %-30s: %s ==\n", (name), (result) ? "success" : "FAILURE");

bool ejtest_nearly_equal(float a, float b)
{
#define EJ_TEST_EPSILON (1.0/4096.0)
    if (a > b && a - b > EJ_TEST_EPSILON) return false;
    else if (b > a && b - a > EJ_TEST_EPSILON) return false;
    return true;
}
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
    if (ejtest_nearly_equal(a, b)) {
        *p = *p && true;
        return;
    }
    printf("%s,%d:: Expected nearly equal: |%f|; |%f|\n", file, line, a, b);
    *p  = false;
}
void ejtest_expect_struct_file_line(bool * p, void * a, void * b, int a_size, int b_size, bool (*compare)(void *, void*), const char * file, int line)
{
   if (a_size == b_size && (*compare)(a, b)) {
       *p = *p && true;
       return;
   }
   printf("%s,%d:: Expected equal structs\n", file, line);
   *p = false;
};
#endif // EJ_TEST_H
