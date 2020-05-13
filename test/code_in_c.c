#include "code_in_c.h"
#include <stdio.h>

int c_func(c_struct_t *p_c_struct)
{
    printf("%s", p_c_struct->p_c_pointer);
    return 0;
}

int c_func_caller(c_struct_t* p_c_struct)
{
    char words[40] = "hello, gmock test c code";
    char *p_words = words;
    int ret;
    p_c_struct->p_c_pointer = p_words;
    ret = p_c_struct->p_c_func(p_c_struct);
    return ret;
}

/*
void main(void)
{
    c_struct_t c_struct;
    c_struct.p_c_func = c_func;
    c_func_caller(&c_struct);
}
*/
