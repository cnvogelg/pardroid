#ifndef TEST_BUFFER_H
#define TEST_BUFFER_H

struct test_buffer_param {
    UWORD size;
    UWORD bias;
    UWORD add_size;
    UWORD sub_size;
};
typedef struct test_buffer_param test_buffer_param_t;

extern ULONG test_buffer_get_default_size(test_buffer_param_t *param);
extern ULONG test_buffer_get_adjusted_size(ULONG size, test_buffer_param_t *param);
extern void test_buffer_fill(ULONG size, UBYTE *mem, test_buffer_param_t *bp, test_param_t *p);
extern int test_buffer_validate_(ULONG size, UBYTE *mem, test_buffer_param_t *bp, test_param_t *p);
extern ULONG test_buffer_check(ULONG size, UBYTE *mem1, UBYTE *mem2);

extern UBYTE *test_buffer_alloc(ULONG size, test_param_t *p);
extern void test_buffer_free(UBYTE *buf);

#endif
