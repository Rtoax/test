#include <vos.h>
#include <stdio.h>

int main() {
    
    char *str1 = vos_malloc(64);
    str1 = vos_realloc(str1, 128);

    sprintf(str1, "Hello World.");
    printf("str1 = %s\n", str1);

    vos_free(str1);
    
}
