#include <stdio.h>

enum {
	enum_A,
	enum_B = sizeof(long double),
};

struct A{};

#define PREDEFINED_VAL 1

#define TEST(val) printf("__builtin_constant_p("#val") is %d\n", __builtin_constant_p(val));

int main() {
	int i = 5;
	printf("__builtin_constant_p(i) is %d\n", __builtin_constant_p(i));
	printf("__builtin_constant_p(PREDEFINED_VAL) is %d\n", __builtin_constant_p(PREDEFINED_VAL));
	printf("__builtin_constant_p(100) is %d\n", __builtin_constant_p(100));

	TEST(__FILE__);
	TEST("Hello RToax");
	TEST("val = %d %s\n");
	TEST(__LINE__);
	TEST(__func__);
	TEST(__FUNCTION__);
	TEST(i+1);
	TEST(enum_A);
	TEST(enum_B);

	char fmt1[] = "Hello %s";
	TEST(fmt1);

	const char fmt2[] = "Hello %s";
	TEST(fmt2);

	const char const fmt3[] = "Hello %s";
	TEST(fmt3);

	// 语句不合法
	//const char const fmt4 const[] = "Hello %s";
	//TEST(fmt4);

	TEST(sizeof(i));
	TEST(sizeof(struct A));

	return 0;
}
