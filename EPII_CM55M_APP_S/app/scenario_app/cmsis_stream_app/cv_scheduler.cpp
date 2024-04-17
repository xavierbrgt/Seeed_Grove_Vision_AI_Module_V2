/*

Generated with CMSIS-Stream python scripts.
The generated code is not covered by CMSIS-Stream license.

The support classes and code are covered by CMSIS-Stream license.

*/


#include <cstdint>
#include "cv_app_settings.h"
#include "GenericNodes.h"
#include "cg_status.h"
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



CG_AFTER_INCLUDES


using namespace arm_cmsis_stream;

/*

Description of the scheduling. 

*/
static uint8_t schedule[3]=
{ 
1,0,2,
};


CG_BEFORE_FIFO_BUFFERS
/***********

FIFO buffers

************/
#define FIFOSIZE0 1
#define FIFOSIZE1 307200

typedef struct {
uint32_t  *buf0;
} buffers_t;

CG_BEFORE_BUFFER
static buffers_t buffers={0};

int init_buffer_cv_scheduler(stream_env_t *env,
                              struct_algoResult *alg_result,
                              struct_fm_algoResult_with_fps *alg_fm_result)
{
    buffers.buf0 = (uint32_t *)CG_MALLOC(1 * sizeof(uint32_t));
    if (buffers.buf0==NULL)
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
}


typedef struct {
FIFO<uint32_t,FIFOSIZE0,1,0> *fifo0;
FIFO<int8_t,FIFOSIZE1,1,0> *fifo1;
} fifos_t;

typedef struct {
    JPEGImage<uint32_t,1,int8_t,307200> *jpeg;
    Camera<uint32_t,1> *ov5647;
    SendResult<int8_t,307200> *sink;
} nodes_t;

CG_BEFORE_BUFFER
static fifos_t fifos={0};

CG_BEFORE_BUFFER
static nodes_t nodes={0};


int init_cv_scheduler(stream_env_t *env,
                              struct_algoResult *alg_result,
                              struct_fm_algoResult_with_fps *alg_fm_result)
{
    CG_BEFORE_FIFO_INIT;
    fifos.fifo0 = new FIFO<uint32_t,FIFOSIZE0,1,0>(buffers.buf0);
    if (fifos.fifo0==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }
    fifos.fifo1 = new FIFO<int8_t,FIFOSIZE1,1,0>((void*)app_get_jpeg_addr());
    if (fifos.fifo1==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }

    CG_BEFORE_NODE_INIT;
    nodes.jpeg = new JPEGImage<uint32_t,1,int8_t,307200>(*(fifos.fifo0),*(fifos.fifo1));
    if (nodes.jpeg==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }
    nodes.ov5647 = new Camera<uint32_t,1>(*(fifos.fifo0));
    if (nodes.ov5647==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }
    nodes.sink = new SendResult<int8_t,307200>(*(fifos.fifo1),env,alg_result,alg_fm_result);
    if (nodes.sink==NULL)
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

    if (nodes.jpeg!=NULL)
    {
        delete nodes.jpeg;
    }
    if (nodes.ov5647!=NULL)
    {
        delete nodes.ov5647;
    }
    if (nodes.sink!=NULL)
    {
        delete nodes.sink;
    }
}


CG_BEFORE_SCHEDULER_FUNCTION
uint32_t cv_scheduler(int *error,stream_env_t *env,
                              struct_algoResult *alg_result,
                              struct_fm_algoResult_with_fps *alg_fm_result)
{
    int cgStaticError=0;
    uint32_t nbSchedule=0;
    int32_t debugCounter=1;


    /* Run several schedule iterations */
    CG_BEFORE_SCHEDULE;
    while((cgStaticError==0) && (debugCounter > 0))
    {
        /* Run a schedule iteration */
        CG_BEFORE_ITERATION;
        for(unsigned long id=0 ; id < 3; id++)
        {
            CG_BEFORE_NODE_EXECUTION(schedule[id]);

            switch(schedule[id])
            {
                case 0:
                {
                   cgStaticError = nodes.jpeg->run();
                }
                break;

                case 1:
                {
                   cgStaticError = nodes.ov5647->run();
                }
                break;

                case 2:
                {
                   cgStaticError = nodes.sink->run();
                }
                break;

                default:
                break;
            }
            CG_AFTER_NODE_EXECUTION(schedule[id]);
            CHECKERROR;
        }
       debugCounter--;
       CG_AFTER_ITERATION;
       nbSchedule++;
    }

errorHandling:
    CG_AFTER_SCHEDULE;
    *error=cgStaticError;
    return(nbSchedule);
}
