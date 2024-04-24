/*

Generated with CMSIS-Stream python scripts.
The generated code is not covered by CMSIS-Stream license.

The support classes and code are covered by CMSIS-Stream license.

*/


#include <cstdint>
#include "cv_app_settings.h"
#include "cg_status.h"
#include "GenericNodes.h"
#include "AppNodes.h"
#include "cv_scheduler.h"

#if !defined(CHECKERROR)
#define CHECKERROR       if (cgStaticError < 0) \
       {\
         goto errorHandling;\
       }

#endif

#if !defined(CG_MALLOC)
#define CG_MALLOC(A) malloc((A))
#endif 

#if !defined(CG_FREE)
#define CG_FREE(A) free((A))
#endif 

#if !defined(CG_BEFORE_ITERATION)
#define CG_BEFORE_ITERATION
#endif 

#if !defined(CG_AFTER_ITERATION)
#define CG_AFTER_ITERATION
#endif 

#if !defined(CG_BEFORE_SCHEDULE)
#define CG_BEFORE_SCHEDULE
#endif

#if !defined(CG_AFTER_SCHEDULE)
#define CG_AFTER_SCHEDULE
#endif

#if !defined(CG_BEFORE_BUFFER)
#define CG_BEFORE_BUFFER
#endif

#if !defined(CG_BEFORE_FIFO_BUFFERS)
#define CG_BEFORE_FIFO_BUFFERS
#endif

#if !defined(CG_BEFORE_FIFO_INIT)
#define CG_BEFORE_FIFO_INIT
#endif

#if !defined(CG_BEFORE_NODE_INIT)
#define CG_BEFORE_NODE_INIT
#endif

#if !defined(CG_AFTER_INCLUDES)
#define CG_AFTER_INCLUDES
#endif

#if !defined(CG_BEFORE_SCHEDULER_FUNCTION)
#define CG_BEFORE_SCHEDULER_FUNCTION
#endif

#if !defined(CG_BEFORE_NODE_EXECUTION)
#define CG_BEFORE_NODE_EXECUTION(ID)
#endif

#if !defined(CG_AFTER_NODE_EXECUTION)
#define CG_AFTER_NODE_EXECUTION(ID)
#endif



#if !defined(CG_RESTORE_STATE_MACHINE_STATE)
#define CG_RESTORE_STATE_MACHINE_STATE
#endif

#if !defined(CG_SAVE_STATE_MACHINE_STATE)
#define CG_SAVE_STATE_MACHINE_STATE
#endif


CG_AFTER_INCLUDES


using namespace arm_cmsis_stream;

/*

Description of the scheduling. 

*/
static uint8_t schedule[7]=
{ 
4,5,2,1,6,3,0,
};

/*

Internal ID identification for the nodes

*/
#define SERIAL_INTERNAL_ID 0
#define CANNY_INTERNAL_ID 1
#define GAUSSIAN_INTERNAL_ID 2
#define JPEG_INTERNAL_ID 3
#define OV5647_INTERNAL_ID 4
#define TO_GRAY8_INTERNAL_ID 5
#define TO_RGB_INTERNAL_ID 6

/* For callback management */

#define CG_PAUSED_SCHEDULER_ID 1
#define CG_SCHEDULER_NOT_STARTED_ID 2
#define CG_SCHEDULER_RUNNING_ID 3

struct cb_state_t
{
    unsigned long scheduleStateID;
    unsigned long nbSched;
    int status;
    kCBStatus running;
};

static cb_state_t cb_state;


static void init_cb_state()
{
    cb_state.status = CG_SCHEDULER_NOT_STARTED_ID;
    cb_state.nbSched = 0;
    cb_state.scheduleStateID = 0;
    cb_state.running = kNewExecution;
}


CG_BEFORE_FIFO_BUFFERS
/***********

FIFO buffers

************/
#define FIFOSIZE0 115200
#define FIFOSIZE1 76800
#define FIFOSIZE2 153600
#define FIFOSIZE3 153600
#define FIFOSIZE4 230400
#define FIFOSIZE5 76900
#define FIFOSIZE6 1

typedef struct {
uint8_t  *buf0;
uint8_t  *buf1;
uint8_t  *buf2;
} buffers_t;

CG_BEFORE_BUFFER
static buffers_t buffers={0};

int init_buffer_cv_scheduler(stream_env_t *env,
                              struct_algoResult *alg_result,
                              struct_fm_algoResult_with_fps *alg_fm_result)
{
    buffers.buf0 = (uint8_t *)CG_MALLOC(230400 * sizeof(uint8_t));
    if (buffers.buf0==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }
    buffers.buf1 = (uint8_t *)CG_MALLOC(153600 * sizeof(uint8_t));
    if (buffers.buf1==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }
    buffers.buf2 = (uint8_t *)CG_MALLOC(4 * sizeof(uint8_t));
    if (buffers.buf2==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }
    return(CG_SUCCESS);
}

void free_buffer_cv_scheduler(stream_env_t *env,
                              struct_algoResult *alg_result,
                              struct_fm_algoResult_with_fps *alg_fm_result)
{
    if (buffers.buf0!=NULL)
    {
        CG_FREE(buffers.buf0);
    }
    if (buffers.buf1!=NULL)
    {
        CG_FREE(buffers.buf1);
    }
    if (buffers.buf2!=NULL)
    {
        CG_FREE(buffers.buf2);
    }
}


typedef struct {
FIFO<int8_t,FIFOSIZE0,1,0> *fifo0;
FIFO<int8_t,FIFOSIZE1,1,0> *fifo1;
FIFO<int8_t,FIFOSIZE2,1,0> *fifo2;
FIFO<int8_t,FIFOSIZE3,1,0> *fifo3;
FIFO<int8_t,FIFOSIZE4,1,0> *fifo4;
FIFO<int8_t,FIFOSIZE5,1,0> *fifo5;
FIFO<uint32_t,FIFOSIZE6,1,0> *fifo6;
} fifos_t;

typedef struct {
    SendResult<int8_t,76900,uint32_t,1> *Serial;
    MVCannyEdge<int8_t,153600,int8_t,153600> *canny;
    GaussianFilter<int8_t,76800,int8_t,153600> *gaussian;
    JPEGEncoder<int8_t,230400,uint32_t,1,int8_t,76900> *jpeg;
    Camera<int8_t,115200> *ov5647;
    YUVToGray8<int8_t,115200,int8_t,76800> *to_gray8;
    Gray16ToRGB<int8_t,153600,int8_t,230400> *to_rgb;
} nodes_t;

CG_BEFORE_BUFFER
static fifos_t fifos={0};

CG_BEFORE_BUFFER
static nodes_t nodes={0};


int init_cv_scheduler(stream_env_t *env,
                              struct_algoResult *alg_result,
                              struct_fm_algoResult_with_fps *alg_fm_result)
{
init_cb_state();

    CG_BEFORE_FIFO_INIT;
    fifos.fifo0 = new FIFO<int8_t,FIFOSIZE0,1,0>((void*)app_get_raw_addr());
    if (fifos.fifo0==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }
    fifos.fifo1 = new FIFO<int8_t,FIFOSIZE1,1,0>(buffers.buf1);
    if (fifos.fifo1==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }
    fifos.fifo2 = new FIFO<int8_t,FIFOSIZE2,1,0>(buffers.buf0);
    if (fifos.fifo2==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }
    fifos.fifo3 = new FIFO<int8_t,FIFOSIZE3,1,0>(buffers.buf1);
    if (fifos.fifo3==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }
    fifos.fifo4 = new FIFO<int8_t,FIFOSIZE4,1,0>(buffers.buf0);
    if (fifos.fifo4==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }
    fifos.fifo5 = new FIFO<int8_t,FIFOSIZE5,1,0>(buffers.buf1);
    if (fifos.fifo5==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }
    fifos.fifo6 = new FIFO<uint32_t,FIFOSIZE6,1,0>(buffers.buf2);
    if (fifos.fifo6==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }

    CG_BEFORE_NODE_INIT;
    nodes.Serial = new SendResult<int8_t,76900,uint32_t,1>(*(fifos.fifo5),*(fifos.fifo6),env,alg_result,alg_fm_result,320,240);
    if (nodes.Serial==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }
    nodes.canny = new MVCannyEdge<int8_t,153600,int8_t,153600>(*(fifos.fifo2),*(fifos.fifo3),320,240);
    if (nodes.canny==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }
    nodes.gaussian = new GaussianFilter<int8_t,76800,int8_t,153600>(*(fifos.fifo1),*(fifos.fifo2),320,240);
    if (nodes.gaussian==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }
    nodes.jpeg = new JPEGEncoder<int8_t,230400,uint32_t,1,int8_t,76900>(*(fifos.fifo4),*(fifos.fifo6),*(fifos.fifo5),320,240);
    if (nodes.jpeg==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }
    nodes.ov5647 = new Camera<int8_t,115200>(*(fifos.fifo0));
    if (nodes.ov5647==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }
    nodes.to_gray8 = new YUVToGray8<int8_t,115200,int8_t,76800>(*(fifos.fifo0),*(fifos.fifo1),320,240);
    if (nodes.to_gray8==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }
    nodes.to_rgb = new Gray16ToRGB<int8_t,153600,int8_t,230400>(*(fifos.fifo3),*(fifos.fifo4),320,240);
    if (nodes.to_rgb==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }

    return(CG_SUCCESS);

}

void free_cv_scheduler(stream_env_t *env,
                              struct_algoResult *alg_result,
                              struct_fm_algoResult_with_fps *alg_fm_result)
{
    if (fifos.fifo0!=NULL)
    {
       delete fifos.fifo0;
    }
    if (fifos.fifo1!=NULL)
    {
       delete fifos.fifo1;
    }
    if (fifos.fifo2!=NULL)
    {
       delete fifos.fifo2;
    }
    if (fifos.fifo3!=NULL)
    {
       delete fifos.fifo3;
    }
    if (fifos.fifo4!=NULL)
    {
       delete fifos.fifo4;
    }
    if (fifos.fifo5!=NULL)
    {
       delete fifos.fifo5;
    }
    if (fifos.fifo6!=NULL)
    {
       delete fifos.fifo6;
    }

    if (nodes.Serial!=NULL)
    {
        delete nodes.Serial;
    }
    if (nodes.canny!=NULL)
    {
        delete nodes.canny;
    }
    if (nodes.gaussian!=NULL)
    {
        delete nodes.gaussian;
    }
    if (nodes.jpeg!=NULL)
    {
        delete nodes.jpeg;
    }
    if (nodes.ov5647!=NULL)
    {
        delete nodes.ov5647;
    }
    if (nodes.to_gray8!=NULL)
    {
        delete nodes.to_gray8;
    }
    if (nodes.to_rgb!=NULL)
    {
        delete nodes.to_rgb;
    }
}


CG_BEFORE_SCHEDULER_FUNCTION
uint32_t cv_scheduler(int *error,stream_env_t *env,
                              struct_algoResult *alg_result,
                              struct_fm_algoResult_with_fps *alg_fm_result)
{
    int cgStaticError=0;
    uint32_t nbSchedule=0;

if (cb_state.status==CG_PAUSED_SCHEDULER_ID)
   {
      nbSchedule = cb_state.nbSched;

   }


    /* Run several schedule iterations */
    CG_BEFORE_SCHEDULE;
CG_RESTORE_STATE_MACHINE_STATE;
    while(cgStaticError==0)
    {
        /* Run a schedule iteration */
        CG_BEFORE_ITERATION;
        unsigned long id=0;
        if (cb_state.status==CG_PAUSED_SCHEDULER_ID)
        {
            id = cb_state.scheduleStateID;
            cb_state.status = CG_SCHEDULER_RUNNING_ID;
        }
        for(; id < 7; id++)
        {
            CG_BEFORE_NODE_EXECUTION(schedule[id]);
            switch(schedule[id])
            {
                case 0:
                {
                    nodes.Serial->setExecutionStatus(cb_state.running);
                   cgStaticError = nodes.Serial->run();
                }
                break;

                case 1:
                {
                    nodes.canny->setExecutionStatus(cb_state.running);
                   cgStaticError = nodes.canny->run();
                }
                break;

                case 2:
                {
                    nodes.gaussian->setExecutionStatus(cb_state.running);
                   cgStaticError = nodes.gaussian->run();
                }
                break;

                case 3:
                {
                    nodes.jpeg->setExecutionStatus(cb_state.running);
                   cgStaticError = nodes.jpeg->run();
                }
                break;

                case 4:
                {
                    nodes.ov5647->setExecutionStatus(cb_state.running);
                   cgStaticError = nodes.ov5647->run();
                }
                break;

                case 5:
                {
                    nodes.to_gray8->setExecutionStatus(cb_state.running);
                   cgStaticError = nodes.to_gray8->run();
                }
                break;

                case 6:
                {
                    nodes.to_rgb->setExecutionStatus(cb_state.running);
                   cgStaticError = nodes.to_rgb->run();
                }
                break;

                default:
                break;
            }
            CG_AFTER_NODE_EXECUTION(schedule[id]);
            cb_state.running = kNewExecution;
            if (cgStaticError == CG_PAUSED_SCHEDULER)
            {
                CG_SAVE_STATE_MACHINE_STATE;
                cb_state.status = CG_PAUSED_SCHEDULER_ID;
                cb_state.nbSched = nbSchedule;
                cb_state.scheduleStateID = id;
                cb_state.running = kResumedExecution;
            }
            CHECKERROR;
        }
       CG_AFTER_ITERATION;
       nbSchedule++;
    }

errorHandling:
    CG_AFTER_SCHEDULE;
    *error=cgStaticError;
    return(nbSchedule);
}
