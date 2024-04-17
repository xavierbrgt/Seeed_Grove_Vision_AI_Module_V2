/* ----------------------------------------------------------------------
 * Project:      CMSIS DSP Library
 * Title:        AppNodes.h
 * Description:  Application nodes for Example simple
 *
 * Target Processor: Cortex-M and Cortex-A cores
 * -------------------------------------------------------------------- 
*
 * Copyright (C) 2021-2023 ARM Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef _APPNODES_H_
#define _APPNODES_H_

#include <string>
#include <forward_list>
#include "cisdp_sensor.h"
#include "send_result.h"
#include "spi_protocol.h"
#include "WE2_core.h"


using namespace arm_cmsis_stream;

/*

C++ template for the Sink node.
A generic implementation is provided
for all types IN.

The template has two arguments: 
- The input datatype IN
- The number of consumed sample on this input : inputSize

*/
template<typename IN, int inputSize>
class SendResult: public GenericSink<IN, inputSize>
{
public:
    // The constructor for the sink is only using
    // the input FIFO (coming from the generated scheduler).
    // This FIFO is passed to the GenericSink contructor.
    // Implementation of this Sink constructor is doing nothing
    SendResult(FIFOBase<IN> &src,
        stream_env_t *env,
        struct_algoResult *alg_result,
        struct_fm_algoResult_with_fps *alg_fm_result,
        int sendJPEG,
        int width, 
        int height):
       GenericSink<IN,inputSize>(src),
       m_env(env),
       m_alg_result(alg_result),
       m_alg_fm_result(alg_fm_result),
       mSendJpeg(sendJPEG),
       mWidth(width),
       mHeight(height){};

    // Used in asynchronous mode. In case of underflow on
    // the input, the execution of this node will be skipped
    int prepareForRunning() final
    {
        if (this->willUnderflow())
        {
           return(CG_SKIP_EXECUTION_ID_CODE); // Skip execution
        }

        return(0);
    };

   
    int run() final
    {
        IN *b=this->getReadBuffer();

        SystemGetTick(&m_env->systick_2, &m_env->loop_cnt_2);
        uint32_t algo_tick = (m_env->loop_cnt_2-m_env->loop_cnt_1)*CPU_CLK+(m_env->systick_1-m_env->systick_2);              
        algo_tick += m_env->capture_image_tick;   

        el_img_t temp_el_jpg_img = el_img_t{};
        if (mSendJpeg)
        {
           temp_el_jpg_img.data = (uint8_t*)app_get_jpeg_addr();
           temp_el_jpg_img.size = app_get_jpeg_sz();
           temp_el_jpg_img.width = app_get_raw_width();
           temp_el_jpg_img.height = app_get_raw_height();
        }
        else
        {
           temp_el_jpg_img.data = (uint8_t*)b;
           temp_el_jpg_img.size = inputSize;
        }
        
        temp_el_jpg_img.format = EL_PIXEL_FORMAT_JPEG;
        temp_el_jpg_img.rotate = EL_PIXEL_ROTATE_0;

        std::forward_list<el_fm_point_t> el_fm_point_algo;
    
        el_fm_point_t temp_el_fm_point_algo;
        temp_el_fm_point_algo.el_box.x = m_alg_fm_result->face_bbox[0].x;
        temp_el_fm_point_algo.el_box.y = m_alg_fm_result->face_bbox[0].y;
        temp_el_fm_point_algo.el_box.w = m_alg_fm_result->face_bbox[0].width;
        temp_el_fm_point_algo.el_box.h = m_alg_fm_result->face_bbox[0].height;
        temp_el_fm_point_algo.el_box.score = m_alg_fm_result->face_bbox[0].face_score;
        temp_el_fm_point_algo.el_box.target = 0;
        for(int c=0;c<FACE_MESH_POINT_NUM;c++)
        {
            
            temp_el_fm_point_algo.el_fm_point[c].x = m_alg_fm_result->fmr[c].x;
            temp_el_fm_point_algo.el_fm_point[c].y = m_alg_fm_result->fmr[c].y;
            temp_el_fm_point_algo.el_fm_point[c].score = temp_el_fm_point_algo.el_box.score;
            temp_el_fm_point_algo.el_fm_point[c].target = 0;
        }
    
        for(int c=0;c<FM_IRIS_POINT_NUM;c++)
        {
            temp_el_fm_point_algo.el_fm_iris[c].x = m_alg_fm_result->fmr_iris[c].x;
            temp_el_fm_point_algo.el_fm_iris[c].y = m_alg_fm_result->fmr_iris[c].y;
            temp_el_fm_point_algo.el_fm_iris[c].score = temp_el_fm_point_algo.el_box.score;
            temp_el_fm_point_algo.el_fm_iris[c].target = 0;
        }
    
    
        temp_el_fm_point_algo.el_fm_angle.yaw = m_alg_fm_result->face_angle.yaw * 100.0;
        temp_el_fm_point_algo.el_fm_angle.pitch = m_alg_fm_result->face_angle.pitch* 100.0;
        temp_el_fm_point_algo.el_fm_angle.roll = m_alg_fm_result->face_angle.roll* 100.0;
        temp_el_fm_point_algo.el_fm_angle.MAR = m_alg_fm_result->face_angle.MAR* 100.0;
        temp_el_fm_point_algo.el_fm_angle.LEAR = m_alg_fm_result->face_angle.LEAR* 100.0;
        temp_el_fm_point_algo.el_fm_angle.REAR = m_alg_fm_result->face_angle.REAR* 100.0;
    
        temp_el_fm_point_algo.el_fm_angle.left_iris_theta = m_alg_fm_result->left_iris_theta* 100.0;
        temp_el_fm_point_algo.el_fm_angle.left_iris_phi = m_alg_fm_result->left_iris_phi* 100.0;
        temp_el_fm_point_algo.el_fm_angle.right_iris_theta = m_alg_fm_result->right_iris_theta* 100.0;
        temp_el_fm_point_algo.el_fm_angle.right_iris_phi = m_alg_fm_result->right_iris_phi* 100.0;
    
        el_fm_point_algo.emplace_front(temp_el_fm_point_algo);
    
        std::forward_list<el_box_t> el_fm_face_bbox_algo;
    
        el_box_t temp_el_fm_face_bbox_algo;
        for(int i = 0; i < MAX_TRACKED_ALGO_RES;i++)
        {
            temp_el_fm_face_bbox_algo.x = m_alg_fm_result->face_bbox[i].x;
            temp_el_fm_face_bbox_algo.y = m_alg_fm_result->face_bbox[i].y;
            temp_el_fm_face_bbox_algo.w = m_alg_fm_result->face_bbox[i].width;
            temp_el_fm_face_bbox_algo.h = m_alg_fm_result->face_bbox[i].height;
            temp_el_fm_face_bbox_algo.score = m_alg_fm_result->face_bbox[i].face_score;
            temp_el_fm_face_bbox_algo.target = i;
    
            el_fm_face_bbox_algo.emplace_front(temp_el_fm_face_bbox_algo);
        }

        send_device_id();
        if (mSendJpeg)
        {
           event_reply(concat_strings(", ", fm_face_bbox_results_2_json_str(el_fm_face_bbox_algo),", ", algo_tick_2_json_str(algo_tick),", ", fm_point_results_2_json_str(el_fm_point_algo), ", ", img_2_json_str(&temp_el_jpg_img)));
        }
        else
        {
           event_reply(concat_strings(", ", fm_face_bbox_results_2_json_str(el_fm_face_bbox_algo),", ", algo_tick_2_json_str(algo_tick),", ", fm_point_results_2_json_str(el_fm_point_algo), ", ", rgb_img_2_json_str(&temp_el_jpg_img)));
        }
        return(0);
    };

protected:
    stream_env_t *m_env;
    struct_algoResult *m_alg_result;
    struct_fm_algoResult_with_fps *m_alg_fm_result;
    int mSendJpeg;
    int mWidth;
    int mHeight;
};


template<typename OUT,int outputSize>
class Camera:public GenericSource<OUT,outputSize>
{
public:
    Camera(FIFOBase<OUT> &dst):GenericSource<OUT,outputSize>(dst){};

    int prepareForRunning() final
    {
        if (this->willOverflow())
        {
           return(CG_SKIP_EXECUTION_ID_CODE); // Skip execution
        }

        return(0);
    };

    int run() final{
        invalidate_cache_for_camera();

        OUT *res=this->getWriteBuffer();
        (void)res;
        
        return(0);
    };

};




#endif