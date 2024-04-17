#ifndef CV_APP_SETTINGS_H_
#define CV_APP_SETTINGS_H_

//#define USE_NPU
//#define ORIGINAL_APP

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
    uint32_t systick_1;
    uint32_t systick_2;
    uint32_t loop_cnt_1;
    uint32_t loop_cnt_2;
#if defined(USE_NPU)
    TfLiteTensor *fd_input;
    TfLiteTensor *fd_output;
    TfLiteTensor *fd_output2;
    TfLiteTensor *fm_input;
    TfLiteTensor *fm_output;
    TfLiteTensor *fm_output2;
    TfLiteTensor *il_input;
    TfLiteTensor *il_output;
    struct network *fd_net;
#endif
} stream_env_t;

#define CG_MALLOC(A) mm_reserve_align((A),0x20);


#define CG_FREE(A) 

#define CPU_CLK (0xffffff+1)



#define CG_BEFORE_ITERATION \
SystemGetTick(&env->systick_1, &env->loop_cnt_1);

#define CG_AFTER_ITERATION                                                                     \
set_model_change_by_uart();                                                                    \
SystemGetTick(&env->systick_1, &env->loop_cnt_1);                                              \
sensordplib_retrigger_capture();                                                               \
                                                                                               \
SystemGetTick(&env->systick_2, &env->loop_cnt_2);                                              \
env->capture_image_tick = (env->loop_cnt_2-env->loop_cnt_1)*CPU_CLK+(env->systick_1-env->systick_2);

#endif
