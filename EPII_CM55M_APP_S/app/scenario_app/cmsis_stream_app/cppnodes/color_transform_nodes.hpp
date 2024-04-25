#pragma once

#ifndef WEB 
// https://github.com/descampsa/yuv2rgb
#include "yuv_rgb.h"

template<typename IN, int inputSize,
         typename OUT,int outputSize>
class YUVToRGB;


template<int inputSize,int outputSize>
class YUVToRGB<int8_t,inputSize,
               int8_t,outputSize>: 
      public GenericNode<int8_t,inputSize,
                         int8_t,outputSize>
{
public:
    /* Constructor needs the input and output FIFOs */
    YUVToRGB(FIFOBase<int8_t> &src,
             FIFOBase<int8_t> &dst,int w,int h):
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
        int8_t *yuv=this->getReadBuffer();
        int8_t *rgb=this->getWriteBuffer();


        const uint8_t *Y = (const uint8_t *)yuv;
        const uint8_t *U = (const uint8_t *)yuv+mW*mH;
        const uint8_t *V = (const uint8_t *)yuv+mW*mH+((mW+1)/2)*((mH+1)/2);

        const size_t y_stride = mW;
        const size_t uv_stride = mW / 2;
        const size_t rgb_stride = mW*3;

        yuv420_rgb24_std(mW,mH,
                         Y,U,V,y_stride, uv_stride, 
                         (uint8_t*)rgb, rgb_stride, YCBCR_709);
        
        return(0);
    };
protected:
    int mW,mH;
};

#endif

template<typename IN, int inputSize,
         typename OUT,int outputSize>
class YUVToGray16;


template<int inputSize,int outputSize>
class YUVToGray16<int8_t,inputSize,
               int8_t,outputSize>: 
      public GenericNode<int8_t,inputSize,
                         int8_t,outputSize>
{
public:
    /* Constructor needs the input and output FIFOs */
    YUVToGray16(FIFOBase<int8_t> &src,
             FIFOBase<int8_t> &dst,int w,int h):
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
        int8_t *yuv=this->getReadBuffer();
        int8_t *d=this->getWriteBuffer();

        q15_t *gray = (q15_t*)d;

        for(int32_t k=0;k<mW*mH;k++)
        {
           *gray++ = ((q15_t)*yuv++) << 7;
        }
        
        return(0);
    };
protected:
    int mW,mH;
};

template<typename IN, int inputSize,
         typename OUT,int outputSize>
class YUVToGray8;


template<int inputSize,int outputSize>
class YUVToGray8<int8_t,inputSize,
               int8_t,outputSize>: 
      public GenericNode<int8_t,inputSize,
                         int8_t,outputSize>
{
public:
    /* Constructor needs the input and output FIFOs */
    YUVToGray8(FIFOBase<int8_t> &src,
             FIFOBase<int8_t> &dst,int w,int h):
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
        int8_t *yuv=this->getReadBuffer();
        int8_t *gray=this->getWriteBuffer();

        memcpy(gray,yuv,mW*mH);
       
        
        return(0);
    };
protected:
    int mW,mH;
};

template<typename IN, int inputSize,
         typename OUT,int outputSize>
class Gray16ToRGB;


template<int inputSize,int outputSize>
class Gray16ToRGB<int8_t,inputSize,
               int8_t,outputSize>: 
      public GenericNode<int8_t,inputSize,
                         int8_t,outputSize>
{
public:
    /* Constructor needs the input and output FIFOs */
    Gray16ToRGB(FIFOBase<int8_t> &src,
             FIFOBase<int8_t> &dst,int w,int h):
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
        int8_t *g=this->getReadBuffer();
        int8_t *rgb=this->getWriteBuffer();

        q15_t *gray = (q15_t*)g;

        for(int32_t k=0;k<mW*mH;k++)
        {
           int8_t v = *gray++ >> 7;
           *rgb++ = v;
           *rgb++ = v;
           *rgb++ = v;
        }
        
        return(0);
    };
protected:
    int mW,mH;
};

template<typename IN, int inputSize,
         typename OUT,int outputSize>
class Gray8ToRGB;


template<int inputSize,int outputSize>
class Gray8ToRGB<int8_t,inputSize,
               int8_t,outputSize>: 
      public GenericNode<int8_t,inputSize,
                         int8_t,outputSize>
{
public:
    /* Constructor needs the input and output FIFOs */
    Gray8ToRGB(FIFOBase<int8_t> &src,
             FIFOBase<int8_t> &dst,int w,int h):
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
        uint8_t *g=(uint8_t*)this->getReadBuffer();
        uint8_t *rgb=(uint8_t*)this->getWriteBuffer();


        for(int32_t k=0;k<mW*mH;k++)
        {
           uint8_t v = *g++;
           *rgb++ = v;
           *rgb++ = v;
           *rgb++ = v;
        }
        
        return(0);
    };
protected:
    int mW,mH;
};


template<typename IN, int inputSize,
         typename OUT,int outputSize>
class Gray8ToRGBA;


template<int inputSize,int outputSize>
class Gray8ToRGBA<int8_t,inputSize,
               int8_t,outputSize>: 
      public GenericNode<int8_t,inputSize,
                         int8_t,outputSize>
{
public:
    /* Constructor needs the input and output FIFOs */
    Gray8ToRGBA(FIFOBase<int8_t> &src,
             FIFOBase<int8_t> &dst,int w,int h):
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
        uint8_t *g=(uint8_t*)this->getReadBuffer();
        uint8_t *rgb=(uint8_t*)this->getWriteBuffer();


        for(int32_t k=0;k<mW*mH;k++)
        {
           uint8_t v = *g++;
           *rgb++ = v;
           *rgb++ = v;
           *rgb++ = v;
           *rgb++ = 255;
        }
        
        return(0);
    };
protected:
    int mW,mH;
};

template<typename IN, int inputSize,
         typename OUT,int outputSize>
class RGBAToGray8;


template<int inputSize,int outputSize>
class RGBAToGray8<int8_t,inputSize,
               int8_t,outputSize>: 
      public GenericNode<int8_t,inputSize,
                         int8_t,outputSize>
{
public:
    /* Constructor needs the input and output FIFOs */
    RGBAToGray8(FIFOBase<int8_t> &src,
             FIFOBase<int8_t> &dst,int w,int h):
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
        uint8_t *src=(uint8_t*)this->getReadBuffer();
        uint8_t *dst=(uint8_t*)this->getWriteBuffer();


        for(int32_t k=0;k<mW*mH;k++)
        {
           uint8_t r = *src++;
           uint8_t b = *src++;
           uint8_t g = *src++;
           src++;

           float c = 0.299f * r + 0.587f * g + 0.114f * b;
          
           *dst++ = (uint8_t)c;
        }
        
        return(0);
    };
protected:
    int mW,mH;
};

template<typename IN, int inputSize,
         typename OUT,int outputSize>
class Gray16ToRGBA;


template<int inputSize,int outputSize>
class Gray16ToRGBA<int8_t,inputSize,
               int8_t,outputSize>: 
      public GenericNode<int8_t,inputSize,
                         int8_t,outputSize>
{
public:
    /* Constructor needs the input and output FIFOs */
    Gray16ToRGBA(FIFOBase<int8_t> &src,
             FIFOBase<int8_t> &dst,int w,int h):
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
        uint16_t *g=(uint16_t*)this->getReadBuffer();
        uint8_t *rgb=(uint8_t*)this->getWriteBuffer();


        for(int32_t k=0;k<mW*mH;k++)
        {
           uint8_t v = (*g++) >> 7;
           *rgb++ = v;
           *rgb++ = v;
           *rgb++ = v;
           *rgb++ = 255;
        }
        
        return(0);
    };
protected:
    int mW,mH;
};

template<typename IN, int inputSize,
         typename OUT,int outputSize>
class Gray8ToGray16;


template<int inputSize,int outputSize>
class Gray8ToGray16<int8_t,inputSize,
               int8_t,outputSize>: 
      public GenericNode<int8_t,inputSize,
                         int8_t,outputSize>
{
public:
    /* Constructor needs the input and output FIFOs */
    Gray8ToGray16(FIFOBase<int8_t> &src,
             FIFOBase<int8_t> &dst,int w,int h):
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
        uint8_t *src=(uint8_t*)this->getReadBuffer();
        uint16_t *dst=(uint16_t*)this->getWriteBuffer();


        for(int32_t k=0;k<mW*mH;k++)
        {
           *dst++ = (uint16_t)(*src++) << 7;
        }
        
        return(0);
    };
protected:
    int mW,mH;
};

template<typename IN, int inputSize,
         typename OUT,int outputSize>
class Gray16ToGray8;


template<int inputSize,int outputSize>
class Gray16ToGray8<int8_t,inputSize,
               int8_t,outputSize>: 
      public GenericNode<int8_t,inputSize,
                         int8_t,outputSize>
{
public:
    /* Constructor needs the input and output FIFOs */
    Gray16ToGray8(FIFOBase<int8_t> &src,
             FIFOBase<int8_t> &dst,int w,int h):
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
        uint16_t *src=(uint16_t*)this->getReadBuffer();
        uint8_t *dst=(uint8_t*)this->getWriteBuffer();


        for(int32_t k=0;k<mW*mH;k++)
        {
           *dst++ = *src++ >> 7;
        }
        
        return(0);
    };
protected:
    int mW,mH;
};