#ifndef EJ_TEST_H
#define EJ_TEST_H

#define EJ_TEST_VERSION 1.0.1

#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#define ejtest_expect_bool(p, a, b) ejtest_expect_bool_file_line((p), (a), (b), #a, #b, __FILE__, __LINE__)
#define ejtest_expect_int(p, a, b) ejtest_expect_int_file_line((p), (a), (b), #a, #b,__FILE__, __LINE__)
#define ejtest_expect_char(p, a, b) ejtest_expect_char_file_line((p), (a), (b), #a, #b,__FILE__, __LINE__)
#define ejtest_expect_float(p, a, b) ejtest_expect_float_file_line((p), (a), (b), #a, #b,__FILE__, __LINE__)
#define ejtest_expect_struct(p, a, b, cmp) ejtest_expect_struct_file_line((p), &(a), &(b), (cmp), sizeof(a), sizeof(b), #a, #b, __FILE__, __LINE__)

//==============================================
// I got the following from RabaDabaDoba at
// https://gist.github.com/RabaDabaDoba/145049536f815903c79944599c6f952a
// I just added the EJTEST_... prefix to not annoy users if they need the names
#define EJTEST_RED "\e[0;31m"
#define EJTEST_GRN "\e[0;32m"
#define EJTEST_YEL "\e[0;33m"
#define EJTEST_CRESET "\e[0m"
//==============================================

void ejtest_print_result(const char * name, bool result)
{
    const char * colour = (result) ? EJTEST_GRN : EJTEST_RED;
    printf("%s== ", colour);
    printf("%-30s", name);
    printf("%s: %s =="EJTEST_CRESET"\n", colour, (result) ? "success" : "FAILURE");
};
bool ejtest_nearly_equal(float a, float b)
{
#define EJ_TEST_EPSILON (1.0/4096.0)
    if (a > b && a - b > EJ_TEST_EPSILON) return false;
    else if (b > a && b - a > EJ_TEST_EPSILON) return false;
    return true;
}
bool ejtest_expect_bool_file_line(
        bool * p,
        bool a, bool b,
        const char * a_str, const char * b_str,
        const char * file, int line)
{
    if (a == b) {
        *p = *p && true;
        return *p;
    }
    const char * a_val = a ? "true" : "false";
    const char * b_val = b ? "true" : "false";
    printf("%s,%d:: Expected equal: |%s|(%s); |%s|(%s)\n", file, line, a_str, a_val, b_str, b_val);
    *p = false;
    return *p;
}
bool ejtest_expect_int_file_line(
        bool * p,
        int a, int b,
        const char * a_str, const char * b_str,
        const char * file, int line)
{
    if (a == b) {
        *p = *p && true;
        return *p;
    }
    printf("%s,%d:: Expected equal: |%s|(%d); |%s|(%d)\n", file, line, a_str, a, b_str, b);
    *p = false;
    return *p;
}

bool ejtest_expect_char_file_line(
        bool * p,
        char a, char b,
        const char * a_str, const char * b_str,
        const char * file, int line)
{
    if (a == b) {
        *p = *p && true;
        return *p;
    }
    //printf("%s,%d:: Expected equal: |%c|; |%c|\n", file, line, a, b);
    printf("%s,%d:: Expected equal: |%s|(%c); |%s|(%c)\n", file, line, a_str, a, b_str, b);
    *p = false;
    return *p;
}
bool ejtest_expect_float_file_line(
        bool * p,
        float a, float b,
        const char * a_str, const char * b_str,
        const char * file, int line)
{
    if (ejtest_nearly_equal(a, b)) {
        *p = *p && true;
        return *p;
    }
    //printf("%s,%d:: Expected nearly equal: |%f|; |%f|\n", file, line, a, b);
    printf("%s,%d:: Expected equal: |%s|(%f); |%s|(%f)\n", file, line, a_str, a, b_str, b);
    *p  = false;
    return *p;
}
bool ejtest_expect_struct_file_line(
        bool * p,
        void * a, void * b,
        bool (*compare)(void *, void*),
        int a_size, int b_size,
        const char * a_str, const char * b_str,
        const char * file, int line)
{
   if (a_size == b_size && (*compare)(a, b)) {
       *p = *p && true;
       return *p;
   }
   //printf("%s,%d:: Expected equal structs\n", file, line);
   printf("%s,%d:: Expected equal: |%s|; |%s|\n", file, line, a_str, b_str);
   *p = false;
   return *p;
};
#endif // EJ_TEST_H
