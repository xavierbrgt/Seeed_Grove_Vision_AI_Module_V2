#pragma once

extern "C" {
    extern uint8_t     g_frame_ready;
};

template<typename OUT,int outputSize>
class Camera;

template<int outputSize>
class Camera<int8_t,outputSize>
:public GenericSource<int8_t,outputSize>
{
public:
    Camera(FIFOBase<int8_t> &dst):GenericSource<int8_t,outputSize>(dst){};

    int prepareForRunning() final
    {
        if (this->willOverflow())
        {
           return(CG_SKIP_EXECUTION_ID_CODE); // Skip execution
        }

        return(0);
    };

    int run() final{
        if (!g_frame_ready)
        {
            return(CG_PAUSED_SCHEDULER);
        }
        g_frame_ready=0;

        invalidate_raw_cache_for_camera(CAMERA_FORMAT);

        int8_t *res=this->getWriteBuffer();
        (void)res;
        
        return(0);
    };

};

template<typename OUT1, int outputSize1,
         typename OUT2, int outputSize2,
         typename OUT3, int outputSize3>
class GenericSource3:public NodeBase
{
public:
     explicit GenericSource3(FIFOBase<OUT1> &dst1,FIFOBase<OUT2> &dst2,FIFOBase<OUT3> &dst3):
     mDst1(dst1),mDst2(dst2),mDst3(dst3){};

protected:
     OUT1 * getWriteBuffer1(int nb=outputSize1) {return mDst1.getWriteBuffer(nb);};
     bool willOverflow1(int nb = outputSize1) const {return mDst1.willOverflowWith(nb);};

     OUT2 * getWriteBuffer2(int nb=outputSize2) {return mDst2.getWriteBuffer(nb);};
     bool willOverflow2(int nb = outputSize2) const {return mDst2.willOverflowWith(nb);};

     OUT3 * getWriteBuffer3(int nb=outputSize3) {return mDst3.getWriteBuffer(nb);};
     bool willOverflow3(int nb = outputSize3) const {return mDst3.willOverflowWith(nb);};

private:
    FIFOBase<OUT1> &mDst1;
    FIFOBase<OUT2> &mDst2;
    FIFOBase<OUT3> &mDst3;
};

template<typename OUT1,int outputSize1,
         typename OUT2,int outputSize2,
         typename OUT3,int outputSize3>
class CameraJPEG;

template<int outputSize1,int outputSize2>
class CameraJPEG<int8_t,outputSize1,
                 int8_t,outputSize2,
                 uint32_t,1>:
                 public GenericSource3<int8_t,outputSize1,
                       int8_t,outputSize2,
                       uint32_t,1>
{
public:
    CameraJPEG(FIFOBase<int8_t> &dst1,FIFOBase<int8_t> &dst2,FIFOBase<uint32_t> &dst3):
    GenericSource3<int8_t,outputSize1,
                   int8_t,outputSize2,
                   uint32_t,1>(dst1,dst2,dst3){};

    int prepareForRunning() final
    {
        if (this->willOverflow1() ||
            this->willOverflow2() ||
            this->willOverflow3())
        {
           return(CG_SKIP_EXECUTION_ID_CODE); // Skip execution
        }

        return(0);
    };

    int run() final{
        if (!g_frame_ready)
        {
            return(CG_PAUSED_SCHEDULER);
        }
        g_frame_ready=0;
        
        invalidate_raw_cache_for_camera(CAMERA_FORMAT);
        invalidate_jpeg_cache_for_camera(CAMERA_FORMAT);

        int8_t *raw=this->getWriteBuffer1();
        (void)raw;

        int8_t *jpeg=this->getWriteBuffer2();
        (void)jpeg;

        uint32_t *l = this->getWriteBuffer3();
        l[0] = app_get_jpeg_sz();
        
        return(0);
    };

};