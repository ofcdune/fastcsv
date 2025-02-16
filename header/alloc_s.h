#ifndef ALLOC_ALLOC_H
#define ALLOC_ALLOC_H

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define VALNUL(x) if (NULL == (x)) {fprintf(stderr, #x " in function %s is NULL, exiting!\n", __func__);exit(1);}

#define NEW(type, name, amount) \
    type *name = (type *) calloc(amount, sizeof(*name)); \
    VALNUL(name) \

#define NEWSTRUCT(type, name) \
    type *name = (type *) calloc(1, sizeof(*name)); \
    VALNUL(name) \

#define NEWSTRUCTMEM(type, strct, member, amount) \
    strct->member = (type *) calloc(amount, sizeof(*strct->member)); \
    VALNUL(strct->member) \

#define DEBUGPRINT(var, modifier) \
    printf("(in function '%s') DEBUGPRINT >>> " #var " = " #modifier "\n", __func__, var)


#define TIMIT_INIT() \
    clock_t begin, end; \
    double time_spent;  \
    printf("(In function '%s') TIMEIT initiated\n", __func__)

#define TIMEIT(func) \
    printf("(in function '%s') TIMEIT >>> '" #func "'\n", __func__); \
    begin = clock(); \
    func; \
    end = clock(); \
    time_spent = (double)(end - begin) / CLOCKS_PER_SEC; \
    printf("(in function '%s') TIMEIT >>> Time elapsed: %f sec\n\n", __func__, time_spent)

#endif
