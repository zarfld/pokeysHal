#ifndef __HAL_H
#define __HAL_H

#include <stdint.h>
#include <stddef.h>

/* Mock HAL header for LinuxCNC */
typedef void * hal_comp_t;
typedef void * hal_pin_t;
typedef void * hal_param_t;
typedef void * hal_sig_t;

typedef float hal_float_t;
typedef int32_t hal_s32_t;
typedef uint32_t hal_u32_t;
typedef int64_t hal_s64_t;
typedef uint64_t hal_u64_t;
typedef int8_t hal_bit_t;

#define HAL_IN 1
#define HAL_OUT 2
#define HAL_IO 3

#define HAL_FLOAT 4
#define HAL_S32 5
#define HAL_U32 6
#define HAL_S64 7
#define HAL_U64 8
#define HAL_BIT 9

#define MAX_HAL_NAME 64
#define HAL_NAME_LEN 64

void *hal_malloc(size_t size);
void hal_free(void *ptr);

int hal_init(const char *comp_name);
int hal_exit(int comp_id);
int hal_ready(int comp_id);
int hal_pin_float_new(const char *name, int dir, hal_float_t **data_ptr, int comp_id);
int hal_pin_s32_new(const char *name, int dir, hal_s32_t **data_ptr, int comp_id);
int hal_pin_u32_new(const char *name, int dir, hal_u32_t **data_ptr, int comp_id);
int hal_pin_bit_new(const char *name, int dir, hal_bit_t **data_ptr, int comp_id);
int hal_pin_float_newf(int dir, hal_float_t **data_ptr, int comp_id, const char *fmt, ...);
int hal_pin_s32_newf(int dir, hal_s32_t **data_ptr, int comp_id, const char *fmt, ...);
int hal_pin_u32_newf(int dir, hal_u32_t **data_ptr, int comp_id, const char *fmt, ...);
int hal_pin_bit_newf(int dir, hal_bit_t **data_ptr, int comp_id, const char *fmt, ...);
int hal_param_float_new(const char *name, int dir, hal_float_t **data_ptr, int comp_id);
int hal_param_s32_new(const char *name, int dir, hal_s32_t **data_ptr, int comp_id);
int hal_param_u32_new(const char *name, int dir, hal_u32_t **data_ptr, int comp_id);

#endif
