/*
 * cvapp_fd_fm.cpp
 *
 *  Created on: Feb 15, 2023
 *      Author: bigcat-himax
 */

#include <cstdio>
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "WE2_device.h"
#include "board.h"
#include "cvapp_fd_fm.h"
#include "cisdp_sensor.h"

#include "cv_app_settings.h"

#if defined(ORIGINAL_APP)

#include "WE2_core.h"

#include "ethosu_driver.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/schema/schema_generated.h"
#include "tensorflow/lite/c/common.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"

#include "img_proc_helium.h"
#include "yolo_postprocessing.h"
#include "pose_processing.h"


#include "xprintf.h"
#include "spi_master_protocol.h"
#include "cisdp_cfg.h"
#include "memory_manage.h"
#include "common_config.h"
#include "send_result.h"


#define APP_IRIS_LANDMARK 	1
#define YUV_640_480_INPUT   1 /*RGB_320_240_INPUT*/


#ifndef YUV_640_480_INPUT /*RGB_320_240_INPUT*/

#define COLOR_CHANNEL					3U
#define MAX_RESIZE_IMAGE_SIDE_LENGTTH 	160
#define FD_INPUT_TENSOR_WIDTH   		MAX_RESIZE_IMAGE_SIDE_LENGTTH
#define FD_INPUT_TENSOR_HEIGHT  		MAX_RESIZE_IMAGE_SIDE_LENGTTH

#define CROP_IMAGE_WIDTH				320
#define CROP_IMAGE_HEIGHT				240

#define FM_INPUT_TENSOR_WIDTH			192
#define FM_INPUT_TENSOR_HEIGHT  		192

#define IL_INPUT_TENSOR_WIDTH   64
#define IL_INPUT_TENSOR_HEIGHT  64

#else

#define COLOR_CHANNEL					1U
#define MAX_RESIZE_IMAGE_SIDE_LENGTTH 	160
#define FD_INPUT_TENSOR_WIDTH   		MAX_RESIZE_IMAGE_SIDE_LENGTTH
#define FD_INPUT_TENSOR_HEIGHT  		MAX_RESIZE_IMAGE_SIDE_LENGTTH

#define CROP_IMAGE_WIDTH				640
#define CROP_IMAGE_HEIGHT				480

#define FM_INPUT_TENSOR_WIDTH			192
#define FM_INPUT_TENSOR_HEIGHT  		192


#define IL_INPUT_TENSOR_WIDTH   64
#define IL_INPUT_TENSOR_HEIGHT  64


#endif

#define TOTAL_STEP_TICK 1

static uint32_t capture_image_tick = 0;
//left eyes indices #16 point
int LEFT_EYE_mesh_index[16] ={ 362, 382, 381, 380, 374, 373, 390, 249, 263, 466, 388, 387, 386, 385,384, 398 };

// int LEFT_EYE_mesh_index[16] ={  249, 263,362,  373,  374, 380,381,382, 384, 385, 386,387 ,388, 390, 398, 466};
//right eyes indices #16 point
int RIGHT_EYE_mesh_index[16] ={33, 7, 163, 144, 145, 153, 154, 155, 133, 173, 157, 158, 159, 160, 161 , 246 };
// int RIGHT_EYE_mesh_index[16] ={7, 33,  133,  144, 145, 153, 154, 155, 157, 158, 159, 160, 161 ,163,173,  246 };

//# mapping from left eye contour index to face landmark index
int RIGHT_EYE_MAP_TO_FACE_MESH_INDEX[71] = {
    //# eye lower contour
    33, 7, 163, 144, 145, 153, 154, 155, 133,
    //# eye upper contour excluding corners
    246, 161, 160, 159, 158, 157, 173,
    //# halo x2 lower contour
    130, 25, 110, 24, 23, 22, 26, 112, 243,
    //# halo x2 upper contour excluding corners
    247, 30, 29, 27, 28, 56, 190,
    //# halo x3 lower contour
    226, 31, 228, 229, 230, 231, 232, 233, 244,
    //# halo x3 upper contour excluding corners
    113, 225, 224, 223, 222, 221, 189,
    //# halo x4 upper contour (no upper due to mesh structure)
   // # or eyebrow inner contour
    35, 124, 46, 53, 52, 65,
    //# halo x5 lower contour
    143, 111, 117, 118, 119, 120, 121, 128, 245,
    //# halo x5 upper contour excluding corners or eyebrow outer contour
    156, 70, 63, 105, 66, 107, 55, 193,
};

//# mapping from right eye contour index to face landmark index
int LEFT_EYE_MAP_TO_FACE_MESH_INDEX[71] = {
    //# eye lower contour
    263, 249, 390, 373, 374, 380, 381, 382, 362,
    //# eye upper contour excluding corners
    466, 388, 387, 386, 385, 384, 398,
    //# halo x2 lower contour
    359, 255, 339, 254, 253, 252, 256, 341, 463,
    //# halo x2 upper contour excluding corners
    467, 260, 259, 257, 258, 286, 414,
   // # halo x3 lower contour
    446, 261, 448, 449, 450, 451, 452, 453, 464,
    //# halo x3 upper contour excluding corners
    342, 445, 444, 443, 442, 441, 413,
    //# halo x4 upper contour (no upper due to mesh structure)
    //# or eyebrow inner contour
    265, 353, 276, 283, 282, 295,
    //# halo x5 lower contour
    372, 340, 346, 347, 348, 349, 350, 357, 465,
   // # halo x5 upper contour excluding corners or eyebrow outer contour
    383, 300, 293, 334, 296, 336, 285, 417,
};


//# irises Indices list
int LEFT_IRIS[5] = { 473, 474,475, 476, 477};
int RIGHT_IRIS[5] = {468, 469, 470, 471 ,472};
#define COMPUTE_ANGLE
#define M_PI           3.14159265358979323846  /* pi*/

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

using namespace std;

//TODO maybe use helium optimize
static void BGRU3C_to_RGB24(uint8_t*in_image, int8_t*out_image,int32_t in_image_width, int32_t in_image_height)
{
	int32_t x,y;
	uint8_t *b_image=in_image, *g_image= in_image+in_image_width*in_image_height, *r_image = g_image+in_image_width*in_image_height;
	
	//channel adjust (in_image BBBBBB/GGGGGG/RRRRRR => out_image RGBRGB)
	for (y = 0; y < in_image_height; y++) {//compute new pixels
			for (x = 0; x < in_image_width; x++) {
				int8_t b,g,r;
				b = b_image[y * in_image_width + x];
				g = g_image[y * in_image_width + x];
				r = r_image[y * in_image_width + x];

				out_image[(in_image_width*y+x)*3] = r-128;
				out_image[(in_image_width*y+x)*3+1] = g-128;
				out_image[(in_image_width*y+x)*3+2] = b-128;
			}
	}

}

//Expand Y channel to 3 Y channel
static void Y_to_YYY(uint8_t*in_image, int8_t*out_image,int32_t in_image_width, int32_t in_image_height)
{
	int32_t x,y;
	uint8_t *Y_image=in_image;
	
	//channel adjust (in_image Y->YYY)
	for (y = 0; y < in_image_height; y++) {//compute new pixels
			for (x = 0; x < in_image_width; x++) {
				int8_t Y;
				Y = Y_image[y * in_image_width + x];
				

				out_image[(in_image_width*y+x)*3] = Y-128;
				out_image[(in_image_width*y+x)*3+1] = Y-128;
				out_image[(in_image_width*y+x)*3+2] = Y-128;
			}
	}

}
//TODO maybe use helium optimize
static void BGRU3C_to_GRAY(uint8_t*in_image, int8_t*out_image,int32_t in_image_width, int32_t in_image_height)
{
	int32_t x,y;
	uint8_t *b_image=in_image, *g_image= in_image+in_image_width*in_image_height, *r_image = g_image+in_image_width*in_image_height;
	
	//channel adjust (in_image BBBBBB/GGGGGG/RRRRRR => out_image RGBRGB)
	for (y = 0; y < in_image_height; y++) {//compute new pixels
			for (x = 0; x < in_image_width; x++) {
				int32_t b,g,r;
				b = b_image[y * in_image_width + x];
				g = g_image[y * in_image_width + x];
				r = r_image[y * in_image_width + x];

				#if 0
					out_image[(in_image_width*y+x)] = (int8_t)((b+g+r)/3-128);
				#else
					int16_t r_i = int16_t((float)r*(float)0.299); 
					int16_t g_i = int16_t((float)g*(float)0.587); 
					int16_t b_i = int16_t((float)b*(float)0.144); 
					out_image[(in_image_width*y+x)] = (int8_t)((b_i+g_i+r_i)-128);
				#endif
			}
	}

}



static void yolo_post_processing(network* net, struct_algoResult *alg_result)
{
	int nboxes=0;
	float thresh = .50;
	float nms = .45;
	uint8_t counter = 0;
	uint32_t sensor_width = app_get_raw_width();
	uint32_t sensor_height = app_get_raw_height();

	std::forward_list<detection> dets = get_network_boxes(net, sensor_width, sensor_height, thresh, &nboxes);
#ifdef FD_FL_DEBUG
	dbg_printf(DBG_LESS_INFO,"box:%d\n",nboxes);
#endif

	//clear_alg_rsult(&alg_result);
	// do nms
	diounms_sort(dets, net->num_classes, nms);

	for (std::forward_list<detection>::iterator it=dets.begin(); it != dets.end(); ++it){
		/**************
		 *
		 * To let FD bbox do not too tight to do FL
			python code to calculate boxes
			ymin = max(d['y'] - d['h']/2, 0)
			d['w'] = d['w'] * 1.2
			d['h'] = d['h'] * 1.2
			xmin = max(d['x'] - d['w']/2, 0)
			xmax = min(d['x'] + d['w']/2, image_shape[1])
			ymax = min(d['y'] + d['h']/2, image_shape[0])
			boxes.append([xmin, ymin, xmax, ymax])
		**************/
		/*** do not change final it->bbox.w and it->bbox.h value
		 * so combine this part at below code
		//		it->bbox.w = it->bbox.w * 1.2;
		//		it->bbox.h = it->bbox.h * 1.2;
		*****/
		// float ymin = it->bbox.y - it->bbox.h / 2.;
		// float xmin = it->bbox.x - ((it->bbox.w * 1.2) / 2.);
		// float xmax = it->bbox.x + ((it->bbox.w * 1.2) / 2.);
		// float ymax = it->bbox.y + ((it->bbox.h * 1.2) / 2.);

		float box_scale_factor = 1.6;//1.8;

		float ymin = it->bbox.y - ((it->bbox.h * box_scale_factor) / 2.);
		float xmin = it->bbox.x - ((it->bbox.w * box_scale_factor) / 2.);
		float xmax = it->bbox.x + ((it->bbox.w * box_scale_factor) / 2.);
		float ymax = it->bbox.y + ((it->bbox.h * box_scale_factor) / 2.);

		if (xmin < 0) xmin = 0;
		if (ymin < 0) ymin = 0;
		if (xmax > sensor_width) xmax = sensor_width;
		if (ymax > sensor_height) ymax = sensor_height;

		float bx = xmin;
		float by = ymin;
		float bw = xmax - xmin;
		float bh = ymax - ymin;
		for (int j = 0; j < net->num_classes; ++j) {
			if (it->prob[j] > 0 && counter < MAX_TRACKED_ALGO_RES) {
				#ifdef FD_DEBUG
				xprintf("{%d \"bbox\":[%d, %d, %d, %d], \"score\":%d},\n", j, (int)(bx), (int)(by), (int)(bw), (int)(bh), (int)(it->prob[j]*100));
				xprintf("xmin: %d, ymin:%d, xmax: %d, ymax: %d\n",(int)(bx), (int)(by), (int)(bx+bw), (int)(by+bh));
				#endif
				alg_result->ht[counter].upper_body_score = (uint32_t)(it->prob[j]*100);
				alg_result->ht[counter].upper_body_bbox.x = (uint32_t)(bx);      //xmin
				alg_result->ht[counter].upper_body_bbox.y = (uint32_t)(by);      //ymin
				alg_result->ht[counter].upper_body_bbox.width = (uint32_t)(bw);  //xmax
				alg_result->ht[counter].upper_body_bbox.height = (uint32_t)(bh); //ymax
				alg_result->ht[counter].upper_body_scale = j;
				counter++;
				alg_result->num_tracked_human_targets++;
			}
		}
		#ifdef FD_DEBUG
		xprintf("alg_result->num_tracked_human_targets: %d\r\n",alg_result->num_tracked_human_targets);
		#endif
	}
	free_dets(dets);
	//free((net->branchs));
}

float CaculateDistance(uint32_t x1,uint32_t y1,uint32_t x2,uint32_t y2)
{
	return (float)sqrt(pow((float)x1 - (float)x2,2)+pow((float)y1 - (float)y2,2));
}

#ifdef COMPUTE_ANGLE
void compute_ypr_face_mesh(struct_fm_algoResult_with_fps *algoresult_fm)
{
	//MAX_FACE_LAND_MARK_TRACKED_POINT

	//# Right eyebrow. 10 point
	// 70 63 105 66 107
	// 46 53 52 65 55  // use lower eyebrow

	//# Left  eyebrow. 10 point
	// 300 293 334 296 336
	// 276 283 282 295 285 // use lower eyebrow

	//yaw
	float A = CaculateDistance(algoresult_fm->fmr[46].x, algoresult_fm->fmr[46].y
								, algoresult_fm->fmr[55].x, algoresult_fm->fmr[55].y
								);
	float B = CaculateDistance(algoresult_fm->fmr[276].x, algoresult_fm->fmr[276].y
								, algoresult_fm->fmr[285].x, algoresult_fm->fmr[285].y
								);
	if (A == B)
	{
		algoresult_fm->face_angle.yaw = 0;
	}
    else if(A < B)
	{
		algoresult_fm->face_angle.yaw = -asin(1.0 - (A / B));
	} 
    else
	{
		algoresult_fm->face_angle.yaw = asin(1.0 - (B / A));
	}
    
	// Face oval. 36 point 
	// from upper center: 
	// 10 
	// upper left face to down:
	// 338 297 332 284 251 389  356  454 323 361 288 397 365 379 378 400 377 
	// lower center:  
	// 152 
	// lower right face to up:
	// 148 176 149 150 136 172 58 132 93 234 127 162 21 54 103 67 109

	//pitch 
	//nose : 5
	// right face : 234
	// left face : 454
	//pitch
	float VD;
	int v1_x = algoresult_fm->fmr[454].x - algoresult_fm->fmr[234].x;
	int v1_y = algoresult_fm->fmr[454].y -algoresult_fm->fmr[234].y;
	int v2_x = algoresult_fm->fmr[5].x -algoresult_fm->fmr[234].x;
	int v2_y = algoresult_fm->fmr[5].y -algoresult_fm->fmr[234].y;
	VD = (float)((v1_x*v2_y)-(v1_y*v2_x)) / CaculateDistance( algoresult_fm->fmr[234].x, algoresult_fm->fmr[234].y
								, algoresult_fm->fmr[454].x, algoresult_fm->fmr[454].y);
	float theta1 = asin(VD / CaculateDistance( algoresult_fm->fmr[5].x, algoresult_fm->fmr[5].y
								, algoresult_fm->fmr[234].x, algoresult_fm->fmr[234].y));
    float theta2 = asin(VD / CaculateDistance( algoresult_fm->fmr[5].x, algoresult_fm->fmr[5].y
								, algoresult_fm->fmr[454].x, algoresult_fm->fmr[454].y));
    algoresult_fm->face_angle.pitch = (theta1+theta2) / 2.0;
	
	//roll 
	// right face : 93
	// left face : 323
	//roll
	float rd_roll;
	float vd_roll;
	rd_roll = CaculateDistance( algoresult_fm->fmr[93].x,  algoresult_fm->fmr[93].y
								,  algoresult_fm->fmr[323].x,  algoresult_fm->fmr[323].y);

	vd_roll = CaculateDistance( algoresult_fm->fmr[323].x,  algoresult_fm->fmr[323].y
								,  algoresult_fm->fmr[323].x,  algoresult_fm->fmr[93].y);
	if (rd_roll == vd_roll)
	{
		algoresult_fm->face_angle.roll = 0;
	}
    else
	{
		algoresult_fm->face_angle.roll = asin(vd_roll/rd_roll);
        if( algoresult_fm->fmr[323].y >  algoresult_fm->fmr[93].y)
		{
			algoresult_fm->face_angle.roll = -algoresult_fm->face_angle.roll;
		}
	}      

	//convert from radian to degree
	algoresult_fm->face_angle.yaw = algoresult_fm->face_angle.yaw * 180.0 / M_PI;
	algoresult_fm->face_angle.pitch = algoresult_fm->face_angle.pitch * 180.0 / M_PI;
	algoresult_fm->face_angle.roll = algoresult_fm->face_angle.roll * 180.0 / M_PI;

	#ifdef COMPUTE_ANGLE_DEBUG
		printf("yaw: %f\r\n", algoresult_fm->face_angle.yaw);
		printf("pitch: %f\r\n",algoresult_fm->face_angle.pitch);
		printf("roll: %f\r\n",algoresult_fm->face_angle.roll);
	#endif//COMPUTE_ANGLE_DEBUG

	return ;
}
void compute_ANGLE_face_mesh(struct_fm_algoResult_with_fps *algoresult_fm)
{
	/*
	 MAR = (np.linalg.norm(marks[61, ] - marks[67, ]) +
               np.linalg.norm(marks[62, ] - marks[66, ]) +
               np.linalg.norm(marks[63, ] - marks[65, ])) / (2*np.linalg.norm(marks[60, ] - marks[64, ]))

	REAR = (np.linalg.norm(marks[37, ] - marks[41, ]) +
                   np.linalg.norm(marks[38, ] - marks[40, ])) / (2*np.linalg.norm(marks[39, ] - marks[36, ]))

    LEAR = (np.linalg.norm(marks[43, ] - marks[47, ]) +
                   np.linalg.norm(marks[44, ] - marks[46, ])) / (2*np.linalg.norm(marks[42, ] - marks[45, ]))
	*/

	//lip
	// right center :
	// 78
	// lower: (right->left)
	// 95 88 178 87 14 317 402  318  324  
	// left center :
	// 308
	// upper: (left->right)
	// 415  310  311  312  13  82  81  80  191 


	// face landmark -> face mesh
	// 60 -> 78
	// 64 -> 308
	// upper
	// 61 -> 82 
	// 62 -> 13
	// 63 -> 312
	// lower
	// 67 -> 87
	// 66 -> 14
	// 65 -> 317

	algoresult_fm->face_angle.MAR = (CaculateDistance( algoresult_fm->fmr[82].x,  algoresult_fm->fmr[82].y
						,  algoresult_fm->fmr[87].x,  algoresult_fm->fmr[87].y) +
						CaculateDistance( algoresult_fm->fmr[13].x,  algoresult_fm->fmr[13].y
						,  algoresult_fm->fmr[14].x,  algoresult_fm->fmr[14].y) +
						CaculateDistance( algoresult_fm->fmr[312].x,  algoresult_fm->fmr[312].y
						,  algoresult_fm->fmr[317].x,  algoresult_fm->fmr[317].y)) /
						(2.0 * CaculateDistance( algoresult_fm->fmr[78].x,  algoresult_fm->fmr[78].y
						,  algoresult_fm->fmr[308].x,  algoresult_fm->fmr[308].y));


	// right eye
	// right center:
	// 33
	// lower (right -> left)
	// 7 163 144 145 153 154 155
	// left center:
	// 133
	// upper (right -> left)
	// 246 161 160 159 158 157 173


	// face landmark -> face mesh
	// center
	// 39 -> 133
	// 36 -> 33
	// 37 -> 160
	// 41 -> 144
	// 38 -> 159
	// 40 -> 145

	
	algoresult_fm->face_angle.REAR =  (CaculateDistance( algoresult_fm->fmr[160].x,  algoresult_fm->fmr[160].y
						,  algoresult_fm->fmr[144].x,  algoresult_fm->fmr[144].y) +
						CaculateDistance( algoresult_fm->fmr[159].x,  algoresult_fm->fmr[159].y
						,  algoresult_fm->fmr[145].x,  algoresult_fm->fmr[145].y)) /
						(2.0 * CaculateDistance( algoresult_fm->fmr[133].x,  algoresult_fm->fmr[133].y
						,  algoresult_fm->fmr[33].x,  algoresult_fm->fmr[33].y));


	// left eye
	// right center:
	// 362
	// lower (right -> left)
	// 382 381 380 374 373 390 249
	// left center:
	// 263
	// upper (right -> left)
	// 398 384 385 386 387 388 466


	// face landmark -> face mesh
	// center
	// 42 -> 362
	// 45 -> 263
	// 43 -> 385
	// 47 -> 380
	// 44 -> 386
	// 46 -> 374


	algoresult_fm->face_angle.LEAR =  (CaculateDistance( algoresult_fm->fmr[385].x,  algoresult_fm->fmr[385].y
						,  algoresult_fm->fmr[380].x,  algoresult_fm->fmr[380].y) +
						CaculateDistance( algoresult_fm->fmr[386].x,  algoresult_fm->fmr[386].y
						,  algoresult_fm->fmr[374].x,  algoresult_fm->fmr[374].y)) /
						(2.0 * CaculateDistance( algoresult_fm->fmr[362].x,  algoresult_fm->fmr[362].y
						,  algoresult_fm->fmr[263].x,  algoresult_fm->fmr[263].y));
	#ifdef COMPUTE_ANGLE_DEBUG
		printf("MAR: %f\r\n",algoresult_fm->face_angle.MAR);
		printf("REAR: %f\r\n",algoresult_fm->face_angle.REAR);
		printf("LEAR: %f\r\n",algoresult_fm->face_angle.LEAR);
	#endif//COMPUTE_ANGLE_DEBUG
	return ;
}
#endif//COMPUTE_ANGLE

#ifdef APP_IRIS_LANDMARK
void crop_single_eye_IL(uint8_t* input_face_w_pad_image,uint8_t* crop_dist_image,struct_position *fm_eye_r_wo_scale ,struct_position* eye_center, int LR)
{
	eye_center->x = 0;
	eye_center->y = 0;
	if(LR == 1){
		#ifdef IL_DEBUG
		xprintf("right eye \n");
		#endif
		for(int i=0;i<16;i++)
		{
			eye_center->x += fm_eye_r_wo_scale[i].x;
			eye_center->y += fm_eye_r_wo_scale[i].y;
		}
		eye_center->x = eye_center->x / 16;
		eye_center->y = eye_center->y / 16;
	}
	else{
		#ifdef IL_DEBUG
		xprintf("left eye \n");
		#endif
		for(int i=0;i<16;i++)
		{
			eye_center->x += fm_eye_r_wo_scale[i].x;
			eye_center->y += fm_eye_r_wo_scale[i].y;
		}
		eye_center->x = eye_center->x / 16;
		eye_center->y = eye_center->y / 16;
	}


	struct__box crop_eye_bbox;
	crop_eye_bbox.x = eye_center->x - IL_INPUT_TENSOR_WIDTH/2;
	crop_eye_bbox.y = eye_center->y - IL_INPUT_TENSOR_HEIGHT/2;	
	crop_eye_bbox.width = IL_INPUT_TENSOR_WIDTH;
	crop_eye_bbox.height = IL_INPUT_TENSOR_HEIGHT;
	hx_lib_image_copy_helium((uint8_t*)input_face_w_pad_image, crop_dist_image, FM_INPUT_TENSOR_WIDTH, FM_INPUT_TENSOR_WIDTH, COLOR_CHANNEL, crop_eye_bbox.x, crop_eye_bbox.y, crop_eye_bbox.width, crop_eye_bbox.height);
}

static void IL_post_proccessing(TfLiteTensor* outputTensor_1,TfLiteTensor* outputTensor_2, struct_fm_algoResult_with_fps *algoresult_fm,struct_position* eye_center, int LR)
{
	int  eye_shift_x,eye_shift_y;
	eye_shift_x= eye_center->x - 64/2;
	eye_shift_y = eye_center->y - 64/2;
	if(eye_shift_x < 0 )eye_shift_x  = 0;
	if(eye_shift_y < 0 )eye_shift_y  = 0;

#ifdef IL_DEBUG
	xprintf("bbox->width: %d bbox->height: %d\r\n",bbox->width,bbox->height);
	xprintf("algoresult_fm->face_bbox[0].width: %d  algoresult_fm->face_bbox[0].height: %d \r\n",algoresult_fm->face_bbox[0].width,  algoresult_fm->face_bbox[0].height);
#endif

	//iris landmark
	float outputTensor_1_scale = ((TfLiteAffineQuantization*)(outputTensor_1->quantization.params))->scale->data[0];
	int outputTensor_1_zeropoint = ((TfLiteAffineQuantization*)(outputTensor_1->quantization.params))->zero_point->data[0];
	int outputTensor_1_size = outputTensor_1->bytes;

	//eye landmark
	float outputTensor_2_scale = ((TfLiteAffineQuantization*)(outputTensor_2->quantization.params))->scale->data[0];
	int outputTensor_2_zeropoint = ((TfLiteAffineQuantization*)(outputTensor_2->quantization.params))->zero_point->data[0];
	int outputTensor_2_size = outputTensor_2->bytes;

	//iris landmark
	//TO-DO
	
	for(int c=0;c<outputTensor_1->dims->data[1];c++)
	{
		int value =  outputTensor_1->data.int8[ c ];
		
		float deq_value = ((float) value-(float)outputTensor_1_zeropoint) * outputTensor_1_scale ;
		int long_Edge;
		if(algoresult_fm->face_bbox[0].width > algoresult_fm->face_bbox[0].height)
		{
			long_Edge = algoresult_fm->face_bbox[0].width;
		}
		else
		{
			long_Edge = algoresult_fm->face_bbox[0].height;
		}
		if((c%3) == 0)
		{
			
			deq_value = (((deq_value + (float)eye_shift_x) /(float) FM_INPUT_TENSOR_WIDTH) * (float)long_Edge);
		
			//update x y face_MESH_landmark point from crop start point
			int16_t temp_deq_value = (int16_t) deq_value + (int16_t)algoresult_fm->face_bbox[0].x;
			if(algoresult_fm->face_bbox[0].height > algoresult_fm->face_bbox[0].width)
			{
				temp_deq_value = temp_deq_value - ((int16_t)algoresult_fm->face_bbox[0].height - (int16_t)algoresult_fm->face_bbox[0].width)/2;
			}
			// algoresult_fm->fmr[].x =  (int16_t)temp_deq_value;
			if(LR==1)algoresult_fm->fmr_iris[c/3 + 5].x =  (int16_t)temp_deq_value;//right
			else algoresult_fm->fmr_iris[c/3].x =  (int16_t)temp_deq_value;//left

		}
		else if((c%3) == 1)
		{
			
			
			deq_value = (((deq_value  + (float)eye_shift_y) /(float) FM_INPUT_TENSOR_HEIGHT) * (float)long_Edge);
			int16_t temp_deq_value = (int16_t) deq_value + (int16_t)algoresult_fm->face_bbox[0].y;
			if(algoresult_fm->face_bbox[0].width > algoresult_fm->face_bbox[0].height)
			{
				temp_deq_value = temp_deq_value - ((int16_t)algoresult_fm->face_bbox[0].width - (int16_t)algoresult_fm->face_bbox[0].height)/2;
			}
			// algoresult_fm->fmr[].y =  (int16_t)temp_deq_value;
			
			if(LR==1)algoresult_fm->fmr_iris[c/3 + 5].y =  (int16_t)temp_deq_value;//right
			else algoresult_fm->fmr_iris[c/3].y =  (int16_t)temp_deq_value;//left
		}
		else 
		{
			continue;
			
		}
		
	}
	
	
	//eye landmark
	for(int c = 0;c < outputTensor_2->dims->data[1]; c++)
	{
		int value =  outputTensor_2->data.int8[ c ];
		
		float deq_value = ((float) value-(float)outputTensor_2_zeropoint) * outputTensor_2_scale ;
		int long_Edge;
		if(algoresult_fm->face_bbox[0].width > algoresult_fm->face_bbox[0].height)
		{
			long_Edge = algoresult_fm->face_bbox[0].width;
		}
		else
		{
			long_Edge = algoresult_fm->face_bbox[0].height;
		}
		
		if((c%3) == 0)
		{
			

			deq_value = (((deq_value + (float)eye_shift_x) /(float) FM_INPUT_TENSOR_WIDTH) * (float)long_Edge);
		
			//update x y face_MESH_landmark point from crop start point
			int16_t temp_deq_value = (int16_t) deq_value + (int16_t)algoresult_fm->face_bbox[0].x;
			if(algoresult_fm->face_bbox[0].height > algoresult_fm->face_bbox[0].width)
			{
				temp_deq_value = temp_deq_value - ((int16_t)algoresult_fm->face_bbox[0].height - (int16_t)algoresult_fm->face_bbox[0].width)/2;
			}
			//TO-DO Update IL model result to original  eye of face mesh
			//
			if(LR==1)
			{
				algoresult_fm->fmr[RIGHT_EYE_MAP_TO_FACE_MESH_INDEX[c/3]].x =  (int16_t)temp_deq_value;//right
				// algoresult_fm->fmr_eye_update[c/3 + 71].x =  (int16_t)temp_deq_value;//right
			}
			else 
			{
				algoresult_fm->fmr[LEFT_EYE_MAP_TO_FACE_MESH_INDEX[c/3]].x =  (int16_t)temp_deq_value;//left
				// algoresult_fm->fmr_eye_update[c/3].x =  (int16_t)temp_deq_value;//left
			}
			
		}
		else if((c%3) == 1)
		{
			deq_value = (((deq_value  + (float)eye_shift_y) /(float) FM_INPUT_TENSOR_HEIGHT) * (float)long_Edge);
			int16_t temp_deq_value = (int16_t) deq_value + (int16_t)algoresult_fm->face_bbox[0].y;
			if(algoresult_fm->face_bbox[0].width > algoresult_fm->face_bbox[0].height)
			{
				temp_deq_value = temp_deq_value - ((int16_t)algoresult_fm->face_bbox[0].width - (int16_t)algoresult_fm->face_bbox[0].height)/2;
			}
			//TO-DO Update IL model result to original  eye of face mesh
			// algoresult_fm->fmr[EYE_mesh_index[c/3]].y =  (int16_t)temp_deq_value;
			if(LR==1)
			{
				algoresult_fm->fmr[RIGHT_EYE_MAP_TO_FACE_MESH_INDEX[c/3]].y =  (int16_t)temp_deq_value;//right
				// algoresult_fm->fmr_eye_update[c/3 + 71].y =  (int16_t)temp_deq_value;//right
			}
			else
			{
				algoresult_fm->fmr[LEFT_EYE_MAP_TO_FACE_MESH_INDEX[c/3]].y =  (int16_t)temp_deq_value;//left
				// algoresult_fm->fmr_eye_update[c/3].y =  (int16_t)temp_deq_value;//left

			} 
		}
		else 
		{
			continue;
			// algoresult_fm->fmr[c/3].z =  (int16_t)deq_value;
		}

	}
}
void cal_iris_angle(struct_fm_algoResult_with_fps *algoresult_fm,int LR)
{
	if(LR==1)//right
	{	
		//right iris angle direction
		int right_eye_max_y = algoresult_fm->fmr[RIGHT_EYE_mesh_index[0]].y;
        int right_eye_min_y = algoresult_fm->fmr[RIGHT_EYE_mesh_index[0]].y;
        for(int i=0;i<16;i++)
        {
            if(algoresult_fm->fmr[RIGHT_EYE_mesh_index[i]].y > right_eye_max_y) right_eye_max_y = algoresult_fm->fmr[RIGHT_EYE_mesh_index[i]].y;
            if(algoresult_fm->fmr[RIGHT_EYE_mesh_index[i]].y < right_eye_min_y) right_eye_min_y = algoresult_fm->fmr[RIGHT_EYE_mesh_index[i]].y ;
        }
        int right_eyeball_radius = right_eye_max_y - right_eye_min_y;
        float e_y0 = 0.0;
        float e_x0 = 0.0;
        for(int i=0;i<16;i++)
        {
            e_y0 = e_y0 + algoresult_fm->fmr[RIGHT_EYE_mesh_index[i]].y;
            e_x0 = e_x0 + algoresult_fm->fmr[RIGHT_EYE_mesh_index[i]].x;
        }
                    
        e_y0 = e_y0 / 16.0;
        e_x0 = e_x0 / 16.0;
        float i_y0 = algoresult_fm->fmr_iris[5].y;
        float i_x0 = algoresult_fm->fmr_iris[5].x;
        float theta_r = 0.0;
        float phi_r = 0.0;
        theta_r = (float)(i_y0 - e_y0)/(float)right_eyeball_radius;
        if(theta_r>1)theta_r=1.0;
        else if(theta_r<(-1))theta_r=-1.0;
        theta_r = (-1.0)*asin(theta_r);

        phi_r = (float)(i_x0 - e_x0)/(float)((float)right_eyeball_radius*(-1.0)*cos(theta_r));
        if(phi_r>1)phi_r=1.0;
        else if(phi_r<(-1))phi_r=-1.0;
        phi_r = asin(phi_r);

		algoresult_fm->right_iris_theta = theta_r;
		algoresult_fm->right_iris_phi = phi_r;
	}		
	else //left
	{
		//left iris angle direction 
		int left_eye_max_y = algoresult_fm->fmr[LEFT_EYE_mesh_index[0]].y;
        int left_eye_min_y = algoresult_fm->fmr[LEFT_EYE_mesh_index[0]].y;
        for(int i=0;i<16;i++)
        {
            if(algoresult_fm->fmr[LEFT_EYE_mesh_index[i]].y > left_eye_max_y) left_eye_max_y = algoresult_fm->fmr[LEFT_EYE_mesh_index[i]].y;
            if(algoresult_fm->fmr[LEFT_EYE_mesh_index[i]].y < left_eye_min_y) left_eye_min_y = algoresult_fm->fmr[LEFT_EYE_mesh_index[i]].y ;
        }
        int left_eyeball_radius = left_eye_max_y - left_eye_min_y;
        float e_y0 = 0.0;
        float e_x0 = 0.0;
        for(int i=0;i<16;i++)
        {
            e_y0 = e_y0 + algoresult_fm->fmr[LEFT_EYE_mesh_index[i]].y;
            e_x0 = e_x0 + algoresult_fm->fmr[LEFT_EYE_mesh_index[i]].x;
        }
                    
        e_y0 = e_y0 / 16.0;
        e_x0 = e_x0 / 16.0;
        float i_y0 = algoresult_fm->fmr_iris[0].y;
        float i_x0 = algoresult_fm->fmr_iris[0].x;
        float theta_l = 0.0;
        float phi_l = 0.0;
        theta_l = (float)(i_y0 - e_y0)/(float)left_eyeball_radius;
        if(theta_l>1)theta_l=1.0;
        else if(theta_l<(-1))theta_l=-1.0;
        theta_l = (-1.0)*asin(theta_l);

        phi_l = (float)(i_x0 - e_x0)/(float)((float)left_eyeball_radius*(-1.0)*cos(theta_l));
        if(phi_l>1)phi_l=1.0;
        else if(phi_l<(-1))phi_l=-1.0;
        phi_l = asin(phi_l);

		algoresult_fm->left_iris_theta = theta_l;
		algoresult_fm->left_iris_phi = phi_l;
	} 
}
#endif
#ifdef APP_IRIS_LANDMARK
void blazeface_mesh_post_procees(TfLiteTensor *outputTensor_1,TfLiteTensor *outputTensor_2,struct_fm_algoResult_with_fps *algoresult_fm, struct_position *fm_eye_r_wo_scale_R,struct_position *fm_eye_r_wo_scale_L)
#else
//void blazeface_mesh_post_procees(TfLiteTensor *outputTensor_1,TfLiteTensor *outputTensor_2,struct__box *bbox, int org_img_w, int org_img_h,struct_fm_algoResult_with_fps *algoresult_fm)
void blazeface_mesh_post_procees(TfLiteTensor *outputTensor_1,TfLiteTensor *outputTensor_2,struct_fm_algoResult_with_fps *algoresult_fm)
#endif
{
	float outputTensor_1_scale = ((TfLiteAffineQuantization*)(outputTensor_1->quantization.params))->scale->data[0];
	int outputTensor_1_zeropoint = ((TfLiteAffineQuantization*)(outputTensor_1->quantization.params))->zero_point->data[0];
	//int outputTensor_1_size = outputTensor_1->bytes;

	float outputTensor_2_scale = ((TfLiteAffineQuantization*)(outputTensor_2->quantization.params))->scale->data[0];
	int outputTensor_2_zeropoint = ((TfLiteAffineQuantization*)(outputTensor_2->quantization.params))->zero_point->data[0];
	//int outputTensor_2_size = outputTensor_2->bytes;

#ifdef APP_IRIS_LANDMARK
	for(int i = 0;i<16;i++)
	{
		int c_l = LEFT_EYE_mesh_index[i] * 3;
		int c_r = RIGHT_EYE_mesh_index[i] *3;
		int value_l_x =  outputTensor_1->data.int8[ c_l ];
		
		float deq_value_l_x = ((float) value_l_x-(float)outputTensor_1_zeropoint) * outputTensor_1_scale ;

		int value_l_y =  outputTensor_1->data.int8[ c_l + 1];
		
		float deq_value_l_y = ((float) value_l_y-(float)outputTensor_1_zeropoint) * outputTensor_1_scale ;


		int value_r_x =  outputTensor_1->data.int8[ c_r ];
		
		float deq_value_r_x = ((float) value_r_x-(float)outputTensor_1_zeropoint) * outputTensor_1_scale ;

		int value_r_y =  outputTensor_1->data.int8[ c_r + 1];
		
		float deq_value_r_y = ((float) value_r_y-(float)outputTensor_1_zeropoint) * outputTensor_1_scale ;


		fm_eye_r_wo_scale_R[i].x = (uint32_t)deq_value_r_x;
		fm_eye_r_wo_scale_R[i].y = (uint32_t)deq_value_r_y;

		fm_eye_r_wo_scale_L[i].x = (uint32_t)deq_value_l_x;
		fm_eye_r_wo_scale_L[i].y = (uint32_t)deq_value_l_y;
	}
	
	
#endif


	int org_img_w = algoresult_fm->face_bbox[0].width;
	int org_img_h = algoresult_fm->face_bbox[0].height;
	

	for(int c=0;c<outputTensor_1->dims->data[3];c++)
	{
		int value =  outputTensor_1->data.int8[ c ];
		
		float deq_value = ((float) value-(float)outputTensor_1_zeropoint) * outputTensor_1_scale ;
		int long_Edge;
		if(org_img_w > org_img_h)
		{
			long_Edge = org_img_w;
		}
		else
		{
			long_Edge = org_img_h;
		}
		if((c%3) == 0)
		{
			deq_value = ((deq_value /(float) FM_INPUT_TENSOR_WIDTH) * (float)long_Edge);
		
			/*update x y face_MESH_landmark point from crop start point*/
			int16_t temp_deq_value = (int16_t) deq_value + (int16_t)algoresult_fm->face_bbox[0].x;
			if(algoresult_fm->face_bbox[0].height > algoresult_fm->face_bbox[0].width)
			{
				temp_deq_value = temp_deq_value - ((int16_t)algoresult_fm->face_bbox[0].height - (int16_t)algoresult_fm->face_bbox[0].width)/2;
			}
			algoresult_fm->fmr[c/3].x =  (int16_t)temp_deq_value;
		}
		else if((c%3) == 1)
		{
			deq_value = ((deq_value /(float) FM_INPUT_TENSOR_HEIGHT) * (float)long_Edge);
			int16_t temp_deq_value = (int16_t) deq_value + (int16_t)algoresult_fm->face_bbox[0].y;
			if(algoresult_fm->face_bbox[0].width > algoresult_fm->face_bbox[0].height)
			{
				temp_deq_value = temp_deq_value - ((int16_t)algoresult_fm->face_bbox[0].width - (int16_t)algoresult_fm->face_bbox[0].height)/2;
			}
			algoresult_fm->fmr[c/3].y =  (int16_t)temp_deq_value;
		}
		else 
		{
			continue;
			// algoresult_fm->fmr[c/3].z =  (int16_t)deq_value;
		}
		// printf("[c/3][c%3] [%d][%d]\r\n",c/3,c%3);
	}
	float sigmoid_out2 = 0;
	//float face_threshold = 0.5;
	for(int c=0;c<outputTensor_2->dims->data[3];c++)
	{
		for(int w=0;w<outputTensor_2->dims->data[2];w++)
		{
			for(int h=0;h<outputTensor_2->dims->data[1];h++)
			{
				int value =  outputTensor_2->data.int8[ (w + h*outputTensor_2->dims->data[2]) * outputTensor_2->dims->data[3] + c];
				
				float deq_value = ((float) value-(float)outputTensor_2_zeropoint) * outputTensor_2_scale ;
				#ifdef BLAZEFACE_MESH_DEBUG
					printf("value: %d\r\n",value);
					printf("outputTensor_2_zeropoint: %d\r\n",outputTensor_2_zeropoint);
					printf("outputTensor_2_scale: %f\r\n",outputTensor_2_scale);
					printf("output1: %f\r\n",deq_value);
				#endif	
				sigmoid_out2 = sigmoid(deq_value);
				algoresult_fm->score = sigmoid_out2;
				#ifdef BLAZEFACE_MESH_DEBUG
					printf("sigmoidf output1: %f\r\n",sigmoid_out2);
				#endif	
			}
		}
	}
}













int cv_fd_fm_run(stream_env_t *env,struct_algoResult *alg_result, struct_fm_algoResult_with_fps *alg_fm_result) {

static uint32_t algo_tick = 0;
#if TOTAL_STEP_TICK
	SystemGetTick(&env->systick_1, &env->loop_cnt_1);
#endif
	int ercode = 0;

	float w_scale;
    float h_scale;
	int pad_sz = 0;

    uint32_t img_w = app_get_raw_width();
    uint32_t img_h = app_get_raw_height();
	#ifndef YUV_640_480_INPUT /*RGB_320_240_INPUT*/
    	uint32_t ch = app_get_raw_channels();
    #else
		uint32_t ch = 1;
	#endif
	uint32_t raw_addr = app_get_raw_addr();

	invalidate_cache_for_camera();
    //send jpeg image and no wait
	#if DBG_APP_LOG
    xprintf("raw info: w[%d] h[%d] ch[%d] addr[%x]\n",img_w, img_h, ch, raw_addr);
	#endif
   

#if TOTAL_STEP_TICK						
	SystemGetTick(&env->systick_2, &env->loop_cnt_2);
	algo_tick = (env->loop_cnt_2-env->loop_cnt_1)*CPU_CLK+(env->systick_1-env->systick_2);				
	alg_fm_result->algo_tick = algo_tick + capture_image_tick;				
#endif

#ifdef UART_SEND_ALOGO_RESEULT
	uint32_t judge_case_data;
	uint32_t g_trans_type;
	hx_drv_swreg_aon_get_appused1(&judge_case_data);
	g_trans_type = (judge_case_data>>16);
if( g_trans_type == 0 || g_trans_type == 2)// transfer type is (UART) or (UART & SPI) 
{
	el_img_t temp_el_jpg_img = el_img_t{};
	temp_el_jpg_img.data = (uint8_t *)app_get_jpeg_addr();
	temp_el_jpg_img.size = app_get_jpeg_sz();
	temp_el_jpg_img.width = app_get_raw_width();
	temp_el_jpg_img.height = app_get_raw_height();
	temp_el_jpg_img.format = EL_PIXEL_FORMAT_JPEG;
	temp_el_jpg_img.rotate = EL_PIXEL_ROTATE_0;

	std::forward_list<el_fm_point_t> el_fm_point_algo;

	el_fm_point_t temp_el_fm_point_algo;
	temp_el_fm_point_algo.el_box.x = alg_fm_result->face_bbox[0].x;
	temp_el_fm_point_algo.el_box.y = alg_fm_result->face_bbox[0].y;
	temp_el_fm_point_algo.el_box.w = alg_fm_result->face_bbox[0].width;
	temp_el_fm_point_algo.el_box.h = alg_fm_result->face_bbox[0].height;
	temp_el_fm_point_algo.el_box.score = alg_fm_result->face_bbox[0].face_score;
	temp_el_fm_point_algo.el_box.target = 0;
	for(int c=0;c<FACE_MESH_POINT_NUM;c++)
	{
		
		temp_el_fm_point_algo.el_fm_point[c].x = alg_fm_result->fmr[c].x;
		temp_el_fm_point_algo.el_fm_point[c].y = alg_fm_result->fmr[c].y;
		temp_el_fm_point_algo.el_fm_point[c].score = temp_el_fm_point_algo.el_box.score;
		temp_el_fm_point_algo.el_fm_point[c].target = 0;
	}

	for(int c=0;c<FM_IRIS_POINT_NUM;c++)
	{
		temp_el_fm_point_algo.el_fm_iris[c].x = alg_fm_result->fmr_iris[c].x;
		temp_el_fm_point_algo.el_fm_iris[c].y = alg_fm_result->fmr_iris[c].y;
		temp_el_fm_point_algo.el_fm_iris[c].score = temp_el_fm_point_algo.el_box.score;
		temp_el_fm_point_algo.el_fm_iris[c].target = 0;
	}


	temp_el_fm_point_algo.el_fm_angle.yaw = alg_fm_result->face_angle.yaw * 100.0;
	temp_el_fm_point_algo.el_fm_angle.pitch = alg_fm_result->face_angle.pitch* 100.0;
	temp_el_fm_point_algo.el_fm_angle.roll = alg_fm_result->face_angle.roll* 100.0;
	temp_el_fm_point_algo.el_fm_angle.MAR = alg_fm_result->face_angle.MAR* 100.0;
	temp_el_fm_point_algo.el_fm_angle.LEAR = alg_fm_result->face_angle.LEAR* 100.0;
	temp_el_fm_point_algo.el_fm_angle.REAR = alg_fm_result->face_angle.REAR* 100.0;

	temp_el_fm_point_algo.el_fm_angle.left_iris_theta = alg_fm_result->left_iris_theta* 100.0;
	temp_el_fm_point_algo.el_fm_angle.left_iris_phi = alg_fm_result->left_iris_phi* 100.0;
	temp_el_fm_point_algo.el_fm_angle.right_iris_theta = alg_fm_result->right_iris_theta* 100.0;
	temp_el_fm_point_algo.el_fm_angle.right_iris_phi = alg_fm_result->right_iris_phi* 100.0;

	el_fm_point_algo.emplace_front(temp_el_fm_point_algo);

	std::forward_list<el_box_t> el_fm_face_bbox_algo;

	el_box_t temp_el_fm_face_bbox_algo;
	for(int i = 0; i < MAX_TRACKED_ALGO_RES;i++)
	{
		temp_el_fm_face_bbox_algo.x = alg_fm_result->face_bbox[i].x;
		temp_el_fm_face_bbox_algo.y = alg_fm_result->face_bbox[i].y;
		temp_el_fm_face_bbox_algo.w = alg_fm_result->face_bbox[i].width;
		temp_el_fm_face_bbox_algo.h = alg_fm_result->face_bbox[i].height;
		temp_el_fm_face_bbox_algo.score = alg_fm_result->face_bbox[i].face_score;
		temp_el_fm_face_bbox_algo.target = i;

		el_fm_face_bbox_algo.emplace_front(temp_el_fm_face_bbox_algo);
	}
	send_device_id();
	event_reply(concat_strings(", ", fm_face_bbox_results_2_json_str(el_fm_face_bbox_algo),", ", algo_tick_2_json_str(algo_tick),", ", fm_point_results_2_json_str(el_fm_point_algo), ", ", img_2_json_str(&temp_el_jpg_img)));

}
else if( g_trans_type == 1)// transfer type is (SPI) 
{

}
	set_model_change_by_uart();
#endif

#ifdef UART_SEND_ALOGO_RESEULT
	SystemGetTick(&env->systick_1, &env->loop_cnt_1);
	//recapture image
	sensordplib_retrigger_capture();

	SystemGetTick(&env->systick_2, &env->loop_cnt_2);
	capture_image_tick = (env->loop_cnt_2-env->loop_cnt_1)*CPU_CLK+(env->systick_1-env->systick_2);				
#endif

	return ercode;
}



#endif