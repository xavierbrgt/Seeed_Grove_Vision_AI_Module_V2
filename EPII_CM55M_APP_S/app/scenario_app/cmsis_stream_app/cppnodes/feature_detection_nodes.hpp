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
                int w,int h):
    GenericNode<int8_t,inputSize,int8_t,outputSize>(src,dst),
    mW(w),mH(h){
            img_tmp_grad1.numRows=3;
            img_tmp_grad1.numCols=w;
            img_tmp_grad1.pData = (divergence_q15_t*) mm_reserve_align(2*3*w*sizeof(q15_t),0x20);

            img_tmp_grad2.numRows=3;
            img_tmp_grad2.numCols=w;
            img_tmp_grad2.pData = (divergence_q15_t*)mm_reserve_align(2*3*w*sizeof(q15_t),0x20);

            img_tmp.numRows=3;
            img_tmp.numCols=w;
            img_tmp.pData = (q15_t*)mm_reserve_align(3*w*sizeof(q15_t),0x20);
    };

    ~CannyEdge()
    {
       //free(img_tmp_grad1.pData);
       //free(img_tmp_grad2.pData);
       //free(img_tmp.pData);
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
                                                 &img_tmp_grad2);
       
        
        return(0);
    };
protected:
    int mW,mH;
    arm_buffer_2_q15_t img_tmp_grad1;
    arm_buffer_2_q15_t img_tmp_grad2;
    arm_image_gray_q15_t img_tmp;
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
                int w,int h):
    GenericNode<int8_t,inputSize,int8_t,outputSize>(src,dst),
    mW(w),mH(h){
            
            gm = (gvec_t*) mm_reserve_align(w*sizeof(gvec_t),0x20);
            buffer = (int*)mm_reserve_align(2*w*sizeof(int),0x20);

           
    };

    ~MVCannyEdge()
    {

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

        arm_matrix_instance_q15 io;

        io.numRows=mH;
        io.numCols=mW;
        io.pData = (q15_t*)o;

        rectangle_t roi={0,0,(int16_t)mW,(int16_t)mH};

      
        memcpy(o,i,sizeof(q15_t)*mW*mH);

        int low_thresh = (int)(0.3*255);
        int high_thresh = (int)(0.7*255);

       
        imlib_edge_canny(&io,&roi, 
                             low_thresh, 
                             high_thresh, 
                             gm, 
                             buffer);
       
        
        return(0);
    };
protected:
    int mW,mH;
    gvec_t *gm;
    int *buffer;
};