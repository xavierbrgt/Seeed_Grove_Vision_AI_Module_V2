#ifndef CV_APP_SETTINGS_H_
#define CV_APP_SETTINGS_H_

#include "python_config.h"
#include "spi_protocol.h"
#include "memory_manage.h"

#if defined(USE_NPU)
#include "tensorflow/lite/c/common.h"
#endif

struct network;

#define COLOR_CHANNEL         1U
#define MAX_RESIZE_IMAGE_SIDE_LENGTTH   160
#define FD_INPUT_TENSOR_WIDTH       MAX_RESIZE_IMAGE_SIDE_LENGTTH
#define FD_INPUT_TENSOR_HEIGHT      MAX_RESIZE_IMAGE_SIDE_LENGTTH

#define CROP_IMAGE_WIDTH        640
#define CROP_IMAGE_HEIGHT       480

#define FM_INPUT_TENSOR_WIDTH     192
#define FM_INPUT_TENSOR_HEIGHT      192


#define IL_INPUT_TENSOR_WIDTH   64
#define IL_INPUT_TENSOR_HEIGHT  64

typedef struct _stream_env {
    uint32_t algo_tick;
    uint32_t capture_image_tick;
    uint32_t function_tick;
    uint32_t systick_1;
    uint32_t systick_2;
    uint32_t loop_cnt_1;
    uint32_t loop_cnt_2;
    uint32_t func_systick_1;
    uint32_t func_systick_2;
    uint32_t func_loop_cnt_1;
    uint32_t func_loop_cnt_2;
} stream_env_t;

#define CG_MALLOC(A) mm_reserve_align((A),0x20);


#define CG_FREE(A) 

#define CPU_CLK (0xfffffful+1)



#define CG_BEFORE_ITERATION \
SystemGetTick(&env->systick_1, &env->loop_cnt_1);

#define CG_AFTER_ITERATION                                                          \
set_model_change_by_uart();                                                         \
{                                                                                   \
    uint32_t systick_1,systick_2,loop_cnt_1,loop_cnt_2;                             \
                                                                                    \
    SystemGetTick(&systick_1, &loop_cnt_1);                                         \
    sensordplib_retrigger_capture();                                                \
                                                                                    \
    SystemGetTick(&systick_2, &loop_cnt_2);                                         \
    env->capture_image_tick = (int32_t)(loop_cnt_2-loop_cnt_1)*CPU_CLK+((int32_t)systick_1-(int32_t)systick_2);\
}

#define CG_BEFORE_NODE_EXECUTION(ID)                          \
if (ID == CANNYEDGE_INTERNAL_ID)                              \
{                                                             \
   SystemGetTick(&env->func_systick_1, &env->func_loop_cnt_1);\
}

#define CG_AFTER_NODE_EXECUTION(ID)                                                                                        \
if (ID == CANNYEDGE_INTERNAL_ID)                                                                                           \
{                                                                                                                          \
    SystemGetTick(&env->func_systick_2, &env->func_loop_cnt_2);                                                            \
    env->function_tick = (int32_t)(&env->func_loop_cnt_2-&env->func_loop_cnt_1)*CPU_CLK+((int32_t)&env->func_systick_1-(int32_t)&env->func_systick_2);\
                                                                                                                           \
}
#endif
