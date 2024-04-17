#include <cstdio>
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include "cmsis_stream_init.h"

#if defined(USE_NPU)
#include "ethosu_driver.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/schema/schema_generated.h"
#include "tensorflow/lite/c/common.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"

#include "yolo_postprocessing.h"
#include "pose_processing.h"
#else
#include <string>
#include <forward_list>
#endif

#include "xprintf.h"


#if defined(USE_NPU)
#ifdef TRUSTZONE_SEC
#define U55_BASE    BASE_ADDR_APB_U55_CTRL_ALIAS
#else
#ifndef TRUSTZONE
#define U55_BASE    BASE_ADDR_APB_U55_CTRL_ALIAS
#else
#define U55_BASE    BASE_ADDR_APB_U55_CTRL
#endif
#endif
#endif

using namespace std;

namespace {

#if defined(USE_NPU)
//constexpr int tensor_arena_size_second_model_tail_size = 736 ;
constexpr int tensor_arena_model_tail_size = 1224;//568;
constexpr int tensor_arena_size = 460*1024;//435*1024;

static uint32_t tensor_arena=0;

struct ethosu_driver ethosu_drv; /* Default Ethos-U device driver */
tflite::MicroInterpreter *fd_int_ptr=nullptr;
TfLiteTensor *fd_input, *fd_output, *fd_output2;

tflite::MicroInterpreter *fm_int_ptr=nullptr;
TfLiteTensor *fm_input, *fm_output, *fm_output2;

tflite::MicroInterpreter *il_int_ptr=nullptr;
TfLiteTensor *il_input, *il_output;

network fd_net;
static uint32_t g_fd_fm_init = 0, g_image_mapping_init = 0;

static tflite::MicroMutableOpResolver<2> op_resolver;
#else
static uint32_t g_fd_fm_init = 0, g_image_mapping_init = 0;

#endif

};

#if defined(USE_NPU)

static network yolo_post_processing_init(TfLiteTensor* out_ten, TfLiteTensor* out2_ten)
{
    int input_w = FD_INPUT_TENSOR_WIDTH;
    int input_h = FD_INPUT_TENSOR_HEIGHT;
    int num_classes = 1;
    int num_branch = 2;
    int topN = 0;
    branch* branchs = (branch*)calloc(num_branch, sizeof(branch));
    static float anchor1[] = {38,63, 66,100, 121,163};
    static float anchor2[] = {6,10 , 12,22, 23,39};
    branchs[0] = create_brach(input_w/32, 3, anchor1, out_ten->data.int8, out_ten->bytes , out_ten->params.scale, out_ten->params.zero_point);
    branchs[1] = create_brach(input_w/16, 3, anchor2, out2_ten->data.int8,  out2_ten->bytes, out2_ten->params.scale, out2_ten->params.zero_point);

    network net = creat_network(input_w, input_h, num_classes, num_branch, branchs,topN);
#ifdef FD_FL_DEBUG
    xprintf("is_post_processing_initialized = true\n");

    xprintf("input_w:%d,input_h:%d,num_classes:%d,num_branch:%d,branchs:%x\n",net.input_w,net.input_h,net.num_classes,net.num_branch,net.branchs);
    xprintf("[branch2]resolution:%d\n",net.branchs[1].resolution,net.branchs[1].num_box);
    xprintf("[branch]addr:%x:%x\n",net.branchs[0],net.branchs[1]);
#endif
    return net;
}

static void _arm_npu_irq_handler(void)
{
    /* Call the default interrupt handler from the NPU driver */
    ethosu_irq_handler(&ethosu_drv);
}

/**
 * @brief  Initialises the NPU IRQ
 **/
static void _arm_npu_irq_init(void)
{
    const IRQn_Type ethosu_irqnum = (IRQn_Type)U55_IRQn;

    /* Register the EthosU IRQ handler in our vector table.
     * Note, this handler comes from the EthosU driver */
    EPII_NVIC_SetVector(ethosu_irqnum, (uint32_t)_arm_npu_irq_handler);

    /* Enable the IRQ */
    NVIC_EnableIRQ(ethosu_irqnum);

}

static int _arm_npu_init(bool security_enable, bool privilege_enable)
{
    int err = 0;

    /* Initialise the IRQ */
    _arm_npu_irq_init();

    /* Initialise Ethos-U55 device */
    const void * ethosu_base_address = (void *)(U55_BASE);

    if (0 != (err = ethosu_init(
                            &ethosu_drv,             /* Ethos-U driver device pointer */
                            ethosu_base_address,     /* Ethos-U NPU's base address. */
                            NULL,       /* Pointer to fast mem area - NULL for U55. */
                            0, /* Fast mem region size. */
                            security_enable,                       /* Security enable. */
                            privilege_enable))) {                   /* Privilege enable. */
        xprintf("failed to initalise Ethos-U device\n");
            return err;
        }

    xprintf("Ethos-U55 device initialised\n");

    return 0;
}
#endif 

int cmsis_stream_init(stream_env_t *env,bool security_enable, bool privilege_enable, uint32_t fd_model_addr, uint32_t fm_model_addr, uint32_t il_model_addr) {
    int ercode = 0;
#if defined(USE_NPU)
    //set memory allocation to tensor_arena, resized_img, crop_img, crop_eye
    tensor_arena = mm_reserve_align(tensor_arena_size,0x20); //435kb

    //for(int i = 0;i<tensor_arena_size;i++)
    //  tensor_arena[i] = 0;

    if(_arm_npu_init(security_enable, privilege_enable)!=0)
        return -1;

    static const tflite::Model*model = tflite::GetModel((const void *)fd_model_addr);
    static const tflite::Model*FM_model = tflite::GetModel((const void *)fm_model_addr);
    static const tflite::Model*IL_model = tflite::GetModel((const void *)il_model_addr);

    if (model->version() != TFLITE_SCHEMA_VERSION) {
        xprintf(
            "[ERROR] FD model's schema version %d is not equal "
            "to supported version %d\n",
            model->version(), TFLITE_SCHEMA_VERSION);
        return false;
    }
    else {
        xprintf("FD model's schema version %d\n", model->version());
    }
    if (FM_model->version() != TFLITE_SCHEMA_VERSION) {
        xprintf(
            "[ERROR] FM model's schema version %d is not equal "
            "to supported version %d\n",
            FM_model->version(), TFLITE_SCHEMA_VERSION);
        return false;
    }
    else {
        xprintf("FM model's schema version %d\n", FM_model->version());
    }
    if (IL_model->version() != TFLITE_SCHEMA_VERSION) {
        xprintf(
            "[ERROR] IL model's schema version %d is not equal "
            "to supported version %d\n",
            IL_model->version(), TFLITE_SCHEMA_VERSION);
        return false;
    }
    else {
        xprintf("IL model's schema version %d\n", IL_model->version());
    }

    static tflite::MicroErrorReporter micro_error_reporter;

    if(g_fd_fm_init==0) {
        if (kTfLiteOk != op_resolver.AddEthosU()){
            xprintf("Failed to add Arm NPU support to op resolver.");
            return false;
        }
        if(kTfLiteOk != op_resolver.AddPad()){
            xprintf("Failed to add Padding support to op resolver.");
            return false;
        }
        
    }

    static tflite::MicroInterpreter fd_static_interpreter(model, op_resolver, (uint8_t*)tensor_arena, tensor_arena_size, &micro_error_reporter);
    static tflite::MicroInterpreter fm_static_interpreter(FM_model, op_resolver, (uint8_t*)tensor_arena, tensor_arena_size-tensor_arena_model_tail_size, &micro_error_reporter);
    static tflite::MicroInterpreter il_static_interpreter(IL_model, op_resolver, (uint8_t*)tensor_arena, tensor_arena_size-(tensor_arena_model_tail_size*2), &micro_error_reporter);
    if(fd_static_interpreter.AllocateTensors()!= kTfLiteOk) {
        return false;
    }
    if(fm_static_interpreter.AllocateTensors()!= kTfLiteOk) {
        return false;
    }
    if(il_static_interpreter.AllocateTensors()!= kTfLiteOk) {
        return false;
    }

    fd_int_ptr = &fd_static_interpreter;
    env->fd_input = fd_static_interpreter.input(0);
    env->fd_output = fd_static_interpreter.output(0);
    env->fd_output2 = fd_static_interpreter.output(1);

    env->fd_net = yolo_post_processing_init(env->fd_output, env->fd_output2);

    fm_int_ptr = &fm_static_interpreter;
    env->fm_input = fm_static_interpreter.input(0);
    env->fm_output = fm_static_interpreter.output(0);
    env->fm_output2 = fm_static_interpreter.output(1);


    il_int_ptr = &il_static_interpreter;
    env->il_input = il_static_interpreter.input(0);
    env->il_output = il_static_interpreter.output(0);
#endif

    xprintf("initial done\n");


#if HD_RECT_DEBUG
    g_spi_master_initial_status = 0;
#endif
    g_fd_fm_init = 1;
    return ercode;
}


int cmsis_stream_deinit()
{
#if defined(USE_NPU)

    free(fd_net.branchs);
    /*ethosu_deinit(&ethosu_drv);

    fd_int_ptr->Reset();
    fl_int_ptr->Reset();
    el_int_ptr->Reset();*/
#endif

    return 0;
}

