#pragma once

#include "gaussian.h"


template<typename IN, int inputSize,
         typename OUT,int outputSize>
class GaussianFilter;

template<int inputSize,int outputSize>
class GaussianFilter<int8_t,inputSize,
                int8_t,outputSize>: 
      public GenericNode<int8_t,inputSize,
                         int8_t,outputSize>
{
public:
    /* Constructor needs the input and output FIFOs */
    GaussianFilter(FIFOBase<int8_t> &src,
                FIFOBase<int8_t> &dst,
                int w,int h):
    GenericNode<int8_t,inputSize,int8_t,outputSize>(src,dst),
    mW(w),mH(h){};

    /* In asynchronous mode, node execution will be 
       skipped in case of underflow on the input 
       or overflow in the output.
    */
    int prepareForRunning() final
    {
        if (this->willOverflow() ||
            this->willUnderflow())
        {
           return(CG_SKIP_EXECUTION_ID_CODE); // Skip execution
        }

        return(0);
    };
    
    /* 
       Node processing
       1 is added to the input
    */
    int run() final{
        int8_t *i=this->getReadBuffer();
        int8_t *o=this->getWriteBuffer();

        arm_image_gray_u8_t src;
        arm_image_gray_q15_t dst;

        src.numRows=mH;
        src.numCols=mW;
        src.pData = (uint8_t*)i;

        dst.numRows=mH;
        dst.numCols=mW;
        dst.pData = (q15_t*)o;
        
        arm_gaussian_filter_3x3_gray_in_u8_out_q15_proc_q15(&src,&dst);

        
        return(0);
    };
protected:
    int mW,mH;
   
};