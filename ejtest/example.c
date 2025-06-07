#include "ejtest.h"

void testBools()
{
    bool R = true;
    ejtest_expect_bool(&R, 1 == 1, true);
    ejtest_expect_bool(&R, 1 == 1, false);
    ejtest_expect_bool(&R, 1 == 2, false);
    ejtest_expect_bool(&R, 1 == 2, true);

    ejtest_print_result("testBools", R);
}
void testInts()
{
    bool R = true;
    ejtest_expect_int(&R, 1, 1);
    ejtest_expect_int(&R, 1, 2);

    ejtest_print_result("testInts", R);
}
void testFloats()
{
    bool R = true;
    ejtest_expect_float(&R, 1.0, 1.0);
    ejtest_expect_float(&R, 1.0, 1.01);

    ejtest_print_result("testFloats", R);
}
void testChars()
{
    bool R = true;
    ejtest_expect_char(&R, 'a', 'a');
    ejtest_expect_char(&R, 'a', 'b');

    ejtest_print_result("testChars", R);
}

typedef struct {
    bool a;
    int b;
    float c;
    char d;
} ExampleStruct;

bool compare_example(void * a, void * b)
{
    ExampleStruct * A = (ExampleStruct *) a;
    ExampleStruct * B = (ExampleStruct *) b;
    return A->a == B->a &&
        A->b == B->b &&
        ejtest_nearly_equal(A->c, B->c) &&
        A->d == B->d;
}

void testStructs()
{
    bool R = true;
    ExampleStruct A, B;
    A = (ExampleStruct){0};
    B = (ExampleStruct){0};

    ejtest_expect_struct(&R, A, B, compare_example);

    A = (ExampleStruct){0};
    B = (ExampleStruct){0};
    A.a = true;
    B.a = false;
    ejtest_expect_struct(&R, A, B, compare_example);

    A = (ExampleStruct){0};
    B = (ExampleStruct){0};
    A.b = 1;
    B.b = 0;
    ejtest_expect_struct(&R, A, B, compare_example);

    A = (ExampleStruct){0};
    B = (ExampleStruct){0};
    A.c = 1.0;
    B.c = 1.01;
    ejtest_expect_struct(&R, A, B, compare_example);
    
    A = (ExampleStruct){0};
    B = (ExampleStruct){0};
    A.d = 'a';
    B.d = 'b';
    ejtest_expect_struct(&R, A, B, compare_example);

    ejtest_print_result("testStructs", R);
}

void testSuccess()
{
    bool R = true;
    ejtest_print_result("testSuccess", R);
};

int main()
{

    testBools();
    testInts();
    testFloats();
    testChars();
    testStructs();
    testSuccess();
    testSuccess();
    testSuccess();
    testSuccess();

    return 0;
};
