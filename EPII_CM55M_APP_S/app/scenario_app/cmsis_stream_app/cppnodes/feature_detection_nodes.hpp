#pragma once


#include "canny_edge.h"

template<typename IN, int inputSize,
         typename OUT,int outputSize>
class CannyEdge;

template<int inputSize,int outputSize>
class CannyEdge<int8_t,inputSize,
                int8_t,outputSize>: 
      public GenericNode<int8_t,inputSize,
                         int8_t,outputSize>
{
public:
    /* Constructor needs the input and output FIFOs */
    CannyEdge(FIFOBase<int8_t> &src,
                FIFOBase<int8_t> &dst,
                int w,int h,
                uint32_t *params):
    GenericNode<int8_t,inputSize,int8_t,outputSize>(src,dst),
    mW(w),mH(h),mParams(params){
            mInitErrorOccured = false;
            img_tmp_grad1.numRows=3;
            img_tmp_grad1.numCols=w;
            img_tmp_grad1.pData = (divergence_q15_t*) CG_MALLOC(2*3*w*sizeof(q15_t));
            if (img_tmp_grad1.pData == nullptr)
            {
                mInitErrorOccured = true;
            }

            img_tmp_grad2.numRows=3;
            img_tmp_grad2.numCols=w;
            img_tmp_grad2.pData = (divergence_q15_t*)CG_MALLOC(2*3*w*sizeof(q15_t));
            if (img_tmp_grad2.pData == nullptr)
            {
                mInitErrorOccured = true;
            }

            img_tmp.numRows=3;
            img_tmp.numCols=w;
            img_tmp.pData = (q15_t*)CG_MALLOC(3*w*sizeof(q15_t));

            if (img_tmp.pData == nullptr)
            {
                mInitErrorOccured = true;
            }
    };

    ~CannyEdge()
    {
       CG_FREE(img_tmp_grad1.pData);
       CG_FREE(img_tmp_grad2.pData);
       CG_FREE(img_tmp.pData);
    }

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

        if (mInitErrorOccured)
        {
            return(CG_MEMORY_ALLOCATION_FAILURE);
        }

        arm_image_gray_q15_t input;
        arm_image_gray_q15_t output;

        input.numRows=mH;
        input.numCols=mW;
        input.pData = (q15_t*)i;

        output.numRows=mH;
        output.numCols=mW;
        output.pData = (q15_t*)o;

       
        arm_canny_edge_sobel_in_q15_out_u8_proc_q15(&input, 
                                                 &output, 
                                                 &img_tmp_grad1, 
                                                 &img_tmp, 
                                                 &img_tmp_grad2,
                                                 mParams[0], 
                                                 mParams[1]);
       
        
        return(0);
    };
protected:
    int mW,mH;
    arm_buffer_2_q15_t img_tmp_grad1;
    arm_buffer_2_q15_t img_tmp_grad2;
    arm_image_gray_q15_t img_tmp;
    bool mInitErrorOccured;
    uint32_t *mParams;
};

template<typename IN, int inputSize,
         typename OUT,int outputSize>
class MVCannyEdge;

template<int inputSize,int outputSize>
class MVCannyEdge<int8_t,inputSize,
                int8_t,outputSize>: 
      public GenericNode<int8_t,inputSize,
                         int8_t,outputSize>
{
public:
    /* MVCannyEdge needs the input and output FIFOs */
    MVCannyEdge(FIFOBase<int8_t> &src,
                FIFOBase<int8_t> &dst,
                int w,int h,
                uint32_t *params):
    GenericNode<int8_t,inputSize,int8_t,outputSize>(src,dst),
    mW(w),mH(h),mParams(params){

            mInitErrorOccured = false;
            gm = (gvec_t*) CG_MALLOC(h*w*sizeof(gvec_t));
            if (gm == nullptr)
            {
                mInitErrorOccured = true;
            }

            buffer = (int*)CG_MALLOC(2*w*sizeof(int));
            if (buffer == nullptr)
            {
                mInitErrorOccured = true;
            }

           
    };

    ~MVCannyEdge()
    {
        CG_FREE(gm);
        CG_FREE(buffer);
    }

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

        if (mInitErrorOccured)
        {
            return(CG_MEMORY_ALLOCATION_FAILURE);
        }

        arm_matrix_instance_q15 io;

        io.numRows=mH;
        io.numCols=mW;
        io.pData = (q15_t*)o;

        rectangle_t roi;
        roi.x= 0;
        roi.y = 0;
        roi.w = mW;
        roi.h = mH;

      
        memcpy(o,i,sizeof(q15_t)*mW*mH);

        //printf("%d : %d\n",mParams[0],mParams[1]);
        imlib_edge_canny(&io,&roi, 
                             mParams[0]/*low_thresh*/, 
                             mParams[1]/*high_thresh*/, 
                             gm, 
                             buffer);
       
        
        return(0);
    };
protected:
    int mW,mH;
    gvec_t *gm;
    int *buffer;
    bool mInitErrorOccured;
    uint32_t *mParams;
};