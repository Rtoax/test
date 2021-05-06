#include <stdio.h>
#include "list.h"


#define offsetof(TYPE, member)  ((unsigned long)&((typeof(TYPE)*)0)->member)

struct test_data{
    int id;
    struct llist_node llist;
};

static void test_llist() {
    struct test_data tests[] = {
        {1, NULL},
        {2, NULL},
    };
        printf("%d\n", offsetof(struct test_data, llist));
}

int main() {
    test_llist();
    
}
