/*
 * memory_manage.h
 *
 *  Created on: Feb 15, 2023
 *      Author: bigcat-himax
 */

#ifndef APP_SCENARIO_APP_cmsis_stream_app_MEMORY_MANAGE_H_
#define APP_SCENARIO_APP_cmsis_stream_app_MEMORY_MANAGE_H_
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif

int mm_set_initial(uint32_t start_addr, uint32_t mm_sz);
uint32_t mm_reserve(uint32_t sz);
uint32_t mm_reserve_align(uint32_t sz, uint32_t alignment_size);

#ifdef __cplusplus
}
#endif
#endif /* APP_SCENARIO_APP_TFLM_FM_FD_MEMORY_MANAGE_H_ */
