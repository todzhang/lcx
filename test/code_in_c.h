#ifndef __CODE_IN_C_H__
#define __CODE_IN_C_H__

typedef struct c_struct_t{
    char *p_c_pointer;
    int(*p_c_func)(struct c_struct_t*);
}c_struct_t;

int c_func_caller(c_struct_t* p_c_struct);

#endif
