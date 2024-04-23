//#include "arm_math_types.h"
#include "sobel.h"
#include <stdlib.h>
#define ATAN_SIMPLE
#ifdef ATAN_SIMPLE
int atan_simplify(int x, int y);
int atan_simplify(int x, int y)
{
	float last = 5.02733949213;			//tan(7pi/16)
	float half_up = 1.49660576267;		//tan(5pi/16)
	float half = 0.66817863791;			//tan(3pi/16)
	float half_down = 0.19891236738;	//tan(pi/16)
	/*int lim = 0
	int same as negatives*/
	//atan(x/abs(y))

	float ratio = abs(x/y);
	if(ratio<half_down)
	{
		return(0);
	}
	else if(ratio<half)
	{
		return(45);
	}
	else if(ratio<half_up)
	{
		return(90);
	}
	else if(ratio<last)
	{
		return(135);
	}
	else
	{
		return(0);
	}
}
#endif
#define INVSQRT_MAGIC_F16           0x59ba      /*  ( 0x1ba = 0x3759df >> 13) */

/* canonical version of INVSQRT_NEWTON_MVE_F16 leads to bad performance */
#define INVSQRT_NEWTON_MVE_F16(invSqrt, xHalf, xStart)                  \
{                                                                       \
    float16x8_t tmp;                                                      \
                                                                        \
    /* tmp = xhalf * x * x */                                           \
    tmp = vmulq(xStart, xStart);                                        \
    tmp = vmulq(tmp, xHalf);                                            \
    /* (1.5f - xhalf * x * x) */                                        \
    tmp = vsubq(vdupq_n_f16((float16_t)1.5), tmp);                      \
    /* x = x*(1.5f-xhalf*x*x); */                                       \
    invSqrt = vmulq(tmp, xStart);                                       \
}

//sobel function for f16
void arm_sobel_in_u8_out_f16_proc_f16(const arm_image_gray_u8_t* ImageIn, arm_image_sobel_f16_t* ImageOut)
{
	//hardcoded value, not good
	int size_x = 240;
	int size_y = 320;
	int16_t gradx;
	int16_t grady;
	ImageOut->numRows = ImageIn->numRows;
	//Main loop
	for( int x =1; x<size_x-1; x++)
	{
		for( int y =1; y < size_y-1; y++)
		{
			//computation of the two gradient
			//using a kernel : [	1, 2, 1,
			//						2, 4, 2,
			//						1, 2, 1 ]
			grady =( ImageIn->pData[(x-1)*ImageIn->numCols+(y-1)] + (ImageIn->pData[(x)*ImageIn->numCols+(y-1)]<<1) + ImageIn->pData[(x+1)*ImageIn->numCols+(y-1)] - ImageIn->pData[(x-1)*ImageIn->numCols+(y+1)] - (ImageIn->pData[(x)*ImageIn->numCols+(y+1)]<<1) - ImageIn->pData[(x+1)*ImageIn->numCols+(y+1)]);
			gradx= (ImageIn->pData[(x-1)*ImageIn->numCols+(y-1)] + (ImageIn->pData[(x-1)*ImageIn->numCols+(y)]<<1) + ImageIn->pData[(x-1)*ImageIn->numCols+(y+1)] - ImageIn->pData[(x+1)*ImageIn->numCols+(y-1)] - (ImageIn->pData[(x+1)*ImageIn->numCols+(y)]<<1) - ImageIn->pData[(x+1)*ImageIn->numCols+(y+1)]);
			//This test is very important, in case both grad = 0, the result of atan is undefined, that causes a performance issue and potential errors
			if(gradx==0&&grady==0)
			{
				ImageOut->pSobelPixel[x*ImageIn->numCols+y].angle = Horizontal;
				ImageOut->pSobelPixel[x*ImageIn->numCols+y].mag = 0;
				continue;
			}
			//Magnitude
			ImageOut->pSobelPixel[x*ImageIn->numCols+y].mag = sqrtf((gradx)*(gradx)+ (grady)*(grady));
			//computation of the angle
			float tan = fabs((atan2(gradx, grady))*(180/PI_F64));
			//simplification of the angle
			//we don't need such a precision on the angle
			//we only need to map the circle of angles to lines, the two diagonals, the vertical and horizontal 
			if(tan< (float)(22))
			{
				ImageOut->pSobelPixel[x*ImageIn->numCols+y].angle = Horizontal;
			}
			else if(tan< (float)(22+45))
			{
				ImageOut->pSobelPixel[x*ImageIn->numCols+y].angle = Diagonal_rl;
			}
			else if(tan< (float)(22+90))
			{
				ImageOut->pSobelPixel[x*ImageIn->numCols+y].angle = Vertical;

			}
			else if(tan< (float)(160))//22+135))
			{
				ImageOut->pSobelPixel[x*ImageIn->numCols+y].angle = Diagonal_lr;
			}
			else
			{
				ImageOut->pSobelPixel[x*ImageIn->numCols+y].angle = Horizontal;
			}
		}
	}
	int x;
	int y;
	x=0;
	//by definition we put the border to 0
	for( int y =0; y < size_y; y++)
	{
		ImageOut->pSobelPixel[x*ImageIn->numCols+y].mag = 0;
	}
	x = size_x-1;
	for( int y =0; y < size_y; y++)
	{
		ImageOut->pSobelPixel[x*ImageIn->numCols+y].mag = 0;
	}
	y = size_y-1;
	for( int x =1; x < size_x; x++)
	{
		ImageOut->pSobelPixel[x*ImageIn->numCols+y].mag = 0;
	}
	y=0;
	for( int x =1; x < size_x; x++)
	{
		ImageOut->pSobelPixel[x*ImageIn->numCols+y].mag = 0;
	}
}

//sobel function for q15
void arm_sobel_in_q15_out_q15_proc_q15( const arm_image_gray_q15_t* ImageIn, arm_image_sobel_q15_t* ImageOut)
{
	int indice;	
	int w = ImageIn->numCols;
	//scale the kernel in order to get a division by 4 of the result
	const q15_t kernel_grady[9] = { 0x2000, 0x4000, 0x2000, 0x0, 0x0, 0x0, 0xE000, 0xC000, 0xE000};
	const q15_t kernel_gradx[9] = { 0x2000, 0x0, 0xE000, 0x4000, 0x0, 0xC000, 0x2000, 0x0, 0xE000};
	q63_t gradx;
	q63_t grady;
	for( int x = 0; x < ImageIn->numRows; x++)
    {
        for( int y =0; y < ImageIn->numCols; y++)
        {
			
			if( x==0 || y==0||y == ImageIn->numCols-1||x== ImageIn->numRows-1)
			{
            	indice = x*ImageIn->numCols + y;
				ImageOut->pSobelPixel[indice].mag = 0;
				ImageOut->pSobelPixel[indice].angle =0;
				continue;
			}
			indice = x*ImageIn->numCols + y;
			//creation matrix input data
			q15_t matrix_in[9] = { ImageIn->pData[indice-w-1], ImageIn->pData[indice-w], ImageIn->pData[indice-w+1], ImageIn->pData[indice-1], ImageIn->pData[indice], ImageIn->pData[indice+1], ImageIn->pData[indice+w-1], ImageIn->pData[indice+w], ImageIn->pData[indice+w+1]};
			
			//dot product between the area 3x3 around the pixel and the two kernel of the gradient
			//calling dot product is less effiscient than just doing the compute our self
			arm_dot_prod_q15( &matrix_in[0], &kernel_gradx[0], 9, &gradx);//gradx in 34.30
			arm_dot_prod_q15( &matrix_in[0], &kernel_grady[0], 9, &grady);//the call of matrix_in and the two kernel maybe can be improve

			//from 34.30 to q15
			gradx = gradx>>15;
			grady = grady>>15;//return to q15
			//Important test avoiding an undefined comportement of atan in case both input value are 0
			if(gradx==0&&grady==0)
			{
				ImageOut->pSobelPixel[x*ImageIn->numCols+y].angle = Horizontal;
				ImageOut->pSobelPixel[x*ImageIn->numCols+y].mag = 0;
				continue;
			}
			//mag
			q15_t vect[2] = { (q15_t)gradx, (q15_t)grady};
			q15_t out;
			q15_t out_tan = 0;
			arm_cmplx_mag_q15( vect, &out, 1);//the magnitude is in 2.14 so a shift is needed

			//atan
			arm_atan2_q15(vect[1], vect[0], &out_tan);
			//doing the absolute value cause an error
			arm_abs_q15( &out_tan, &out_tan, 1);
			ImageOut->pSobelPixel[indice].mag = out<<1;
			//final result in 1.15 BUT the kernel of the gradient was shifted by two so in real we are in 3.13, so we only need to shif by 4 to found the grayscale value
			//mapping the circle of the angles to the four line of horizontal, vertical and the two diagonals
			if((out_tan & 0x7FFF) < (0xC4A))//22 in rad in 2.13
            {
				ImageOut->pSobelPixel[indice].angle = Horizontal;
            }
            else if((out_tan & 0x7FFF) < (0x256C))//67 in rad in 2.13
            {
				ImageOut->pSobelPixel[indice].angle = Diagonal_rl;
            }
            else if((out_tan & 0x7FFF) < (0x3E8E))//112 in rad in 2.13
            {
				ImageOut->pSobelPixel[indice].angle = Vertical;    
            }
            else if((out_tan & 0x7FFF) < (0x595C))//160 in rad in 2.13
            {
				ImageOut->pSobelPixel[indice].angle = Diagonal_lr;
			}
            else//case tan between 135+22.5 and 180
            {
				ImageOut->pSobelPixel[indice].angle = Horizontal;    
            }
		}
	}
}
#if ((!defined(ARM_MATH_MVEI)) |(defined(FORCE_SCALAR)))
//sobel function for q15 using a buffer to avoid repetition of computation Scalar Version
void arm_sobel_in_q15_out_q15_proc_q15_buff( const arm_image_gray_q15_t* ImageIn, arm_image_sobel_q15_t* ImageOut, arm_buffer_2_q15_t* Img_tmp)
{
	int indice;	
	q63_t gradx;
	q63_t grady;
	int x = 0;
	//we start by a first step of initialisation by computing the first line of gradient
	//because we are at the top of the input image, we cannot compute the vetical gradient
	for(int y = 1; y<ImageIn -> numCols- 1; y++)
	{
		Img_tmp->pData[x*Img_tmp->numCols +y].x = (ImageIn->pData[x*ImageIn->numCols+(y-1)] + (ImageIn->pData[x*ImageIn->numCols+(y)]<<1) + ImageIn->pData[x*ImageIn->numCols+(y+1)])>>2;
	}
	x = 1;
	//second step of initialisation we now can compute the second line of the buffer of gradient
	//and this time we can compute the two gradient
	for(int y = 1; y<ImageIn -> numCols- 1; y++)
	{
		Img_tmp->pData[x*Img_tmp->numCols +y].y = (ImageIn->pData[(x-1)*ImageIn->numCols+y] + (ImageIn->pData[x*ImageIn->numCols+y]<<1) + ImageIn->pData[(x+1)*ImageIn->numCols+y])>>2;//possibility to >>2/3 to reduce buffer size to int8 //6value in the buffer aren't used the six on the dorder vertical to painful to adapt code for it
		Img_tmp->pData[x*Img_tmp->numCols +y].x = (ImageIn->pData[x*ImageIn->numCols+(y-1)] + (ImageIn->pData[x*ImageIn->numCols+(y)]<<1) + ImageIn->pData[x*ImageIn->numCols+(y+1)])>>2;
	}
	//main loop
	for( int x = 0; x < ImageIn->numRows; x++)
    {
        for( int y =0; y < ImageIn->numCols; y++)
        {
			int xm = x%3;
			//set the first line to 0 coukld be separated toi removed the if condition
			if( x==0 ||x== ImageIn->numRows)
			{
            	indice = x*ImageIn->numCols + y;
				ImageOut->pSobelPixel[indice].mag = 0;
				ImageOut->pSobelPixel[indice].angle =0;
				continue;
			}
			//this test is for the two colomun on the border without affecting the first and last line
			//it set everything at 0 for the targeted case(mag angle)
			if((y==0||y == ImageIn->numCols-1))
			{
				Img_tmp->pData[xm*Img_tmp->numCols +y].y = (ImageIn->pData[(x-1)*ImageIn->numCols+y] + (ImageIn->pData[x*ImageIn->numCols+y]<<1) + ImageIn->pData[(x+1)*ImageIn->numCols+y])>>2;
				indice = x*ImageIn->numCols + y;
				ImageOut->pSobelPixel[indice].mag = 0;
				ImageOut->pSobelPixel[indice].angle =0;
				continue;
			}
			//the case of the second line could also be removed
			Img_tmp->pData[xm*Img_tmp->numCols +y].y = (ImageIn->pData[(x-1)*ImageIn->numCols+y] + (ImageIn->pData[x*ImageIn->numCols+y]<<1) + ImageIn->pData[(x+1)*ImageIn->numCols+y])>>2;
			Img_tmp->pData[xm*Img_tmp->numCols +y].x = (ImageIn->pData[x*ImageIn->numCols+(y-1)] + (ImageIn->pData[x*ImageIn->numCols+(y)]<<1) + ImageIn->pData[x*ImageIn->numCols+(y+1)])>>2;
			//this serve to complete the initialisation
			if(x==1)
			{
				continue;
			}
			indice = (x-1)*ImageIn->numCols + y;
			//computing the gradient
			gradx = Img_tmp->pData[((x-2)%3)*Img_tmp->numCols +y].x - Img_tmp->pData[(xm)*Img_tmp->numCols +y].x;
			grady = Img_tmp->pData[((x-1)%3)*Img_tmp->numCols +(y-1)].y - Img_tmp->pData[((x-1)%3)*Img_tmp->numCols +(y+1)].y;
			//
			if(gradx==0&&grady==0)
			{
				ImageOut->pSobelPixel[indice].angle = Horizontal;
				ImageOut->pSobelPixel[indice].mag = 0;
				continue;
			}
			//mag
			q15_t vect[2] = { (q15_t)gradx, (q15_t)grady};
			q15_t out;
			q15_t out_tan = 0;

      		q31_t in[2] = {((q31_t)vect[0]), ((q31_t)vect[1])};
      		q31_t out2[2];
      		q31_t out3;
      		q31_t root ;
      		out2[0]=(in[0]*in[0]);
      		out2[1]=(in[1]*in[1]);
      		out3 = (out2[0]+out2[1])>>1;
      		arm_sqrt_q31(out3, &root);
      		out = root>>15;

			//atan
			arm_atan2_q15(vect[0], vect[1], &out_tan);
			//doing the abs introduce a bug we can catch a bit later
			arm_abs_q15( &out_tan, &out_tan, 1);
			ImageOut->pSobelPixel[indice].mag = out;//final result in 1.15 BUT the kernel of the gradient was shifted by two so in real we are in 3.13, so we only need to shif by 4 to found the grayscale value
			//maping of the angle on the fourth direction we consider
			if((out_tan ) < (0xC4A))//22 in rad in 2.13
            {
				ImageOut->pSobelPixel[indice].angle = Horizontal;
            }
            else if((out_tan ) < (0x256C))//67 in rad in 2.13
            {
				ImageOut->pSobelPixel[indice].angle = Diagonal_rl;
            }
            else if((out_tan ) < (0x3E8E))//112 in rad in 2.13
            {
				ImageOut->pSobelPixel[indice].angle = Vertical;    
            }
            else if((out_tan ) < (0x595C))//160 in rad in 2.13
            {
				ImageOut->pSobelPixel[indice].angle = Diagonal_lr;
			}
            else//case tan between 135+22.5 and 180
            {
				ImageOut->pSobelPixel[indice].angle = Horizontal;    
            }
		}
	}
}
#else
//sobel function for q15 using a buffer to avoid repetition of computation vector Version
void arm_sobel_in_q15_out_q15_proc_q15_buff( const arm_image_gray_q15_t* ImageIn, arm_image_sobel_q15_t* ImageOut, arm_buffer_2_q15_t* Img_tmp)
{
	int x = 0;
	int w = ImageIn->numCols;
	//computation of the firt line for the buffer with tailpred to be independant of the image size
	//the tail isn't usefull maybe we can gain perf by removinf it because writting outside the buffer cannot happen here and the next line will be written on just after
	for(int y = 1; y<((ImageIn -> numCols)>>3)+1; y++)
	{
		int indice = ((y-1)<<3) + 1;
		q15x8_t vect1 = vldrhq_s16(&ImageIn->pData[indice-1]);
		vect1 = vshrq_n_u16(vect1, 2);
		q15x8_t vect2 = vldrhq_s16(&ImageIn->pData[indice]);
		vect2 = vshrq_n_s16(vect2, 1);
		q15x8_t vect3 = vldrhq_s16(&ImageIn->pData[indice+1]);
		vect3 = vshrq_n_s16(vect3, 2);
		q15x8_t vect_void = vdupq_n_s16(0);
		vect1 = vaddq_s16(vect2, vect1);
		vect1 = vaddq_s16( vect1, vect3);
		q15x8x2_t vect_buff_di;
		vect_buff_di.val[0] = vect1;
		vect_buff_di.val[1] = vect_void;
		vst2q_s16((int16_t*)&Img_tmp->pData[indice], vect_buff_di);
	}

	//tail for the first line maybe not necessary
	int numtail = (w-1)%8;//loop necessary
	if(numtail>0)
	{
		int indice = (w-2)&0xFFF8;
		mve_pred16_t vtail = vctp16q(numtail);
		q15x8_t vect1 = vldrhq_z_s16(&ImageIn->pData[indice-1], vtail);
		vect1 = vshrq_m_n_u16(vect1, vect1, 2, vtail);
		q15x8_t vect2 =vldrhq_z_s16(&ImageIn->pData[indice], vtail);
		vect2 = vshrq_m_n_u16(vect2,vect2, 1, vtail);
		q15x8_t vect3 =vldrhq_z_s16(&ImageIn->pData[indice+1], vtail);
		vect3 = vshrq_m_n_s16(vect3,vect3, 2, vtail);
		vect1 = vaddq_s16(vect1, vect2);
		vect1 = vaddq_s16(vect1, vect3);
		for(int j=0; j<numtail; j++)
		{
			Img_tmp->pData[indice+j].x = vect1[j];
		}
	}
	x =1;
	//second line of the buffer now we can compute both composant of the buffer, 
	//stil the tail isn't necessary 
	Img_tmp->pData[x*w].y = (ImageIn->pData[(x-1)*w] + (ImageIn->pData[x*w]<<1) + ImageIn->pData[(x+1)*w])>>2;
	for(int y =1; y<((w)>>3); y++)
	{
		int indice = w +((y-1)<<3)+1;//something not good here really wil not work
		q15x8_t vect1 = vldrhq_s16(&ImageIn->pData[indice-1]);
		vect1 = vshrq_n_s16(vect1, 2);
		q15x8_t vect2 = vldrhq_s16(&ImageIn->pData[indice]);
		vect2 = vshrq_n_s16(vect2, 1);
		q15x8_t vect3 = vldrhq_s16(&ImageIn->pData[indice+1]);
		vect3 = vshrq_n_s16(vect3, 2);
		vect1 = vaddq_s16(vect1, vect2);
		vect1 = vaddq_s16(vect1, vect3);
		q15x8_t vecth = vldrhq_s16(&ImageIn->pData[indice-w]);
		vecth = vshrq_n_u16(vecth, 2);
		q15x8_t vectb = vldrhq_s16(&ImageIn->pData[indice+w]);
		vectb = vshrq_n_s16(vectb, 2);
		vectb = vaddq_s16(vectb, vect2);
		vectb = vaddq_s16(vectb, vecth);
		q15x8x2_t vect_buff_di;
		vect_buff_di.val[0] = vect1;
		vect_buff_di.val[1] = vectb;
		
		vst2q_s16(&Img_tmp->pData[indice].x, vect_buff_di);//no prediction so issue i think
	}
	if(numtail>0)
	{
		int indice = w + ((w-2)&0xFFF8);
		mve_pred16_t vtail = vctp16q(numtail);
		q15x8_t vect1 = vldrhq_z_s16(&ImageIn->pData[indice-1], vtail);
		vect1 = vshrq_m_n_s16(vect1, vect1, 2, vtail);
		q15x8_t vect2 =vldrhq_z_s16(&ImageIn->pData[indice], vtail);
		vect2 = vshrq_m_n_s16(vect2,vect2, 1, vtail);
		q15x8_t vect3 =vldrhq_z_s16(&ImageIn->pData[indice+1], vtail);
		vect3 = vshrq_m_n_s16(vect3,vect3, 2, vtail);
		vect1 = vaddq_s16(vect1, vect2);
		vect1 = vaddq_s16(vect1, vect3);
		q15x8_t vecth = vldrhq_s16(&ImageIn->pData[indice-w]);
		vecth = vshrq_n_s16(vecth, 2);
		q15x8_t vectb = vldrhq_s16(&ImageIn->pData[indice+w]);
		vectb = vshrq_n_s16(vectb, 2);
		vectb = vaddq_s16(vectb, vect2);
		vectb = vaddq_s16(vectb, vecth);
		for(int j=0; j<numtail+2; j++)
		{
			Img_tmp->pData[indice + j].x = vect1[j];
			Img_tmp->pData[indice + j].y = vectb[j];
		}
	}
	Img_tmp->pData[w+w-1].y = (ImageIn->pData[(x-1)*w+w-1] + (ImageIn->pData[x*w+w-1]<<1) + ImageIn->pData[(x+1)*w+w-1])>>2;
	//Main Loop
	//the tail is necessary or we need to increase the size of the buffer by 8 elements to be safe
	//if not we nee a tail
	//we first start to compute a vector of the buffer then we go to compute the vector for the sobel image
	for(int x =2; x< ImageIn->numRows; x++)
	{
		int xm = x%3;
		int xm1 = (x-1)%3;
		int xm2 = (x-2)%3;
		//first value of the line only have y conponent
		Img_tmp->pData[xm*w].y = (ImageIn->pData[(x-1)*w] + (ImageIn->pData[x*w]<<1) + ImageIn->pData[(x+1)*w])>>2;
		//loop computing one vector for the buffer and one vector for the output for each turn
		for(int y =1; y< w-7;y+=8)
		{
			int indice = x*w + y;
			int indicem = xm*w + y;
			int indicem1 = xm1*w+y;
			int indicem2 = xm2*w + y;
			q15x8_t vect1 = vldrhq_s16(&ImageIn->pData[indice-1]);
			q15x8_t vect2 = vldrhq_s16(&ImageIn->pData[indice]);
			//all the vector read are scaled in order to not overflow
			vect1 = vshrq_n_s16(vect1, 2);
			q15x8_t vect3 = vldrhq_s16(&ImageIn->pData[indice+1]);
			vect2 = vshrq_n_s16(vect2, 1);
			
			q15x8_t vecth = vldrhq_s16(&ImageIn->pData[indice-w]);
			vect1 = vaddq_s16(vect1, vect2);
			vect3 = vshrq_n_s16(vect3, 2);
			
			vecth = vshrq_n_s16(vecth, 2);
			
			q15x8_t vectb = vldrhq_s16(&ImageIn->pData[indice+w]);
			vect1 = vaddq_s16(vect1, vect3);
			vectb = vshrq_n_s16(vectb, 2);
			vectb = vaddq_s16(vectb, vect2);
			
			q15x8x2_t vect_buff_di;
			vect_buff_di.val[0] = vect1;
			vectb = vaddq_s16(vectb, vecth);
			vect_buff_di.val[1] = vectb;
			q15x8x2_t vec_x_y_1 = vld2q_s16(&Img_tmp->pData[indicem2].x);
			//we here store the two component of the buffer in one go, the first vector contain the partial sum on x, second on y
			vst2q_s16(&Img_tmp->pData[indicem].x, vect_buff_di);			
			q15x8x2_t vec_y_1 = vld2q_s16(&Img_tmp->pData[indicem1-1].x);
			

			q15x8_t vectgradx = vsubq_s16(vec_x_y_1.val[0], vect_buff_di.val[0]);
			q15x8x2_t vec_y_2 = vld2q_s16(&Img_tmp->pData[indicem1+1].x);
			int numVect = 8;
			int16x8_t vect_res_1;
			int16x8_t vect_res_2;
			//This multiplication would cause an overflow so we have to increase the size of the data typpe of the vector 
			//that lead to work with two vector, one containing our even value and one the odd
			q31x4_t vect_gradx_1 = vmullbq_int_s16(vectgradx, vectgradx);
			q15x8_t vectgrady = vsubq_s16(vec_y_1.val[1], vec_y_2.val[1]);
			q31x4_t vect_grady_1 = vmullbq_int_s16(vectgrady, vectgrady);
			vect_gradx_1 = vaddq_s32(vect_gradx_1, vect_grady_1);
			q31x4_t vect_gradx_2 = vmulltq_int_s16(vectgradx, vectgradx);
			vect_gradx_1 = vshrq(vect_gradx_1, 1);
			q31x4_t vect_grady_2 = vmulltq_int_s16(vectgrady, vectgrady);
			vect_gradx_2 = vaddq_s32(vect_gradx_2, vect_grady_2);
			vect_gradx_2 = vshrq(vect_gradx_2, 1);
			//two loops one for each of the two vectors we have
			for(int j =0; j < numVect; j+=2)
			{
				if(vectgradx[j]==0&&vectgrady[j]==0)
				{
					vect_res_2[j] = Horizontal;
					vect_res_1[j] =  0;
					continue;
				}
				//mag
				q15_t out;
				q15_t out_tan = 0;
				q31_t root ;
				arm_sqrt_q31(vect_gradx_1[j>>1], &root);
      			out = root>>15;
				vect_res_1[j] = out;
				arm_atan2_q15(vectgradx[j], vectgrady[j], &out_tan);
				arm_abs_q15( &out_tan, &out_tan, 1);
				if((out_tan) < (0xC4A))//22 in rad in 2.13
            	{
					vect_res_2[j] = Horizontal;
					continue;
            	}
            	else if((out_tan ) < (0x256C))//67 in rad in 2.13
            	{
					vect_res_2[j] = Diagonal_rl;
					continue;
            	}
            	else if((out_tan) < (0x3E8E))//112 in rad in 2.13
            	{
					vect_res_2[j] = Vertical;    
            		continue;
				}
            	else if((out_tan) < (0x595C))//160 in rad in 2.13
            	{
					vect_res_2[j] = Diagonal_lr;
					continue;
				}
            	else//case tan between 135+22.5 and 180
            	{
					vect_res_2[j] = Horizontal;    
            		continue;
				}
			}
			//second loop for treating the second vector we got 
			for(int j =1; j < numVect; j+=2)
			{
				if(vectgradx[j]==0&&vectgrady[j]==0)
				{
					vect_res_2[j] = Horizontal;
					vect_res_1[j] =  0;
					continue;
				}
				//mag
				q15_t out;
				q15_t out_tan = 0;
				q31_t root ;
				arm_sqrt_q31(vect_gradx_2[j>>1], &root);
      			out = root>>15;
				vect_res_1[j] = out;

				arm_atan2_q15(vectgradx[j], vectgrady[j], &out_tan);
				arm_abs_q15( &out_tan, &out_tan, 1);
				out_tan = out_tan & 0x7FFF;
				if((out_tan) < (0xC4A))//22 in rad in 2.13
            	{
					vect_res_2[j] = Horizontal;
            		continue;
				}
            	else if((out_tan) < (0x256C))//67 in rad in 2.13
            	{
					vect_res_2[j] = Diagonal_rl;
					continue;
            	}
            	else if((out_tan) < (0x3E8E))//112 in rad in 2.13
            	{
					vect_res_2[j] = Vertical;    
            		continue;
				}
            	else if((out_tan) < (0x595C))//160 in rad in 2.13
            	{
					vect_res_2[j] = Diagonal_lr;
					continue;
				}
            	else//case tan between 135+22.5 and 180
            	{
					vect_res_2[j] = Horizontal;    
            		continue;
				}
				vect_res_2[j] = vect_res_2[j];
			}
			int16x8x2_t vect_res;
			vect_res.val[0] = vect_res_1;
			vect_res.val[1] = vect_res_2;
			vst2q_s16((int16_t*)&ImageOut->pSobelPixel[indice-w], vect_res);
		}
		if(numtail>0)
		{
			for(int j = 0; j<numtail+1; j++)
			{
				int y = ((w-2)&0xFFF8) + j;
				if((y==0||y == w-1)&&x!=0&& x != ImageIn->numRows-1)
				{
					Img_tmp->pData[xm*w +y].y = (ImageIn->pData[(x-1)*w+y] + (ImageIn->pData[x*w+y]<<1) + ImageIn->pData[(x+1)*w+y])>>2;
					int indice = x*w + y;
					ImageOut->pSobelPixel[indice].mag = 0;
					ImageOut->pSobelPixel[indice].angle =0;
					continue;
				}
				if( x==0 || y==0||y == w-1)
				{
            		int indice = x*w + y;
					ImageOut->pSobelPixel[indice].mag = 0;
					ImageOut->pSobelPixel[indice].angle =0;
					continue;
				}
				Img_tmp->pData[xm*w +y].y = (ImageIn->pData[(x-1)*w+y] + (ImageIn->pData[x*w+y]<<1) + ImageIn->pData[(x+1)*w+y])>>2;
				Img_tmp->pData[xm*w +y].x = (ImageIn->pData[x*w+(y-1)] + (ImageIn->pData[x*w+(y)]<<1) + ImageIn->pData[x*w+(y+1)])>>2;
				if(x==1)
				{
					continue;
				}
				int indice = (x-1)*w + y;
				q63_t gradx = Img_tmp->pData[xm2*w +y].x - Img_tmp->pData[(xm)*w +y].x;
				q63_t grady = Img_tmp->pData[xm1*w +(y-1)].y - Img_tmp->pData[xm1*w +(y+1)].y;
				if(gradx==0&&grady==0)
				{
					ImageOut->pSobelPixel[indice].angle = Horizontal;
					ImageOut->pSobelPixel[indice].mag = 0;
					continue;
				}
				//mag
				q15_t vect[2] = { (q15_t)gradx, (q15_t)grady};
				q15_t out;
				q15_t out_tan = 0;

      			q31_t in[2] = {((q31_t)vect[0]), ((q31_t)vect[1])};
      			q31_t out2[2];
      			q31_t out3;
      			q31_t root ;
      			out2[0]=(in[0]*in[0]);
      			out2[1]=(in[1]*in[1]);
      			out3 = (out2[0]+out2[1])>>1;
      			arm_sqrt_q31(out3, &root);
      			out = root>>15;

				//atan
				arm_atan2_q15(vect[0], vect[1], &out_tan);
				arm_abs_q15( &out_tan, &out_tan, 1);
				ImageOut->pSobelPixel[indice].mag = out;//final result in 1.15 BUT the kernel of the gradient was shifted by two so in real we are in 3.13, so we only need to shif by 4 to found the grayscale value
				if((out_tan) < (0xC4A))//22 in rad in 2.13
            	{
					ImageOut->pSobelPixel[indice].angle = Horizontal;
					continue;
            	}
            	else if((out_tan) < (0x256C))//67 in rad in 2.13
            	{
					ImageOut->pSobelPixel[indice].angle = Diagonal_rl;
					continue;
            	}
            	else if((out_tan) < (0x3E8E))//112 in rad in 2.13
            	{
					ImageOut->pSobelPixel[indice].angle = Vertical;    
            		continue;
				}
            	else if((out_tan) < (0x595C))//160 in rad in 2.13
            	{
					ImageOut->pSobelPixel[indice].angle = Diagonal_lr;
					continue;
				}
            	else//case tan between 135+22.5 and 180
            	{
					ImageOut->pSobelPixel[indice].angle = Horizontal;    
            		continue;
				}
			}
		}
	}
}
#endif
#if ((!defined(ARM_MATH_MVEI)) |(defined(FORCE_SCALAR)))
//sobel function for f16 using a buffer to avoid repetition of computation Scalar Version
void arm_sobel_in_u8_out_f16_proc_f16_buff(const arm_image_gray_u8_t* ImageIn, arm_image_sobel_f16_t* ImageOut, arm_buffer_2_u16_t* Img_tmp)
{
	int size_x = 240;
	int size_y = 320;
	int16_t gradx;
	int16_t grady;
	ImageOut->numRows = ImageIn->numRows;
	int x = 0;
	//First line of the buffer compute
	for(int y = 1; y<ImageIn -> numCols- 1; y++)
	{
		Img_tmp->pData[x*Img_tmp->numCols +y].x = ImageIn->pData[x*ImageIn->numCols+(y-1)] + (ImageIn->pData[x*ImageIn->numCols+(y)]<<1) + ImageIn->pData[x*ImageIn->numCols+(y+1)];
	}
	x = 1;
	//Second line of the buffer this time it's possible to compute the two partial sum of the gradient
	Img_tmp->pData[x*Img_tmp->numCols].y = ImageIn->pData[(x-1)*ImageIn->numCols] + (ImageIn->pData[x*ImageIn->numCols]<<1) + ImageIn->pData[(x+1)*ImageIn->numCols];
	for(int y = 1; y<ImageIn -> numCols- 1; y++)
	{
		Img_tmp->pData[x*Img_tmp->numCols +y].y = ImageIn->pData[(x-1)*ImageIn->numCols+y] + (ImageIn->pData[x*ImageIn->numCols+y]<<1) + ImageIn->pData[(x+1)*ImageIn->numCols+y];//possibility to >>2/3 to reduce buffer size to int8 //6value in the buffer aren't used the six on the dorder vertical to painful to adapt code for it
		Img_tmp->pData[x*Img_tmp->numCols +y].x = ImageIn->pData[x*ImageIn->numCols+(y-1)] + (ImageIn->pData[x*ImageIn->numCols+(y)]<<1) + ImageIn->pData[x*ImageIn->numCols+(y+1)];
	}
	Img_tmp->pData[x*Img_tmp->numCols+size_y-1].y = ImageIn->pData[(x-1)*ImageIn->numCols+size_y-1] + (ImageIn->pData[x*ImageIn->numCols+size_y-1]<<1) + ImageIn->pData[(x+1)*ImageIn->numCols+size_y-1];
	//Main loop
	for( int x =2; x<size_x-1; x++)
	{
		int xm = x%3;
		int y = 0;
		//first value on a line we can only compute the partial sum on x for the buffer
		Img_tmp->pData[xm*Img_tmp->numCols +y].y = ImageIn->pData[(x-1)*ImageIn->numCols+y] + (ImageIn->pData[x*ImageIn->numCols+y]<<1) + ImageIn->pData[(x+1)*ImageIn->numCols+y];
		//Main loop where we compute the buffer value and the output value
		for( int y =1; y < size_y-1; y++)
		{
			//first computing of the intermediate value and putting them on the buffer
			Img_tmp->pData[xm*Img_tmp->numCols +y].y = ImageIn->pData[(x-1)*ImageIn->numCols+y] + (ImageIn->pData[x*ImageIn->numCols+y]<<1) + ImageIn->pData[(x+1)*ImageIn->numCols+y];
			Img_tmp->pData[xm*Img_tmp->numCols +y].x = ImageIn->pData[x*ImageIn->numCols+(y-1)] + (ImageIn->pData[x*ImageIn->numCols+(y)]<<1) + ImageIn->pData[x*ImageIn->numCols+(y+1)];
			//then computing of the gradient
			gradx = Img_tmp->pData[((x-2)%3)*Img_tmp->numCols +y].x - Img_tmp->pData[(xm)*Img_tmp->numCols +y].x;
			grady = Img_tmp->pData[((x-1)%3)*Img_tmp->numCols +(y-1)].y - Img_tmp->pData[((x-1)%3)*Img_tmp->numCols +(y+1)].y;		
			//Important test to avoid undefined behavior if 
			if(gradx==0&&grady==0)
			{
				ImageOut->pSobelPixel[(x-1)*ImageIn->numCols+y].angle = Horizontal;
				ImageOut->pSobelPixel[(x-1)*ImageIn->numCols+y].mag = 0;
				continue;
			}
			//Computing of the magnitude
			ImageOut->pSobelPixel[(x-1)*ImageIn->numCols+y].mag = sqrtf((gradx)*(gradx)+ (grady)*(grady));
			//Computing of the angle, here only atan is requiere and it run faster than atan2 so it's better
			float tan = atan(gradx/abs(grady))*(180/PI_F64);
			//selecting the correct case corresponding to the angle and writing it to the output
			if(tan < (float)(22))
			{
				if(tan < (float)(-68))
				{
					ImageOut->pSobelPixel[(x-1)*ImageIn->numCols+y].angle = Vertical;
				}
				else if(tan <(float)(-20))
				{
					ImageOut->pSobelPixel[(x-1)*ImageIn->numCols+y].angle = Diagonal_lr;
					continue;
				}
				else
				{
					ImageOut->pSobelPixel[(x-1)*ImageIn->numCols+y].angle = Horizontal;
				}
			}
			else if(tan <(float)(67))
			{
				ImageOut->pSobelPixel[(x-1)*ImageIn->numCols+y].angle = Diagonal_rl;
				continue;
			}
			else
			{
				ImageOut->pSobelPixel[(x-1)*ImageIn->numCols+y].angle = Vertical;
			}
		}
		y = size_y-1;
		Img_tmp->pData[xm*Img_tmp->numCols +y].y = ImageIn->pData[(x-1)*ImageIn->numCols+y] + (ImageIn->pData[x*ImageIn->numCols+y]<<1) + ImageIn->pData[(x+1)*ImageIn->numCols+y];
	}
	//last line done indepedently
	x = 239;
	int xm = x%3;
	for( int y =1; y < size_y-1; y++)
	{
		//we only compute the partial sum on x because theire's no line futher behind to compute it on y
		Img_tmp->pData[xm*Img_tmp->numCols +y].x = ImageIn->pData[x*ImageIn->numCols+(y-1)] + (ImageIn->pData[x*ImageIn->numCols+(y)]<<1) + ImageIn->pData[x*ImageIn->numCols+(y+1)];
		//computation of the gradient
		gradx = Img_tmp->pData[((x-2)%3)*Img_tmp->numCols +y].x - Img_tmp->pData[(xm)*Img_tmp->numCols +y].x;
		grady = Img_tmp->pData[((x-1)%3)*Img_tmp->numCols +(y-1)].y - Img_tmp->pData[((x-1)%3)*Img_tmp->numCols +(y+1)].y;
		//avoid undefined behavior
		if(gradx==0&&grady==0)
		{
			ImageOut->pSobelPixel[(x-1)*ImageIn->numCols+y].angle = Horizontal;
			ImageOut->pSobelPixel[(x-1)*ImageIn->numCols+y].mag = 0;
			continue;
		}
		//Computation of magnitude
		ImageOut->pSobelPixel[(x-1)*ImageIn->numCols+y].mag = sqrtf((gradx)*(gradx)+ (grady)*(grady));
		//computation and simplification of the angle
		float tan = atan(gradx/abs(grady))*(180/PI_F64);
		if(tan < (float)(22))
		{
			if(tan < (float)(-68))
			{
				ImageOut->pSobelPixel[(x-1)*ImageIn->numCols+y].angle = Vertical;
			}
			else if(tan <(float)(-20))
			{
				ImageOut->pSobelPixel[(x-1)*ImageIn->numCols+y].angle = Diagonal_lr;
				continue;
			}
			else
			{
				ImageOut->pSobelPixel[(x-1)*ImageIn->numCols+y].angle = Horizontal;
			}
		}
		else if(tan <(float)(67))
		{
			ImageOut->pSobelPixel[(x-1)*ImageIn->numCols+y].angle = Diagonal_rl;
			continue;
		}
		else
		{
			ImageOut->pSobelPixel[(x-1)*ImageIn->numCols+y].angle = Vertical;
		}
	}
	//border of the image are set to 0
	int y;
	x=0;
	for( int y =0; y < size_y; y++)
	{
		ImageOut->pSobelPixel[x*ImageIn->numCols+y].mag = 0;
	}
	x = size_x-1;
	for( int y =0; y < size_y; y++)
	{
		ImageOut->pSobelPixel[x*ImageIn->numCols+y].mag = 0;
	}
	y = size_y-1;
	for( int x =1; x < size_x; x++)
	{
		ImageOut->pSobelPixel[x*ImageIn->numCols+y].mag = 0;
	}
	y=0;
	for( int x =1; x < size_x; x++)
	{
		ImageOut->pSobelPixel[x*ImageIn->numCols+y].mag = 0;
	}
}
#else
//sobel function for f16 using a buffer to avoid repetition of computation vector Version
#if 1//(!defined SQRT_ATAN_VECT)
void arm_sobel_in_u8_out_f16_proc_f16_buff(const arm_image_gray_u8_t* ImageIn, arm_image_sobel_f16_t* ImageOut, arm_buffer_2_u16_t* Img_tmp)
{
	int size_x = 240;
	int size_y = 320;
	//int16_t gradx;
	//int16_t grady;
	ImageOut->numRows = ImageIn->numRows;
	int numtail = (ImageIn->numCols-1)%8;
	int x = 0;
	for(int y =1; y<((ImageIn->numCols)>>3); y++)
	{
		int indice = ((y-1)<<3)+1;
		uint16x8_t vect11 = vldrbq_u16(&ImageIn->pData[indice-1]);
		uint16x8_t vect2 = vldrbq_u16(&ImageIn->pData[indice]);
		vect2 = vshlq_n_u16(vect2, 1);
		uint16x8_t vect31 = vldrbq_u16(&ImageIn->pData[indice+1]);
		vect11 = vaddq_u16(vect11, vect2);
		vect11 = vaddq_u16(vect11, vect31);
		q15x8_t vect_void = vdupq_n_s16(0);
		uint16x8x2_t vect_buff_di1;
		vect_buff_di1.val[0] = vect11;
		vect_buff_di1.val[1] = vect_void;
		vst2q_u16(&Img_tmp->pData[indice].x, vect_buff_di1);
	}
	if(numtail>0)
	{
		int indice = (((ImageIn -> numCols-2)>>3) <<3);
		mve_pred16_t vtail = vctp16q(numtail);
		uint16x8_t vect1 = vldrbq_z_u16(&ImageIn->pData[indice-1], vtail);
		uint16x8_t vect2 =vldrbq_z_u16(&ImageIn->pData[indice], vtail);
		vect2 = vshlq_m_n_u16(vect2,vect2, 1, vtail);
		uint16x8_t vect3 =vldrbq_z_u16(&ImageIn->pData[indice+1], vtail);
		vect1 = vaddq_u16(vect1, vect2);//vtail or not?
		vect1 = vaddq_u16(vect1, vect3);//idem
		for(int j=0; j<numtail+1; j++)
		{
			Img_tmp->pData[indice + j].x = vect1[j];
			Img_tmp->pData[indice + j].y = 0;
		}
	}
	x = 1;
	Img_tmp->pData[x*Img_tmp->numCols].y = ImageIn->pData[(x-1)*ImageIn->numCols] + (ImageIn->pData[x*ImageIn->numCols]<<1) + ImageIn->pData[(x+1)*ImageIn->numCols];
	for(int y =1; y<((ImageIn->numCols)>>3); y++)
	{
		int indice = ImageIn->numCols +((y-1)<<3)+1;
		uint16x8_t vect11 = vldrbq_u16(&ImageIn->pData[indice-1]);
		uint16x8_t vect2 = vldrbq_u16(&ImageIn->pData[indice]);
		vect2 = vshlq_n_u16(vect2, 1);
		uint16x8_t vect31 = vldrbq_u16(&ImageIn->pData[indice+1]);
		vect11 = vaddq_u16(vect11, vect2);
		vect11 = vaddq_u16(vect11, vect31);
		uint16x8_t vecth1 = vldrbq_u16(&ImageIn->pData[indice-ImageIn->numCols]);
		uint16x8_t vectb1 = vldrbq_u16(&ImageIn->pData[indice+ImageIn->numCols]);
		vectb1 = vaddq_s16(vectb1, vect2);
		vectb1 = vaddq_s16(vectb1, vecth1);
		uint16x8x2_t vect_buff_di1;
		vect_buff_di1.val[0] = vect11;
		vect_buff_di1.val[1] = vectb1;
		vst2q_u16(&Img_tmp->pData[indice].x, vect_buff_di1);
	}
	if(numtail>0)
	{
		int indice = ImageIn->numCols + (((ImageIn -> numCols-2)>>3) <<3);
		mve_pred16_t vtail = vctp16q(numtail);
		uint16x8_t vect1 = vldrbq_z_u16(&ImageIn->pData[indice-1], vtail);
		uint16x8_t vect2 =vldrbq_z_u16(&ImageIn->pData[indice], vtail);
		vect2 = vshlq_m_n_u16(vect2,vect2, 1, vtail);
		uint16x8_t vect3 =vldrbq_z_u16(&ImageIn->pData[indice+1], vtail);
		vect1 = vaddq_u16(vect1, vect2);//vtail or not?
		vect1 = vaddq_u16(vect1, vect3);//idem
		uint16x8_t vecth = vldrbq_u16(&ImageIn->pData[indice-ImageIn->numCols]);
		uint16x8_t vectb = vldrbq_u16(&ImageIn->pData[indice+ImageIn->numCols]);
		vectb = vaddq_u16(vectb, vect2);
		vectb = vaddq_u16(vectb, vecth);
		for(int j=0; j<numtail+2; j++)
		{
			Img_tmp->pData[indice + j].x = vect1[j];
			Img_tmp->pData[indice + j].y = vectb[j];
		}
	}
	Img_tmp->pData[x*Img_tmp->numCols+size_y-1].y = ImageIn->pData[(x-1)*ImageIn->numCols+size_y-1] + (ImageIn->pData[x*ImageIn->numCols+size_y-1]<<1) + ImageIn->pData[(x+1)*ImageIn->numCols+size_y-1];
	for( int x =2; x<size_x; x++)
	{
		int xm = x%3;
		int y = 0;
		Img_tmp->pData[xm*Img_tmp->numCols +y].y = ImageIn->pData[(x-1)*ImageIn->numCols+y] + (ImageIn->pData[x*ImageIn->numCols+y]<<1) + ImageIn->pData[(x+1)*ImageIn->numCols+y];
		for( int y =1; y < size_y-numtail-1; y+=8)
		{
			/*
			if(gradx==0&&grady==0)
			{
				ImageOut->pSobelPixel[(x-1)*ImageIn->numCols+y].angle = Horizontal;
				ImageOut->pSobelPixel[(x-1)*ImageIn->numCols+y].mag = 0;
				continue;
			}*/
			int indice = x*ImageIn->numCols + y;
			int indicem = xm*ImageIn->numCols + y;
			uint16x8_t vect11 = vldrbq_u16(&ImageIn->pData[indice-1]);
			uint16x8_t vect2 = vldrbq_u16(&ImageIn->pData[indice]);
			vect2 = vshlq_n_u16(vect2, 1);
			uint16x8_t vect31 = vldrbq_u16(&ImageIn->pData[indice+1]);
			vect11 = vaddq_u16(vect11, vect2);
			vect11 = vaddq_u16(vect11, vect31);
			uint16x8_t vecth1 = vldrbq_u16(&ImageIn->pData[indice-ImageIn->numCols]);
			uint16x8_t vectb1 = vldrbq_u16(&ImageIn->pData[indice+ImageIn->numCols]);
			vectb1 = vaddq_s16(vectb1, vect2);
			vectb1 = vaddq_s16(vectb1, vecth1);
			uint16x8x2_t vect_buff_di1;
			vect_buff_di1.val[0] = vect11;
			vect_buff_di1.val[1] = vectb1;
			vst2q_u16(&Img_tmp->pData[indicem].x, vect_buff_di1);
			
			uint16x8x2_t vec_x_y_1 = vld2q_u16(&Img_tmp->pData[((x-2)%3)*ImageIn->numCols + y].x);
			uint16x8x2_t vec_x_2 = vld2q_u16(&Img_tmp->pData[indicem].x);
			uint16x8x2_t vec_y_1 = vld2q_u16(&Img_tmp->pData[((x-1)%3)*ImageIn->numCols + y-1].x);
			uint16x8x2_t vec_y_2 = vld2q_u16(&Img_tmp->pData[((x-1)%3)*ImageIn->numCols + y+1].x);
			int16x8_t vectgradx = vsubq_s16(vec_x_y_1.val[0], vec_x_2.val[0]);
			int16x8_t vectgrady = vsubq_s16(vec_y_1.val[1], vec_y_2.val[1]);
			//int numVect = 8;
			float16x8_t vect_res_1;
			float16x8_t vect_res_2;
			int32x4_t vect_gradx_1 = vmullbq_int_s16(vectgradx, vectgradx);
			int32x4_t vect_gradx_2 = vmulltq_int_s16(vectgradx, vectgradx);
			int32x4_t vect_grady_1 = vmullbq_int_s16(vectgrady, vectgrady);
			int32x4_t vect_grady_2 = vmulltq_int_s16(vectgrady, vectgrady);
			vect_gradx_1 = vaddq_s32(vect_gradx_1, vect_grady_1);
			vect_gradx_2 = vaddq_s32(vect_gradx_2, vect_grady_2);

			for(int i = 0; i< 8; i++)
			{
				//vect for grad
				//gradx = vectgradx[i];
				//magnitude
				
				//VERY IMPORTANT PIECE OF CODE
				//allow you to avoid the case 0 0 in the tan causing huge perf loses
				if(vectgradx[i]==0&&vectgrady[i]==0)
				{
					vect_res_2[i] = Horizontal;
					vect_res_1[i] = 0;
					continue;
				}


				if(i%2)
				{
					vect_res_1[i] = sqrtf(vect_gradx_2[i>>1]);
				}
				else
				{
					vect_res_1[i] = sqrtf(vect_gradx_1[i>>1]);
				}
				/*float tan*/ImageOut->pSobelPixel[(x-1)*ImageIn->numCols+y+i].angle = atan_simplify(/*gradx/abs(grady))*(180/PI_F64)*/vectgradx[i],vectgrady[i]);
				/*if(tan < (float)(22))
				{
					if(tan < (float)(-68))
					{
						vect_res_2[i] = Vertical;
					}
					else if(tan <(float)(-20))
					{
						ImageOut->pSobelPixel[(x-1)*ImageIn->numCols+y+i].angle = Diagonal_lr;
						vect_res_2[i] = Diagonal_lr;
					}
					else
					{
						vect_res_2[i] = Horizontal;
					}
				}
				else if(tan <(float)(67))
				{
					ImageOut->pSobelPixel[(x-1)*ImageIn->numCols+y+i].angle = Diagonal_rl;
					vect_res_2[i] = Diagonal_rl;
				}
				else
				{
					vect_res_2[i] = Vertical;
				}*/
			}
			float16x8x2_t vect_res;
			vect_res.val[0] = vect_res_1;
			vect_res.val[1] = vect_res_2;
			vst2q_f16((float16_t*)&ImageOut->pSobelPixel[indice-ImageIn->numCols], vect_res);
		}
		//Tail vpred need to be clean
		if(numtail>0)
		{
			for(int j = 0; j<numtail+1; j++)
			{
				int y = (((ImageIn->numCols-2)>>3)<<3) + j;

				Img_tmp->pData[xm*Img_tmp->numCols +y].y = (ImageIn->pData[(x-1)*ImageIn->numCols+y] + (ImageIn->pData[x*ImageIn->numCols+y]<<1) + ImageIn->pData[(x+1)*ImageIn->numCols+y]);
				Img_tmp->pData[xm*Img_tmp->numCols +y].x = (ImageIn->pData[x*ImageIn->numCols+(y-1)] + (ImageIn->pData[x*ImageIn->numCols+(y)]<<1) + ImageIn->pData[x*ImageIn->numCols+(y+1)]);
				if(x==1)
				{
					continue;
				}
				int indice = (x-1)*ImageIn->numCols + y;
				int32_t gradx = Img_tmp->pData[((x-2)%3)*Img_tmp->numCols +y].x - Img_tmp->pData[(xm)*Img_tmp->numCols +y].x;
				int32_t grady = Img_tmp->pData[((x-1)%3)*Img_tmp->numCols +(y-1)].y - Img_tmp->pData[((x-1)%3)*Img_tmp->numCols +(y+1)].y;
				if(gradx==0&&grady==0)
				{
					ImageOut->pSobelPixel[indice].angle = Horizontal;
					ImageOut->pSobelPixel[indice].mag = 0;
					continue;
				}
				ImageOut->pSobelPixel[(x-1)*ImageIn->numCols+y].mag = sqrtf((gradx)*(gradx)+ (grady)*(grady));
				/*float tan*/ImageOut->pSobelPixel[(x-1)*ImageIn->numCols+y].angle = atan_simplify(/*gradx/abs(grady))*(180/PI_F64)*/gradx, grady);

				/*if(tan < (float)(22))
				{
					if(tan < (float)(-68))
					{
						ImageOut->pSobelPixel[(x-1)*ImageIn->numCols+y].angle = Vertical;
					}
					else if(tan <(float)(-20))
					{
						ImageOut->pSobelPixel[(x-1)*ImageIn->numCols+y].angle = Diagonal_lr;
					}
					else
					{
						ImageOut->pSobelPixel[(x-1)*ImageIn->numCols+y].angle = Horizontal;
					}
				}
				else if(tan <(float)(67))
				{
					ImageOut->pSobelPixel[(x-1)*ImageIn->numCols+y].angle = Diagonal_rl;
				}
				else
				{
					ImageOut->pSobelPixel[(x-1)*ImageIn->numCols+y].angle = Vertical;
				}*/
			}
		}
		y = size_y-1;
		Img_tmp->pData[xm*Img_tmp->numCols +y].y = ImageIn->pData[(x-1)*ImageIn->numCols+y] + (ImageIn->pData[x*ImageIn->numCols+y]<<1) + ImageIn->pData[(x+1)*ImageIn->numCols+y];
	}
	int y;
	x=0;
	for( int y =0; y < size_y; y++)
	{
		ImageOut->pSobelPixel[x*ImageIn->numCols+y].mag = 0;
	}
	x = size_x-1;
	for( int y =0; y < size_y; y++)
	{
		ImageOut->pSobelPixel[x*ImageIn->numCols+y].mag = 0;
	}
	y = size_y-1;
	for( int x =1; x < size_x; x++)
	{
		ImageOut->pSobelPixel[x*ImageIn->numCols+y].mag = 0;
	}
	y=0;
	for( int x =1; x < size_x; x++)
	{
		ImageOut->pSobelPixel[x*ImageIn->numCols+y].mag = 0;
	}
}
#else
void arm_sobel_in_u8_out_f16_proc_f16_buff(const arm_image_gray_u8_t* ImageIn, arm_image_sobel_f16_t* ImageOut, arm_buffer_2_u16_t* Img_tmp)
{
	int c = 0;
	int size_x = 240;
	int size_y = 320;
	int16_t gradx;
	//int16_t grady;
	ImageOut->numRows = ImageIn->numRows;
	int numtail = (ImageIn->numCols-1)%8;
	int x = 0;
	for(int y =1; y<((ImageIn->numCols)>>3); y++)
	{
		int indice = ((y-1)<<3)+1;
		uint16x8_t vect11 = vldrbq_u16(&ImageIn->pData[indice-1]);
		uint16x8_t vect2 = vldrbq_u16(&ImageIn->pData[indice]);
		vect2 = vshlq_n_u16(vect2, 1);
		uint16x8_t vect31 = vldrbq_u16(&ImageIn->pData[indice+1]);
		vect11 = vaddq_u16(vect11, vect2);
		vect11 = vaddq_u16(vect11, vect31);
		q15x8_t vect_void = vdupq_n_s16(0);
		uint16x8x2_t vect_buff_di1;
		vect_buff_di1.val[0] = vect11;
		vect_buff_di1.val[1] = vect_void;
		vst2q_u16(&Img_tmp->pData[indice].x, vect_buff_di1);
	}
	if(numtail>0)
	{
		int indice = (((ImageIn -> numCols-2)>>3) <<3);
		mve_pred16_t vtail = vctp16q(numtail);
		uint16x8_t vect1 = vldrbq_z_u16(&ImageIn->pData[indice-1], vtail);
		uint16x8_t vect2 =vldrbq_z_u16(&ImageIn->pData[indice], vtail);
		vect2 = vshlq_m_n_u16(vect2,vect2, 1, vtail);
		uint16x8_t vect3 =vldrbq_z_u16(&ImageIn->pData[indice+1], vtail);
		vect1 = vaddq_u16(vect1, vect2);//vtail or not?
		vect1 = vaddq_u16(vect1, vect3);//idem
		for(int j=0; j<numtail+1; j++)
		{
			Img_tmp->pData[indice + j].x = vect1[j];
			Img_tmp->pData[indice + j].y = 0;
		}
	}
	x = 1;
	Img_tmp->pData[x*Img_tmp->numCols].y = ImageIn->pData[(x-1)*ImageIn->numCols] + (ImageIn->pData[x*ImageIn->numCols]<<1) + ImageIn->pData[(x+1)*ImageIn->numCols];
	for(int y =1; y<((ImageIn->numCols)>>3); y++)
	{
		int indice = ImageIn->numCols +((y-1)<<3)+1;
		uint16x8_t vect11 = vldrbq_u16(&ImageIn->pData[indice-1]);
		uint16x8_t vect2 = vldrbq_u16(&ImageIn->pData[indice]);
		vect2 = vshlq_n_u16(vect2, 1);
		uint16x8_t vect31 = vldrbq_u16(&ImageIn->pData[indice+1]);
		vect11 = vaddq_u16(vect11, vect2);
		vect11 = vaddq_u16(vect11, vect31);
		uint16x8_t vecth1 = vldrbq_u16(&ImageIn->pData[indice-ImageIn->numCols]);
		uint16x8_t vectb1 = vldrbq_u16(&ImageIn->pData[indice+ImageIn->numCols]);
		vectb1 = vaddq_s16(vectb1, vect2);
		vectb1 = vaddq_s16(vectb1, vecth1);
		uint16x8x2_t vect_buff_di1;
		vect_buff_di1.val[0] = vect11;
		vect_buff_di1.val[1] = vectb1;
		vst2q_u16(&Img_tmp->pData[indice].x, vect_buff_di1);
	}
	if(numtail>0)
	{
		int indice = ImageIn->numCols + (((ImageIn -> numCols-2)>>3) <<3);
		mve_pred16_t vtail = vctp16q(numtail);
		uint16x8_t vect1 = vldrbq_z_u16(&ImageIn->pData[indice-1], vtail);
		uint16x8_t vect2 =vldrbq_z_u16(&ImageIn->pData[indice], vtail);
		vect2 = vshlq_m_n_u16(vect2,vect2, 1, vtail);
		uint16x8_t vect3 =vldrbq_z_u16(&ImageIn->pData[indice+1], vtail);
		vect1 = vaddq_u16(vect1, vect2);//vtail or not?
		vect1 = vaddq_u16(vect1, vect3);//idem
		uint16x8_t vecth = vldrbq_u16(&ImageIn->pData[indice-ImageIn->numCols]);
		uint16x8_t vectb = vldrbq_u16(&ImageIn->pData[indice+ImageIn->numCols]);
		vectb = vaddq_u16(vectb, vect2);
		vectb = vaddq_u16(vectb, vecth);
		for(int j=0; j<numtail+2; j++)
		{
			Img_tmp->pData[indice + j].x = vect1[j];
			Img_tmp->pData[indice + j].y = vectb[j];
		}
	}
	Img_tmp->pData[x*Img_tmp->numCols+size_y-1].y = ImageIn->pData[(x-1)*ImageIn->numCols+size_y-1] + (ImageIn->pData[x*ImageIn->numCols+size_y-1]<<1) + ImageIn->pData[(x+1)*ImageIn->numCols+size_y-1];
	for( int x =2; x<size_x; x++)
	{
		int xm = x%3;
		int y = 0;
		Img_tmp->pData[xm*Img_tmp->numCols +y].y = ImageIn->pData[(x-1)*ImageIn->numCols+y] + (ImageIn->pData[x*ImageIn->numCols+y]<<1) + ImageIn->pData[(x+1)*ImageIn->numCols+y];
		for( int y =1; y < size_y-numtail-1; y+=8)
		{
			/*
			if(gradx==0&&grady==0)
			{
				ImageOut->pSobelPixel[(x-1)*ImageIn->numCols+y].angle = Horizontal;
				ImageOut->pSobelPixel[(x-1)*ImageIn->numCols+y].mag = 0;
				continue;
			}*/
			int indice = x*ImageIn->numCols + y;
			int indicem = xm*ImageIn->numCols + y;
			uint16x8_t vect11 = vldrbq_u16(&ImageIn->pData[indice-1]);
			uint16x8_t vect2 = vldrbq_u16(&ImageIn->pData[indice]);
			vect2 = vshlq_n_u16(vect2, 1);
			uint16x8_t vect31 = vldrbq_u16(&ImageIn->pData[indice+1]);
			vect11 = vaddq_u16(vect11, vect2);
			vect11 = vaddq_u16(vect11, vect31);
			uint16x8_t vecth1 = vldrbq_u16(&ImageIn->pData[indice-ImageIn->numCols]);
			uint16x8_t vectb1 = vldrbq_u16(&ImageIn->pData[indice+ImageIn->numCols]);
			vectb1 = vaddq_s16(vectb1, vect2);
			vectb1 = vaddq_s16(vectb1, vecth1);
			uint16x8x2_t vect_buff_di1;
			vect_buff_di1.val[0] = vect11;
			vect_buff_di1.val[1] = vectb1;
			vst2q_u16(&Img_tmp->pData[indicem].x, vect_buff_di1);
			
			uint16x8x2_t vec_x_y_1 = vld2q_u16(&Img_tmp->pData[((x-2)%3)*ImageIn->numCols + y].x);
			uint16x8x2_t vec_x_2 = vld2q_u16(&Img_tmp->pData[indicem].x);
			uint16x8x2_t vec_y_1 = vld2q_u16(&Img_tmp->pData[((x-1)%3)*ImageIn->numCols + y-1].x);
			uint16x8x2_t vec_y_2 = vld2q_u16(&Img_tmp->pData[((x-1)%3)*ImageIn->numCols + y+1].x);
			int16x8_t vectgradx = vsubq_s16(vec_x_y_1.val[0], vec_x_2.val[0]);
			int16x8_t vectgrady = vsubq_s16(vec_y_1.val[1], vec_y_2.val[1]);
			//int numVect = 8;

			

			//float16x8_t vect_res_1;
			float16x8_t vect_res_2;
			uint16x8_t vect_gradx_1 = vqdmulhq(vectgradx, vectgradx);
			uint16x8_t vect_grady_1 = vqdmulhq(vectgrady, vectgrady);
			//shift 1 right before add
			vect_grady_1 = vrshrq( vect_grady_1 ,1);
			vect_gradx_1 = vrshrq( vect_gradx_1 ,1);
			vect_gradx_1 = vaddq_u16(vect_gradx_1, vect_grady_1);
			
			q15x8_t newtonStartVec = vdupq_n_s16(INVSQRT_MAGIC_F16) - vshrq( (q15x8_t)vect_gradx_1, 1);
        	float16x8_t sumHalf = vcvtq(vrshrq(vect_gradx_1, 1));
			float16x8_t invSqrt;
        	/*
        	 * compute 3 x iterations
        	 *
        	 * The more iterations, the more accuracy.
        	 * If you need to trade a bit of accuracy for more performance,
        	 * you can comment out the 3rd use of the macro.
        	 */
        	INVSQRT_NEWTON_MVE_F16(invSqrt, sumHalf, (float16x8_t) newtonStartVec);
			for(int j = 0; j<8; j++)
			{
				if((float)invSqrt[j] < (float)0.0)
				{
					if(c ==0)
					{
						c++;
						for(int i = 0; i<8; i++)
						{
							printf("%f ", (double)invSqrt[i]);
						}
						printf("\n");
						for(int i = 0; i<8; i++)
						{
							printf("%f ", (double)newtonStartVec[i]);
						}
						printf("HUDGE ISSUE\n");
					}
					
				}
			}
        	INVSQRT_NEWTON_MVE_F16(invSqrt, sumHalf, invSqrt);
			for(int j = 0; j<8; j++)
			{
				if(c ==1)
				{
					for(int i = 0; i<8; i++)
					{
						printf("%f ", (double)invSqrt[i]);
					}
					printf("\n");
					for(int i = 0; i<8; i++)
					{
						printf("%f ", (double)newtonStartVec[i]);
					}
					printf("HUDGE ISSUE\n");
				}
			}
        	INVSQRT_NEWTON_MVE_F16(invSqrt, sumHalf, invSqrt);
        	for(int j = 0; j<8; j++)
			{
				if(c ==1)
				{
					c++;
					for(int i = 0; i<8; i++)
					{
						printf("%f ", (double)invSqrt[i]);
					}
					printf("\n");
					for(int i = 0; i<8; i++)
					{
						printf("%f ", (double)newtonStartVec[i]);
					}
					printf("HUDGE ISSUE\n");
				}
			}
			/*
        	 * set negative values to 0
        	 */
        	//invSqrt = vdupq_m(invSqrt, (float16_t)0.0f, vcmpltq(invSqrt, (float16_t)0.0f));
        	/*
        	 * sqrt(x) = x * invSqrt(x)
        	 */
        	float16x8x2_t vect_res;
			vect_res.val[0] = vmulq(vcvtq(vect_gradx_1), invSqrt);
			//shift two left after square
			for(int i = 0; i< 8; i++)
			{
				//vect for grad
				gradx = vectgradx[i];
				//magnitude
				
				//VERY IMPORTANT PIECE OF CODE
				//allow you to avoid the case 0 0 in the tan causing huge perf loses
				/*if(vectgradx[i]==0&&vectgrady[i]==0)
				{
					vect_res_2[i] = Horizontal;
					vect_res_1[i] = 0;
					continue;
				}*/

				float tan = atan(vectgradx[i]/abs(vectgrady[i]))*(180/PI_F64);
				if(tan < (float)(22))
				{
					if(tan < (float)(-68))
					{
						vect_res_2[i] = Vertical;
					}
					else if(tan <(float)(-20))
					{
						ImageOut->pSobelPixel[(x-1)*ImageIn->numCols+y+i].angle = Diagonal_lr;
						vect_res_2[i] = Diagonal_lr;
					}
					else
					{
						vect_res_2[i] = Horizontal;
					}
				}
				else if(tan <(float)(67))
				{
					ImageOut->pSobelPixel[(x-1)*ImageIn->numCols+y+i].angle = Diagonal_rl;
					vect_res_2[i] = Diagonal_rl;
				}
				else
				{
					vect_res_2[i] = Vertical;
				}
			}
			/*float16x8x2_t vect_res;
			vect_res.val[0] = vect_res_1;*/
			vect_res.val[1] = vect_res_2;
			vst2q_f16(&ImageOut->pSobelPixel[indice-ImageIn->numCols].mag, vect_res);
		}
		//Tail vpred need to be clean
		if(numtail>0)
		{
			for(int j = 0; j<numtail+1; j++)
			{
				int y = (((ImageIn->numCols-2)>>3)<<3) + j;

				Img_tmp->pData[xm*Img_tmp->numCols +y].y = (ImageIn->pData[(x-1)*ImageIn->numCols+y] + (ImageIn->pData[x*ImageIn->numCols+y]<<1) + ImageIn->pData[(x+1)*ImageIn->numCols+y]);
				Img_tmp->pData[xm*Img_tmp->numCols +y].x = (ImageIn->pData[x*ImageIn->numCols+(y-1)] + (ImageIn->pData[x*ImageIn->numCols+(y)]<<1) + ImageIn->pData[x*ImageIn->numCols+(y+1)]);
				if(x==1)
				{
					continue;
				}
				int indice = (x-1)*ImageIn->numCols + y;
				int32_t gradx = Img_tmp->pData[((x-2)%3)*Img_tmp->numCols +y].x - Img_tmp->pData[(xm)*Img_tmp->numCols +y].x;
				int32_t grady = Img_tmp->pData[((x-1)%3)*Img_tmp->numCols +(y-1)].y - Img_tmp->pData[((x-1)%3)*Img_tmp->numCols +(y+1)].y;
				if(gradx==0&&grady==0)
				{
					ImageOut->pSobelPixel[indice].angle = Horizontal;
					ImageOut->pSobelPixel[indice].mag = 0;
					continue;
				}
				ImageOut->pSobelPixel[(x-1)*ImageIn->numCols+y].mag = sqrtf((gradx)*(gradx)+ (grady)*(grady));
				float tan = atan(gradx/abs(grady))*(180/PI_F64);
				if(tan < (float)(22))
				{
					if(tan < (float)(-68))
					{
						ImageOut->pSobelPixel[(x-1)*ImageIn->numCols+y].angle = Vertical;
					}
					else if(tan <(float)(-20))
					{
						ImageOut->pSobelPixel[(x-1)*ImageIn->numCols+y].angle = Diagonal_lr;
					}
					else
					{
						ImageOut->pSobelPixel[(x-1)*ImageIn->numCols+y].angle = Horizontal;
					}
				}
				else if(tan <(float)(67))
				{
					ImageOut->pSobelPixel[(x-1)*ImageIn->numCols+y].angle = Diagonal_rl;
				}
				else
				{
					ImageOut->pSobelPixel[(x-1)*ImageIn->numCols+y].angle = Vertical;
				}
			}
		}
		y = size_y-1;
		Img_tmp->pData[xm*Img_tmp->numCols +y].y = ImageIn->pData[(x-1)*ImageIn->numCols+y] + (ImageIn->pData[x*ImageIn->numCols+y]<<1) + ImageIn->pData[(x+1)*ImageIn->numCols+y];
	}
	int y;
	x=0;
	for( int y =0; y < size_y; y++)
	{
		ImageOut->pSobelPixel[x*ImageIn->numCols+y].mag = 0;
	}
	x = size_x-1;
	for( int y =0; y < size_y; y++)
	{
		ImageOut->pSobelPixel[x*ImageIn->numCols+y].mag = 0;
	}
	y = size_y-1;
	for( int x =1; x < size_x; x++)
	{
		ImageOut->pSobelPixel[x*ImageIn->numCols+y].mag = 0;
	}
	y=0;
	for( int x =1; x < size_x; x++)
	{
		ImageOut->pSobelPixel[x*ImageIn->numCols+y].mag = 0;
	}
}
#endif
#endif

