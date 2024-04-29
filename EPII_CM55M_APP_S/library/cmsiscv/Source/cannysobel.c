#include "arm_math.h"
#include "arm_type_cvl.h"
#include "canny_edge.h"


#if ((!defined(ARM_MATH_MVEI)) ||(defined(FORCE_SCALAR)))
//function performing canny edge on an image where a gaussian filter has been applied
//this function uses three buffers, one for storing intermediate values for computing the gradient, one for storing the gradient and one for storing the magnitude conputed with the gradient
//exept the buffer for the magnitude, the buffer have two component
//the purpose of this function is to avoid repetition of compute by storing the intermediate part of the compute and by fusing the end of canny edge and sobel
//to avoid repetition of condition for the end of the canny edge
void arm_canny_edge_sobel_in_q15_out_u8_proc_q15(const arm_image_gray_q15_t* ImageIn, 
                                                 arm_image_gray_q15_t* ImageOut, 
                                                 arm_buffer_2_q15_t* Img_tmp_grad/*4*/, 
                                                 arm_image_gray_q15_t* Img_tmp_mag/*3*/, 
                                                 arm_buffer_2_q15_t* Img_tmp_temporary/*3*/,
                                                 int low_threshold,
                                                 int high_threshold)
{
	//low_threshold = (int)(2528);
    //int high_threshold = (int)(5728);
	int indice;	
	q63_t gradx;
	q63_t grady;
	int x = 0;
	//Initialisation steps

	//ensure the buffers are empty
	for(int t = 0; t<Img_tmp_grad->numCols*3; t++)
	{
		Img_tmp_grad->pData[t].x=0;
		Img_tmp_mag->pData[t]=0;
		Img_tmp_temporary->pData[t].x=0;
	}
	//first initialisation of the temporary buffer, for the first line we cannot compute the composant on y, so we only do the composant on x 
	for(int y = 1; y<ImageIn -> numCols- 1; y++)
	{
		Img_tmp_temporary->pData[x*Img_tmp_temporary->numCols +y].x = (ImageIn->pData[x*ImageIn->numCols+(y-1)] + (ImageIn->pData[x*ImageIn->numCols+(y)]<<1) + ImageIn->pData[x*ImageIn->numCols+(y+1)])>>2;
	}
	//for the second line we compute both component of the temporary buffer
	x = 1;
	Img_tmp_temporary->pData[x*Img_tmp_temporary->numCols].y = (ImageIn->pData[(x-1)*ImageIn->numCols] + (ImageIn->pData[x*ImageIn->numCols]<<1) + ImageIn->pData[(x+1)*ImageIn->numCols])>>2;
	for(int y = 1; y<ImageIn -> numCols- 1; y++)
	{
		Img_tmp_temporary->pData[x*Img_tmp_temporary->numCols +y].y = (ImageIn->pData[(x-1)*ImageIn->numCols+y] + (ImageIn->pData[x*ImageIn->numCols+y]<<1) + ImageIn->pData[(x+1)*ImageIn->numCols+y])>>2;//possibility to >>2/3 to reduce buffer size to int8 //6value in the buffer aren't used the six on the dorder vertical to painful to adapt code for it
		Img_tmp_temporary->pData[x*Img_tmp_temporary->numCols +y].x = (ImageIn->pData[x*ImageIn->numCols+(y-1)] + (ImageIn->pData[x*ImageIn->numCols+(y)]<<1) + ImageIn->pData[x*ImageIn->numCols+(y+1)])>>2;
	}
	Img_tmp_temporary->pData[x*Img_tmp_temporary->numCols+Img_tmp_temporary->numCols-1].y = (ImageIn->pData[(x-1)*ImageIn->numCols+Img_tmp_temporary->numCols-1] + (ImageIn->pData[x*ImageIn->numCols+Img_tmp_temporary->numCols-1]<<1) + ImageIn->pData[(x+1)*ImageIn->numCols+Img_tmp_temporary->numCols-1])>>2;
	//third line, we compute a third line for the temporary buffer, so it is now full and we are now able to start the computation of the gradient buffer so we also compute teh first line of the gradient buffer
	//also if we have the gradient, we can compute the magnitude too
	x=2;
	for( int y =0; y < ImageIn->numCols; y++)
	{
		//Cmputation of the temporary intermediate computation 
		int xm = x%3;
		if((y==0||y == ImageIn->numCols-1)&& x != ImageIn->numRows-1)
		{
			Img_tmp_temporary->pData[xm*Img_tmp_temporary->numCols +y].y = (ImageIn->pData[(x-1)*ImageIn->numCols+y] + (ImageIn->pData[x*ImageIn->numCols+y]<<1) + ImageIn->pData[(x+1)*ImageIn->numCols+y])>>2;
			indice = x*ImageIn->numCols + y;
			ImageOut->pData[indice] = 0;
			continue;
		}
		Img_tmp_temporary->pData[xm*Img_tmp_temporary->numCols +y].y = (ImageIn->pData[(x-1)*ImageIn->numCols+y] + (ImageIn->pData[x*ImageIn->numCols+y]<<1) + ImageIn->pData[(x+1)*ImageIn->numCols+y])>>2;
		Img_tmp_temporary->pData[xm*Img_tmp_temporary->numCols +y].x = (ImageIn->pData[x*ImageIn->numCols+(y-1)] + (ImageIn->pData[x*ImageIn->numCols+(y)]<<1) + ImageIn->pData[x*ImageIn->numCols+(y+1)])>>2;

		//computation of the gradient
		indice = (x-1)*ImageIn->numCols + y;

		gradx = Img_tmp_temporary->pData[((x-2)%3)*Img_tmp_temporary->numCols +y].x - Img_tmp_temporary->pData[(xm)*Img_tmp_temporary->numCols +y].x;
		grady = Img_tmp_temporary->pData[((x-1)%3)*Img_tmp_temporary->numCols +(y-1)].y - Img_tmp_temporary->pData[((x-1)%3)*Img_tmp_temporary->numCols +(y+1)].y;
		if(gradx==0&&grady==0)
		{
			ImageOut->pData[indice] = 0;
			continue;
		}
		//Computation of the magnitude
		q15_t vect[2] = { (q15_t)gradx, (q15_t)grady};
		q15_t out;

		q31_t in[2] = {((q31_t)vect[0]), ((q31_t)vect[1])};
		q31_t out2[2];
		q31_t out3;
		q31_t root;
		//multiplication of two q15 give a q31 in output
		out2[0]=(in[0]*in[0]);
		out2[1]=(in[1]*in[1]);
		//addition of two q1.1.30 give a q1.2.29 shift by one a q1.1.30
		out3 = (out2[0]+out2[1])>>1;
		//root q3 give in output a q1.31 shit by 15, back to a q1.15 because of the previous shift by one 
		arm_sqrt_q31(out3, &root);
		out = root>>15;
		Img_tmp_grad->pData[(x-1)%3 * Img_tmp_temporary->numCols + y].y = grady;
		Img_tmp_grad->pData[(x-1)%3 * Img_tmp_temporary->numCols + y].x = gradx;
		Img_tmp_mag->pData[(x-1)%3 * Img_tmp_temporary->numCols + y]	= out;	
	}
	//here we continue our process but it's the main loop, 
	//as the previous loop we will compute a line of the temporary buffer and the associated line in gradient buffer and magnitude buffer
	//so we have two line of the gradient and magnitude buffer
	//for the computing of an output pixel we would need three line of those buffers but we chose to put all the border to 0 in our implementation of sobel, so the value are know, if the buffers are empty when passed to function
	//Here it's the case so we can start the computing of the output image
	for( int x = 3; x < ImageIn->numRows; x++)
    {
		//first loop on the line to compute the three buffers
        for( int y =0; y < ImageIn->numCols; y++)
        {
			//computation of the temporary buffer
			int xm = x%3;
			//ensure the border are initialized at 0
			if((y==0||y == ImageIn->numCols-1)&& x != ImageIn->numRows-1)
			{
				Img_tmp_temporary->pData[xm*Img_tmp_temporary->numCols +y].y = (ImageIn->pData[(x-1)*ImageIn->numCols+y] + (ImageIn->pData[x*ImageIn->numCols+y]<<1) + ImageIn->pData[(x+1)*ImageIn->numCols+y])>>2;
				indice = x*ImageIn->numCols + y;
				ImageOut->pData[indice] = 0;
				continue;
			}
			
			Img_tmp_temporary->pData[xm*Img_tmp_temporary->numCols +y].y = (ImageIn->pData[(x-1)*ImageIn->numCols+y] + (ImageIn->pData[x*ImageIn->numCols+y]<<1) + ImageIn->pData[(x+1)*ImageIn->numCols+y])>>2;
			Img_tmp_temporary->pData[xm*Img_tmp_temporary->numCols +y].x = (ImageIn->pData[x*ImageIn->numCols+(y-1)] + (ImageIn->pData[x*ImageIn->numCols+(y)]<<1) + ImageIn->pData[x*ImageIn->numCols+(y+1)])>>2;
			//computation of the gradient buffer
			indice = (x-1)*ImageIn->numCols + y;
			gradx = Img_tmp_temporary->pData[((x-2)%3)*Img_tmp_temporary->numCols +y].x - Img_tmp_temporary->pData[(xm)*Img_tmp_temporary->numCols +y].x;
			grady = Img_tmp_temporary->pData[((x-1)%3)*Img_tmp_temporary->numCols +(y-1)].y - Img_tmp_temporary->pData[((x-1)%3)*Img_tmp_temporary->numCols +(y+1)].y;
			//computation of the magnitude buffer
			if(gradx==0&&grady==0)
			{
				ImageOut->pData[indice] = 0;
				Img_tmp_mag->pData[(x-1)%3 * Img_tmp_temporary->numCols + y]	= 0;
				Img_tmp_grad->pData[(x-1)%3 * Img_tmp_temporary->numCols + y].y = grady;
				Img_tmp_grad->pData[(x-1)%3 * Img_tmp_temporary->numCols + y].x = gradx;
				continue;
			}
			q15_t vect[2] = { (q15_t)gradx, (q15_t)grady};
			q15_t out;

      		q31_t in[2] = {((q31_t)vect[0]), ((q31_t)vect[1])};
      		q31_t out2[2];
      		q31_t out3;
      		q31_t root;
      		out2[0]=(in[0]*in[0]);
      		out2[1]=(in[1]*in[1]);
      		out3 = (out2[0]+out2[1])>>1;
      		arm_sqrt_q31(out3, &root);
      		out = root>>15;
			//setting all the data in the buffers, if not done properly, errors will occurs
			Img_tmp_grad->pData[(x-1)%3 * Img_tmp_temporary->numCols + y].y = grady;
			Img_tmp_grad->pData[(x-1)%3 * Img_tmp_temporary->numCols + y].x = gradx;
			Img_tmp_mag->pData[(x-1)%3 * Img_tmp_temporary->numCols + y]	= out>>5<<5;	
		}
		//second loop on the line to compute the output data
		for( int y =1; y < ImageIn->numCols-1; y++)
		{
			int indice = (x-2)*ImageIn->numCols +y;
			int mag = Img_tmp_mag->pData[((x-2)%3) * (ImageIn->numCols)+y];
			//First thing to test is the magnitude, if it's under the lowthreshold, then there is no need to test further, the output value will be 0
			if(mag < low_threshold)
			{
				ImageOut->pData[indice] = 0;
				continue;
			}
			//we need to compare the magnitude with two other one, determine by the angle
			//if the magnitude is higher than those two, we check if the magnitude is higer the the high threshold 
			//if yes, the output value is 255
			//if no, we need to check in the 3x3 square of magnitude if ther is at least one that is higher than the value
			//if yes 255, else 0
			//This is the step where checking the angle is unavoidable and where we used the gradient buffer, in scalar because of the atan
			else
			{
				q15_t angle;
				arm_atan2_q15(Img_tmp_grad->pData[((x-2)%3)*ImageIn->numCols+y].x, Img_tmp_grad->pData[((x-2)%3)*ImageIn->numCols+y].y, &angle);
				arm_abs_q15( &angle, &angle, 1);
				//decision based on the angle
				if((angle ) < (0xC4A))//22 in rad in 2.13
				{
					if( mag <= Img_tmp_mag->pData[((x-2)%3)*ImageIn->numCols+y-1] || mag <= Img_tmp_mag->pData[((x-2)%3)*ImageIn->numCols+y+1])
					{
						ImageOut->pData[indice] = 0;
						continue;
					}
					else
					{
						if(mag < high_threshold)
						{
							if(Img_tmp_mag->pData[((x-3)%3) * (ImageIn->numCols)+y-1]>=(int)(high_threshold) ||Img_tmp_mag->pData[((x-3)%3) * (ImageIn->numCols)+y]>=(int)(high_threshold) ||Img_tmp_mag->pData[((x-3)%3) * (ImageIn->numCols)+y+1]>=(int)(high_threshold)||Img_tmp_mag->pData[((x-1)%3) * (ImageIn->numCols)+y+1]>=(int)(high_threshold) ||Img_tmp_mag->pData[((x-1)%3) * (ImageIn->numCols)+y]>=(int)(high_threshold) ||Img_tmp_mag->pData[((x-1)%3) * (ImageIn->numCols)+y-1]>=(int)(high_threshold))
							{
								ImageOut->pData[indice] = 0x7FFF;
								continue;
							}
							else
							{
								ImageOut->pData[indice] = 0;
								continue;
							}
						}
						else
						{
							ImageOut->pData[indice] = 0x7FFF;
							continue;
						}
					}
					continue;
				}
				else if((angle ) < (0x256C))//67 in rad in 2.13
				{
					/*if((Img_tmp_grad->pData[((x-2)%3)*ImageIn->numCols+y].x&0x8000)>>15)
					{
						if( mag <= Img_tmp_mag->pData[((x-3)%3) * (ImageIn->numCols)+y-1] || mag <= Img_tmp_mag->pData[((x-1)%3) * (ImageIn->numCols)+y+1])
						{
							ImageOut->pData[indice] = 0;
							continue;
						}
						else
						{
							if(mag < high_threshold)
							{
								if (Img_tmp_mag->pData[((x-3)%3) * (ImageIn->numCols)+y]>=(int)(high_threshold) ||Img_tmp_mag->pData[((x-3)%3) * (ImageIn->numCols)+y+1]>=(int)(high_threshold) ||Img_tmp_mag->pData[((x-2)%3) * (ImageIn->numCols)+y-1]>=(int)(high_threshold) ||Img_tmp_mag->pData[((x-2)%3) * (ImageIn->numCols)+y+1]>=(int)(high_threshold) ||Img_tmp_mag->pData[((x-1)%3) * (ImageIn->numCols)+y]>=(int)(high_threshold) ||Img_tmp_mag->pData[((x-1)%3) * (ImageIn->numCols)+y-1]>=(int)(high_threshold))
								{
									ImageOut->pData[indice] = 255;
									continue;
								}
								else
								{
									ImageOut->pData[indice] = 0;
									continue;
								}
							}
							else
							{
								ImageOut->pData[indice] = 255;
								continue;
							}
						}
						continue;
					}
					else
					{*/
						if( mag <= Img_tmp_mag->pData[((x-1)%3) * (ImageIn->numCols)+y-1] || mag <= Img_tmp_mag->pData[((x-3)%3) * (ImageIn->numCols)+y+1])
						{
							ImageOut->pData[indice] = 0;
							continue;
						}
						else
						{
							if( mag < low_threshold)
							{
								ImageOut->pData[indice] = 0;
								continue;
							}
							else if(mag < high_threshold)
							{
								if (Img_tmp_mag->pData[((x-3)%3) * (ImageIn->numCols)+y-1]>=(int)(high_threshold) ||Img_tmp_mag->pData[((x-3)%3) * (ImageIn->numCols)+y]>=(int)(high_threshold)||Img_tmp_mag->pData[((x-2)%3) * (ImageIn->numCols)+y-1]>=(int)(high_threshold) ||Img_tmp_mag->pData[((x-2)%3) * (ImageIn->numCols)+y+1]>=(int)(high_threshold)||Img_tmp_mag->pData[((x-1)%3) * (ImageIn->numCols)+y]>=(int)(high_threshold) ||Img_tmp_mag->pData[((x-1)%3) * (ImageIn->numCols)+y-1]>=(int)(high_threshold))
								{
									ImageOut->pData[indice] = 0x7FFF;
									continue;
								}
								else
								{
									ImageOut->pData[indice] = 0;
									continue;
								}
							}
							else
							{
								ImageOut->pData[indice] = 0x7FFF;
								continue;
							}
						}
						continue;
					//}	
				}
				else if((angle ) < (0x3E8E))//112 in rad in 2.13
				{
					if(  0 <= Img_tmp_mag->pData[((x-3)%3) * (ImageIn->numCols)+y] - mag||  0 <= Img_tmp_mag->pData[((x-1)%3) * (ImageIn->numCols)+y] - mag)
					{
						ImageOut->pData[indice] = 0;
						continue;
					}
					else
					{
						if(mag < high_threshold)
						{
							if (Img_tmp_mag->pData[((x-3)%3) * (ImageIn->numCols)+y-1]>=(int)(high_threshold)||Img_tmp_mag->pData[((x-3)%3) * (ImageIn->numCols)+y+1]>=(int)(high_threshold) ||Img_tmp_mag->pData[((x-2)%3) * (ImageIn->numCols)+y-1]>=(int)(high_threshold) ||Img_tmp_mag->pData[((x-2)%3) * (ImageIn->numCols)+y+1]>=(int)(high_threshold) ||Img_tmp_mag->pData[((x-1)%3) * (ImageIn->numCols)+y+1]>=(int)(high_threshold)||Img_tmp_mag->pData[((x-1)%3) * (ImageIn->numCols)+y-1]>=(int)(high_threshold))
							{
								ImageOut->pData[indice] = 0x7FFF;
								continue;
							}
							else
							{
								ImageOut->pData[indice] = 0;
								continue;
							}
						}
						else
						{
							ImageOut->pData[indice] = 0x7FFF;
							continue;
						}
					}
					continue;
				}
				else if((angle ) < (0x595C))//160 in rad in 2.13
				{
					/*if((Img_tmp_grad->pData[((x-2)%3)*ImageIn->numCols+y].x&0x8000)>>15)
					{
						if( mag <= Img_tmp_mag->pData[((x-1)%3) * (ImageIn->numCols)+y-1] || mag <= Img_tmp_mag->pData[((x-3)%3) * (ImageIn->numCols)+y+1])
						{
							ImageOut->pData[indice] = 0;
							continue;
						}
						else
						{
							if(mag < high_threshold)
							{
								if (Img_tmp_mag->pData[((x-3)%3) * (ImageIn->numCols)+y-1]>=(int)(high_threshold) ||Img_tmp_mag->pData[((x-3)%3) * (ImageIn->numCols)+y]>=(int)(high_threshold)||Img_tmp_mag->pData[((x-2)%3) * (ImageIn->numCols)+y-1]>=(int)(high_threshold) ||Img_tmp_mag->pData[((x-2)%3) * (ImageIn->numCols)+y+1]>=(int)(high_threshold)||Img_tmp_mag->pData[((x-1)%3) * (ImageIn->numCols)+y]>=(int)(high_threshold) ||Img_tmp_mag->pData[((x-1)%3) * (ImageIn->numCols)+y-1]>=(int)(high_threshold))
								{
									ImageOut->pData[indice] = 255;
									continue;
								}
								else
								{
									ImageOut->pData[indice] = 0;
									continue;
								}
							}
							else
							{
								ImageOut->pData[indice] = 255;
								continue;
							}
						}
						continue;
					}
					else
					{*/
						if( mag <= Img_tmp_mag->pData[((x-3)%3) * (ImageIn->numCols)+y-1] || mag <= Img_tmp_mag->pData[((x-1)%3) * (ImageIn->numCols)+y+1])
						{
							ImageOut->pData[indice] = 0;
							continue;
						}
						else
						{
							if(mag < high_threshold)
							{
								if (Img_tmp_mag->pData[((x-3)%3) * (ImageIn->numCols)+y]>=(int)(high_threshold) ||Img_tmp_mag->pData[((x-3)%3) * (ImageIn->numCols)+y+1]>=(int)(high_threshold) ||Img_tmp_mag->pData[((x-2)%3) * (ImageIn->numCols)+y-1]>=(int)(high_threshold) ||Img_tmp_mag->pData[((x-2)%3) * (ImageIn->numCols)+y+1]>=(int)(high_threshold) ||Img_tmp_mag->pData[((x-1)%3) * (ImageIn->numCols)+y]>=(int)(high_threshold) ||Img_tmp_mag->pData[((x-1)%3) * (ImageIn->numCols)+y-1]>=(int)(high_threshold))
								{
									ImageOut->pData[indice] = 0x7FFF;
									continue;
								}
								else
								{
									ImageOut->pData[indice] = 0;
									continue;
								}
							}
							else
							{
								ImageOut->pData[indice] = 0x7FFF;
								continue;
							}
						}
						continue;
					//} 	
				}
				else//case tan between 135+22.5 and 180
				{
					if( mag <= Img_tmp_mag->pData[((x-2)%3) * (ImageIn->numCols)+y-1] || mag <= Img_tmp_mag->pData[((x-2)%3) * (ImageIn->numCols)+y+1])
					{
						ImageOut->pData[indice] = 0;
						continue;
					}
					else
					{
						if(mag < high_threshold)
						{
							if(Img_tmp_mag->pData[((x-3)%3) * (ImageIn->numCols)+y-1]>=(int)(high_threshold) ||Img_tmp_mag->pData[((x-3)%3) * (ImageIn->numCols)+y]>=(int)(high_threshold) ||Img_tmp_mag->pData[((x-3)%3) * (ImageIn->numCols)+y+1]>=(int)(high_threshold)||Img_tmp_mag->pData[((x-1)%3) * (ImageIn->numCols)+y+1]>=(int)(high_threshold) ||Img_tmp_mag->pData[((x-1)%3) * (ImageIn->numCols)+y]>=(int)(high_threshold) ||Img_tmp_mag->pData[((x-1)%3) * (ImageIn->numCols)+y-1]>=(int)(high_threshold))
							{
								ImageOut->pData[indice] = 0x7FFF;
								continue;
							}
							else
							{
								ImageOut->pData[indice] = 0;
								continue;
							}
						}
						else
						{
							ImageOut->pData[indice] = 0x7FFF;
							continue;
						}
					}
					continue;
				}
			}
		}
	}
	//last line of the outup image to be computed the conditions are simpler because we know that the last line of the image is 0, such as the gradient and magnitude for this line
	#if 0
	int x = 240;
	for( int y =1; y < ImageIn->numCols-1; y++)
	{
		
		int indice = (x-2)*ImageIn->numCols +y;
		int mag = Img_tmp_mag->pData[((x-2)%3) * (ImageIn->numCols)+y];
		//First thing to test is the magnitude, if it's under the lowthreshold, then there is no need to test further, out will be 0
		if(mag < low_threshold)
		{
			ImageOut->pData[indice] = 0;
			continue;
		}
		//we need to compare the magnitude with two other one, determine by the angle
		//if the magnitude is higher than those two, we check if the magnitude is higer the the high threshold 
		//if yes, the output value is 255
		//if no, we need to check in the 3x3 square of magnitude if ther is at least one that is higher than the value
		//if yes 255, else 0
		//This is the step where checking the angle is unavoidable and where we used the gradient buffer, in scalar because of the atan
		else
		{
			q15_t angle;
			arm_atan2_q15(Img_tmp_grad->pData[((x-2)%3)*ImageIn->numCols+y].x, Img_tmp_grad->pData[((x-2)%3)*ImageIn->numCols+y].y, &angle);
			if(Img_tmp_grad->pData[((x-2)%3)*ImageIn->numCols+y].x ==0&& Img_tmp_grad->pData[((x-2)%3)*ImageIn->numCols+y].y==0)
			{
				angle = 0;
			}
			arm_abs_q15( &angle, &angle, 1);
			if((angle ) < (0xC4A))//22 in rad in 2.13
			{
				if( mag <= Img_tmp_mag->pData[((x-2)%3)*ImageIn->numCols+y-1] || mag <= Img_tmp_mag->pData[((x-2)%3)*ImageIn->numCols+y+1])
				{
					ImageOut->pData[indice] = 0;
					continue;
				}
				else
				{
					if(mag < high_threshold)
					{
						if(Img_tmp_mag->pData[((x-1)%3) * (ImageIn->numCols)+y+1]>=(int)(high_threshold) ||Img_tmp_mag->pData[((x-1)%3) * (ImageIn->numCols)+y]>=(int)(high_threshold) ||Img_tmp_mag->pData[((x-1)%3) * (ImageIn->numCols)+y-1]>=(int)(high_threshold))
						{
							ImageOut->pData[indice] = 0x7FFF;
							continue;
						}
						else
						{
							ImageOut->pData[indice] = 0;
							continue;
						}
					}
					else
					{
						ImageOut->pData[indice] = 0x7FFF;
						continue;
					}
				}
				continue;
			}
			else if((angle ) < (0x256C))//67 in rad in 2.13
			{
				/*if((Img_tmp_grad->pData[((x-2)%3)*ImageIn->numCols+y].x&0x8000)>>15)
				{
					if( mag <= Img_tmp_mag->pData[((x-1)%3) * (ImageIn->numCols)+y+1])
					{
						ImageOut->pData[indice] = 0;
						continue;
					}
					else
					{
						if(mag < high_threshold)
						{
							if (Img_tmp_mag->pData[((x-2)%3) * (ImageIn->numCols)+y-1]>=(int)(high_threshold) ||Img_tmp_mag->pData[((x-2)%3) * (ImageIn->numCols)+y+1]>=(int)(high_threshold) ||Img_tmp_mag->pData[((x-1)%3) * (ImageIn->numCols)+y]>=(int)(high_threshold) ||Img_tmp_mag->pData[((x-1)%3) * (ImageIn->numCols)+y-1]>=(int)(high_threshold))
							{
								ImageOut->pData[indice] = 255;
								continue;
							}
							else
							{
								ImageOut->pData[indice] = 0;
								continue;
							}
						}
						else
						{
							ImageOut->pData[indice] = 255;
							continue;
						}
					}
					continue;
				}
				else
				{*/
					if( mag <= Img_tmp_mag->pData[((x-1)%3) * (ImageIn->numCols)+y-1] )
					{
						ImageOut->pData[indice] = 0;
						continue;
					}
					else
					{
						if(mag < high_threshold)
						{
							if (Img_tmp_mag->pData[((x-2)%3) * (ImageIn->numCols)+y-1]>=(int)(high_threshold) ||Img_tmp_mag->pData[((x-2)%3) * (ImageIn->numCols)+y+1]>=(int)(high_threshold)||Img_tmp_mag->pData[((x-1)%3) * (ImageIn->numCols)+y]>=(int)(high_threshold) ||Img_tmp_mag->pData[((x-1)%3) * (ImageIn->numCols)+y-1]>=(int)(high_threshold))
							{
								ImageOut->pData[indice] = 0x7FFF;
								continue;
							}
							else
							{
								ImageOut->pData[indice] = 0;
								continue;
							}
						}
						else
						{
							ImageOut->pData[indice] = 0x7FFF;
							continue;
						}
					}
					continue;
				//}	
			}
			else if((angle ) < (0x3E8E))//112 in rad in 2.13
			{
				if(  0 <= Img_tmp_mag->pData[((x-1)%3) * (ImageIn->numCols)+y] - mag)
				{
					ImageOut->pData[indice] = 0;
					continue;
				}
				else
				{
					if(mag < high_threshold)
					{
						if (Img_tmp_mag->pData[((x-2)%3) * (ImageIn->numCols)+y-1]>=(int)(high_threshold) ||Img_tmp_mag->pData[((x-2)%3) * (ImageIn->numCols)+y+1]>=(int)(high_threshold) ||Img_tmp_mag->pData[((x-1)%3) * (ImageIn->numCols)+y+1]>=(int)(high_threshold)||Img_tmp_mag->pData[((x-1)%3) * (ImageIn->numCols)+y-1]>=(int)(high_threshold))
						{
							ImageOut->pData[indice] = 0x7FFF;
							continue;
						}
						else
						{
							ImageOut->pData[indice] = 0;
							continue;
						}
					}
					else
					{
						ImageOut->pData[indice] = 0x7FFF;
						continue;
					}
				}
				continue;
			}
			else if((angle ) < (0x595C))//160 in rad in 2.13
			{
				/*if((Img_tmp_grad->pData[((x-2)%3)*ImageIn->numCols+y].x&0x8000)>>15)
				{
					if( mag <= Img_tmp_mag->pData[((x-1)%3) * (ImageIn->numCols)+y-1] )
					{
						ImageOut->pData[indice] = 0;
						continue;
					}
					else
					{
						if(mag < high_threshold)
						{
							if (Img_tmp_mag->pData[((x-2)%3) * (ImageIn->numCols)+y-1]>=(int)(high_threshold) ||Img_tmp_mag->pData[((x-2)%3) * (ImageIn->numCols)+y+1]>=(int)(high_threshold)||Img_tmp_mag->pData[((x-1)%3) * (ImageIn->numCols)+y]>=(int)(high_threshold) ||Img_tmp_mag->pData[((x-1)%3) * (ImageIn->numCols)+y-1]>=(int)(high_threshold))
							{
								ImageOut->pData[indice] = 255;
								continue;
							}
							else
							{
								ImageOut->pData[indice] = 0;
								continue;
							}
						}
						else
						{
							ImageOut->pData[indice] = 255;
							continue;
						}
					}
					continue;
				}
				else
				{*/
					if( mag <= Img_tmp_mag->pData[((x-1)%3) * (ImageIn->numCols)+y+1])
					{
						ImageOut->pData[indice] = 0;
						continue;
					}
					else
					{
						if(mag < high_threshold)
						{
							if (Img_tmp_mag->pData[((x-2)%3) * (ImageIn->numCols)+y-1]>=(int)(high_threshold) ||Img_tmp_mag->pData[((x-2)%3) * (ImageIn->numCols)+y+1]>=(int)(high_threshold) ||Img_tmp_mag->pData[((x-1)%3) * (ImageIn->numCols)+y]>=(int)(high_threshold) ||Img_tmp_mag->pData[((x-1)%3) * (ImageIn->numCols)+y-1]>=(int)(high_threshold))
							{
								ImageOut->pData[indice] = 0x7FFF;
								continue;
							}
							else
							{
								ImageOut->pData[indice] = 0;
								continue;
							}
						}
						else
						{
							ImageOut->pData[indice] = 0x7FFF;
							continue;
						}
					}
					continue;
				//} 	
			}
			else//case tan between 135+22.5 and 180
			{
				if( mag <= Img_tmp_mag->pData[((x-2)%3) * (ImageIn->numCols)+y-1] || mag <= Img_tmp_mag->pData[((x-2)%3) * (ImageIn->numCols)+y+1])
				{
					ImageOut->pData[indice] = 0;
					continue;
				}
				else
				{
					if(mag < high_threshold)
					{
						if(Img_tmp_mag->pData[((x-1)%3) * (ImageIn->numCols)+y+1]>=(int)(high_threshold) ||Img_tmp_mag->pData[((x-1)%3) * (ImageIn->numCols)+y]>=(int)(high_threshold) ||Img_tmp_mag->pData[((x-1)%3) * (ImageIn->numCols)+y-1]>=(int)(high_threshold))
						{
							ImageOut->pData[indice] = 0x7FFF;
							continue;
						}
						else
						{
							ImageOut->pData[indice] = 0;
							continue;
						}
					}
					else
					{
						ImageOut->pData[indice] = 0x7FFF;
						continue;
					}
				}
				continue;
			}
		}
	}
	#endif
}

#else

void arm_canny_edge_sobel_in_q15_out_u8_proc_q15(const arm_image_gray_q15_t* ImageIn, arm_image_gray_q15_t* ImageOut, arm_buffer_2_q15_t* Img_tmp_grad/*4*/, arm_image_gray_q15_t* Img_tmp_mag/*3*/, arm_buffer_2_q15_t* Img_tmp_temporary/*3*/)
{
	q15x8_t vect_mag;
    int x = 0;
	int w = ImageIn->numCols;
	int low_threshold = (int)(2528);
    int high_threshold = (int)(5728);
	//Initialize all the three buffer to 0
	//maybe useless maybe not, security measure
	for(int t = 0; t<320*3; t++)
	{
		Img_tmp_grad->pData[t].x=0;
		Img_tmp_mag->pData[t]=0;
		Img_tmp_temporary->pData[t].x=0;
	}
	//Computation of the first line of the temporary buffer contaning the partial sum for the gradient computation
	//because we are on the first line we only do the horizontal one and not the vertical one
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
		vst2q_s16((int16_t*)&Img_tmp_temporary->pData[indice], vect_buff_di);
	}
	//tail of the first line for partial sum for the gradient
	int numtail = (w-1)%8;
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
		vect1 = vaddq_s16(vect1, vect2);//vtail or not?
		vect1 = vaddq_s16(vect1, vect3);//idem
		for(int j=0; j<numtail; j++)
		{
			Img_tmp_temporary->pData[indice+j].x = vect1[j];
            Img_tmp_temporary->pData[indice+j].y = 0;
		}
	}
	//Computation of the second line of the partial sum for the gradient computation
	//this time we can do the two part vetical and horizontal
	x =1;
	Img_tmp_temporary->pData[x*w].x =0;
	Img_tmp_temporary->pData[x*w].y = (ImageIn->pData[(x-1)*w] + (ImageIn->pData[x*w]<<1) + ImageIn->pData[(x+1)*w])>>2;
	for(int y =1; y<((w)>>3); y++)
	{
		int indice = w +((y-1)<<3)+1;
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
		
		vst2q_s16(&Img_tmp_temporary->pData[indice].x, vect_buff_di);
	}
	//Tail of the second line of the partial sum for the gradient computation
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
			Img_tmp_temporary->pData[indice + j].x = vect1[j];
			Img_tmp_temporary->pData[indice + j].y = vectb[j];
		}
	}
	Img_tmp_temporary->pData[w+w-1].x =0;
	Img_tmp_temporary->pData[w+w-1].y = (ImageIn->pData[(x-1)*w+w-1] + (ImageIn->pData[x*w+w-1]<<1) + ImageIn->pData[(x+1)*w+w-1])>>2;
	//Last step of the initialisation, we compute thee thrid line of the partial sum and a first line of the gradient value but also the magnitude value
	for(int x =2; x< 3; x++)
	{
		int x3 = x%3;
		//Computation of the partial sum and the gradient at the same time, the gradient have a different offset becase it requieres to have three line of memory for the partial sums
		Img_tmp_temporary->pData[x3*w].y = (ImageIn->pData[(x-1)*w] + (ImageIn->pData[x*w]<<1) + ImageIn->pData[(x+1)*w])>>2;
		for(int y =1; y< w-7;y+=8)
		{
			int indice = x*w + y;
			int indice3 = x3*w + y;
			q15x8_t vect1 = vldrhq_s16(&ImageIn->pData[indice-1]);
			
			q15x8_t vect2 = vldrhq_s16(&ImageIn->pData[indice]);
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
			q15x8x2_t vec_x_y_1 = vld2q_s16(&Img_tmp_temporary->pData[((x-2)%3)*w + y].x);
			vst2q_s16(&Img_tmp_temporary->pData[indice3].x, vect_buff_di);			
			q15x8x2_t vec_y_1 = vld2q_s16(&Img_tmp_temporary->pData[((x-1)%3)*w + y-1].x);
			
			q15x8x2_t vect_grad_1;
			q15x8_t vectgradx = vsubq_s16(vec_x_y_1.val[0], vect_buff_di.val[0]);
			q15x8x2_t vec_y_2 = vld2q_s16(&Img_tmp_temporary->pData[((x-1)%3)*w + y+1].x);
			int numVect = 8;
			int16x8_t vect_res_1;
			q31x4_t vect_gradx_1 = vmullbq_int_s16(vectgradx, vectgradx);
			q15x8_t vectgrady = vsubq_s16(vec_y_1.val[1], vec_y_2.val[1]);
			q31x4_t vect_grady_1 = vmullbq_int_s16(vectgrady, vectgrady);
			
			vect_gradx_1 = vaddq_s32(vect_gradx_1, vect_grady_1);
			q31x4_t vect_gradx_2 = vmulltq_int_s16(vectgradx, vectgradx);
			vect_gradx_1 = vshrq(vect_gradx_1, 1);
			q31x4_t vect_grady_2 = vmulltq_int_s16(vectgrady, vectgrady);
			
			vect_gradx_2 = vaddq_s32(vect_gradx_2, vect_grady_2);
			vect_gradx_2 = vshrq(vect_gradx_2, 1);
			//
			
			vect_grad_1.val[0] = vectgradx;
			vect_grad_1.val[1] = vectgrady;
            vst2q(&Img_tmp_grad->pData[(x-1)%3*w+y].x, vect_grad_1);
			//Computation of the magnitude, due to the square root, we can't keep using vector
            //split into two loops because we tryed to keep the vector as long as possible so we only have the square root in scalar but 
			//the vector have been split in two with a desinterlive
			for(int j =0; j < numVect; j+=2)
			{
				if(vectgradx[j]==0&&vectgrady[j]==0)
				{
					vect_res_1[j] =  0;
					continue;
				}
				//mag
				q15_t out;
				q31_t root ;
				arm_sqrt_q31(vect_gradx_1[j>>1], &root);
      			out = root>>15;
				vect_res_1[j] = out;
			}
			for(int j =1; j < numVect; j+=2)
			{
				if(vectgradx[j]==0&&vectgrady[j]==0)
				{
					vect_res_1[j] =  0;
					continue;
				}
				//mag
				q15_t out;
				q31_t root ;
				arm_sqrt_q31(vect_gradx_2[j>>1], &root);
      			out = root>>15;
				vect_res_1[j] = out;
			}
			vst1q_s16((int16_t*)&Img_tmp_mag->pData[((x-1)%3)*w + y], vect_res_1);
		}
		//Tail of the third line of partial sum and the first gradient and magnitude May be doable in vector but for now all is in scalar
		for(int j = 0; j<8; j++)
		{
			int y = ((w-2)&0xFFF8) + j;
			if((y==0||y == w-1)&&x!=0&& x != ImageIn->numRows-1)
			{
				Img_tmp_grad->pData[(x-1)%3*w +y].y = (Img_tmp_temporary->pData[(x-1)*w+y].y + (Img_tmp_temporary->pData[x*w+y].y<<1) + Img_tmp_temporary->pData[(x+1)*w+y].y)>>2;
				int indice = x*w + y;
				Img_tmp_mag->pData[indice-w] = 0;
				continue;
			}
			if( x==0 || y==0||y == w-1)
			{
				int indice = (x%3)*w + y;
				Img_tmp_mag->pData[indice-w] = 0;
				Img_tmp_grad->pData[indice-w].x = 0;
				Img_tmp_grad->pData[indice-w].y = 0;
				continue;
			}
			Img_tmp_temporary->pData[(x)%3*w +y].y = ((ImageIn->pData[(x-1)*w+y]) + (ImageIn->pData[x*w+y]<<1) + (ImageIn->pData[(x+1)*w+y]))>>2;
			Img_tmp_temporary->pData[(x)%3*w +y].x = ((ImageIn->pData[x*w+(y-1)]) + (ImageIn->pData[x*w+(y)]<<1) + (ImageIn->pData[x*w+(y+1)]))>>2;
			if(x==1)
			{
				continue;
			}
			int indice = (x-1)*w + y;
			q63_t gradx = Img_tmp_temporary->pData[((x-2)%3)*w +y].x - Img_tmp_temporary->pData[(x3)*w +y].x;
			q63_t grady = Img_tmp_temporary->pData[(x-1)%3*w +(y-1)].y - Img_tmp_temporary->pData[(x-1)%3*w +(y+1)].y;
			Img_tmp_grad->pData[(x-1)%3*w+y].x = gradx;
			Img_tmp_grad->pData[(x-1)%3*w+y].y = grady;
			if(gradx==0&&grady==0)
			{
				Img_tmp_mag->pData[indice-w] = 0;
				continue;
			}
			//mag
			q15_t vect[2] = { (q15_t)gradx, (q15_t)grady};

			q31_t in[2] = {((q31_t)vect[0]), ((q31_t)vect[1])};
			q31_t out2[2];
			q31_t out3;
			q31_t root;
			out2[0]=(in[0]*in[0]);
			out2[1]=(in[1]*in[1]);
			out3 = (out2[0]+out2[1])>>1;
			arm_sqrt_q31(out3, &root);
			Img_tmp_mag->pData[((x-1)%3)*w + y] = root>>15;
		}
		Img_tmp_temporary->pData[(x)%3*w+w-1].y = (ImageIn->pData[(x-1)*w+w-1] + (ImageIn->pData[x*w+w-1]<<1) + ImageIn->pData[(x+1)*w+w-1])>>2;
	}  
	(void)low_threshold;
	(void)high_threshold;
	//Main Loop of the process on all the possible lines
	//This loop do, the update of the oldest line of the partial sum buffer, the computation of a line of gradient and magnitude(I think ther's an issue and I would have to do an other turn of the previous loop because i lack one line Maybe not because the border are forced to be at  I think not)
	//and a computation of a line of the output image using the manitude and the gradient store
	//this last computation used the line of magnitude and in some case, a square 3x3 center around the indice of the considered value in the magnitude buffer(so it will check the magnitude in a 3x3 square)
	for(int x = 3; x< ImageIn->numRows; x++)
	{
		int x3 = x%3;
		//First the computation of the partial sum, magnitude and gradient
		//I can't put the output wit it because it need the value of the magnitude in a 3x3x square meaning in the case of the last value of a vector willl need this process
		//the value in the bottom right will not have been calculated yet => need to split the loop in two process
		Img_tmp_temporary->pData[x3*w].y = (ImageIn->pData[(x-1)*w] + (ImageIn->pData[x*w]<<1) + ImageIn->pData[(x+1)*w])>>2;
		for(int y =1; y< w-7;y+=8)
		{
			int indice = x*w + y;
			int indice3 = x3*w + y;
			q15x8_t vect1 = vldrhq_s16(&ImageIn->pData[indice-1]);
			q15x8_t vect2 = vldrhq_s16(&ImageIn->pData[indice]);

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
			q15x8x2_t vec_x_y_1 = vld2q_s16(&Img_tmp_temporary->pData[((x-2)%3)*w + y].x);
			vst2q_s16(&Img_tmp_temporary->pData[indice3].x, vect_buff_di);
			q15x8x2_t vec_y_1 = vld2q_s16(&Img_tmp_temporary->pData[((x-1)%3)*w + y-1].x);
			
			q15x8x2_t vect_grad_1;
			q15x8_t vectgradx = vsubq_s16(vec_x_y_1.val[0], vect_buff_di.val[0]);
			q15x8x2_t vec_y_2 = vld2q_s16(&Img_tmp_temporary->pData[((x-1)%3)*w + y+1].x);
			int numVect = 8;
			int16x8_t vect_res_1;
			q31x4_t vect_gradx_1 = vmullbq_int_s16(vectgradx, vectgradx);
			q15x8_t vectgrady = vsubq_s16(vec_y_1.val[1], vec_y_2.val[1]);
			q31x4_t vect_grady_1 = vmullbq_int_s16(vectgrady, vectgrady);
			
			vect_gradx_1 = vaddq_s32(vect_gradx_1, vect_grady_1);
			q31x4_t vect_gradx_2 = vmulltq_int_s16(vectgradx, vectgradx);
			vect_gradx_1 = vshrq(vect_gradx_1, 1);
			q31x4_t vect_grady_2 = vmulltq_int_s16(vectgrady, vectgrady);
			
			vect_gradx_2 = vaddq_s32(vect_gradx_2, vect_grady_2);
			vect_gradx_2 = vshrq(vect_gradx_2, 1);
			
			vect_grad_1.val[0] = vectgradx;
			vect_grad_1.val[1] = vectgrady;
            vst2q(&Img_tmp_grad->pData[(x-1)%3*w+y].x, vect_grad_1);
            for(int j =0; j < numVect; j+=2)
			{
				if(vectgradx[j]==0&&vectgrady[j]==0)
				{
					vect_res_1[j] =  0;
					continue;
				}
				//mag
				q15_t out;
				q31_t root ;
				arm_sqrt_q31(vect_gradx_1[j>>1], &root);
      			out = root>>15;
				vect_res_1[j] = out;
			}
			for(int j =1; j < numVect; j+=2)
			{
				if(vectgradx[j]==0&&vectgrady[j]==0)
				{
					vect_res_1[j] =  0;
					continue;
				}
				//mag
				q15_t out;
				q31_t root ;
				arm_sqrt_q31(vect_gradx_2[j>>1], &root);
      			out = root>>15;
				vect_res_1[j] = out;
			}
			vst1q_s16((int16_t*)&Img_tmp_mag->pData[((x-1)%3)*w + y], vect_res_1);
		}
		//Tail for the process of the temporary image gradient and magnitude
		for(int j = 0; j<8; j++)
		{
			int y = ((w-2)&0xFFF8) + j;
			if((y==0||y == w-1)&&x!=0&& x != ImageIn->numRows-1)
			{
				Img_tmp_temporary->pData[x3*w +y].y = (ImageIn->pData[(x-1)*w+y] + (ImageIn->pData[x*w+y]<<1) + ImageIn->pData[(x+1)*w+y])>>2;
				vect_mag[j] =0;
				continue;
			}
			if( x==0 || y==0||y == w-1)
			{
				vect_mag[j] = 0;
				continue;
			}
			Img_tmp_temporary->pData[x3*w +y].y = (ImageIn->pData[(x-1)*w+y] + (ImageIn->pData[x*w+y]<<1) + ImageIn->pData[(x+1)*w+y])>>2;
			Img_tmp_temporary->pData[x3*w +y].x = (ImageIn->pData[x*w+(y-1)] + (ImageIn->pData[x*w+(y)]<<1) + ImageIn->pData[x*w+(y+1)])>>2;
			if(x==1)
			{
				continue;
			}
			q63_t gradx = Img_tmp_temporary->pData[((x-2)%3)*w +y].x - Img_tmp_temporary->pData[(x3)*w +y].x;
			q63_t grady = Img_tmp_temporary->pData[(x-1)%3*w +(y-1)].y - Img_tmp_temporary->pData[(x-1)%3*w +(y+1)].y;
			Img_tmp_grad->pData[(x-1)%3*w+y].x = gradx;
			Img_tmp_grad->pData[(x-1)%3*w+y].y = grady;
			if(gradx==0&&grady==0)
			{
				vect_mag[j] = 0;
				continue;
			}
			//mag
			q15_t vect[2] = { (q15_t)gradx, (q15_t)grady};

			q31_t in[2] = {((q31_t)vect[0]), ((q31_t)vect[1])};
			q31_t out2[2];
			q31_t out3;
			q31_t root;
			out2[0]=(in[0]*in[0]);
			out2[1]=(in[1]*in[1]);
			out3 = (out2[0]+out2[1])>>1;
			arm_sqrt_q31(out3, &root);
			vect_mag[j] = root>>15;
		}
		int y = ((w-2)&0xFFF8);
		(void)ImageOut;
		vst1q(&Img_tmp_mag->pData[((((x-1)%3*w)) + y)], vect_mag);
		//Computation of the output value, in vector
		//The focus heve been put on trying to avoid as much unecessary test as possible
        for(int y= 1; y < ((ImageIn-> numCols-2)&0xFFF8); y+=8)
        {
			int indice = (x-2)*ImageIn->numCols +y;
            vect_mag = vld1q(&Img_tmp_mag->pData[((x-2)%3) * (ImageIn->numCols)+y]);
			q15x8_t vect_out;
            int16x8x2_t vect_grad = vld2q_s16(&Img_tmp_grad->pData[((x-2)%3)*ImageIn->numCols+y].x);
			for(int j =0; j< 8; j+=1)
            {
                int mag = vect_mag[j];
				//First thing to test is the magnitude, if it's under the lowthreshold, then there is no need to test further, out will be 0
				if(mag < low_threshold)
				{
					vect_out[j] = 0;
					continue;
				}
				//we need to compare the magnitude with two other one, determine by the angle
				//if the magnitude is higher than those two, we check if the magnitude is higer the the high threshold 
				//if yes, the output value is 255
				//if no, we need to check in the 3x3 square of magnitude if ther is at least one that is higher than the value
				//if yes 255, else 0
				//This is the step where checking the angle is unavoidable and where we used the gradient buffer, in scalar because of the atan
				else
				{
					q15_t angle =0;
					arm_atan2_q15(vect_grad.val[0][j], vect_grad.val[1][j], &angle);

					arm_abs_q15( &angle, &angle, 1);
					//int grady = vect_grad.val[0][j];
					if((angle ) < (0xC4A))//22 in rad in 2.13
					{
						if( mag <= Img_tmp_mag->pData[((x-2)%3)*ImageIn->numCols+y+j-1] || mag <= Img_tmp_mag->pData[((x-2)%3)*ImageIn->numCols+y+j+1])
						{
							vect_out[j] = 0;
							continue;
						}
						else
						{
							if(mag < high_threshold)
							{
								if(Img_tmp_mag->pData[(x-3)%3*ImageIn->numCols+y-1+j]>=(int)(high_threshold) ||Img_tmp_mag->pData[(x-3)%3*ImageIn->numCols+y+j]>=(int)(high_threshold) ||Img_tmp_mag->pData[(x-3)%3*ImageIn->numCols+y+1+j]>=(int)(high_threshold)||Img_tmp_mag->pData[(x-1)%3*ImageIn->numCols+y+j+1]>=(int)(high_threshold) ||Img_tmp_mag->pData[(x-1)%3*ImageIn->numCols+y+j]>=(int)(high_threshold) ||Img_tmp_mag->pData[(x-1)%3*ImageIn->numCols+y+j-1]>=(int)(high_threshold))
								{
									vect_out[j] = 0x7FFF;
									continue;
								}
								else
								{
									vect_out[j] = 0;
									continue;
								}
							}
							else
							{
								vect_out[j] = 0x7FFF;
								continue;
							}
						}
						continue;
					}
					else if((angle ) < (0x256C))//67 in rad in 2.13
					{
						/*if((grady&0x8000)>>15)
						{
							if( mag <= Img_tmp_mag->pData[(x-2)%3*ImageIn->numCols+y+j-w-1] || mag <= Img_tmp_mag->pData[(x-1)%3*ImageIn->numCols+y+j+1])
							{
								vect_out[j] = 0;
								continue;
							}
							else
							{
								if(mag < high_threshold)
								{
									if (Img_tmp_mag->pData[(x-3)%3*ImageIn->numCols+y+j]>=(int)(high_threshold) ||Img_tmp_mag->pData[(x-3)%3*ImageIn->numCols+y+j+1]>=(int)(high_threshold) ||Img_tmp_mag->pData[(x-2)%3*ImageIn->numCols+y+j-1]>=(int)(high_threshold) ||Img_tmp_mag->pData[(x-2)%3*ImageIn->numCols+y+j+1]>=(int)(high_threshold) ||Img_tmp_mag->pData[(x-1)%3*ImageIn->numCols+y+j]>=(int)(high_threshold) ||Img_tmp_mag->pData[(x-1)%3*ImageIn->numCols+y+j-1]>=(int)(high_threshold))
									{
										vect_out[j] = 255;
										continue;
									}
									else
									{
										vect_out[j] = 0;
										continue;
									}
								}
								else
								{
									vect_out[j] = 255;
									continue;
								}
							}
							continue;
						}
						else
						{*/
							if( mag <= Img_tmp_mag->pData[(x-1)%3*ImageIn->numCols+y+j-1] || mag <= Img_tmp_mag->pData[(x-3)%3*ImageIn->numCols+y+j+1])
							{
								vect_out[j] = 0;
								continue;
							}
							else
							{
								if(mag < high_threshold)
								{
									if (Img_tmp_mag->pData[(x-3)%3*ImageIn->numCols+y+j-1]>=(int)(high_threshold) ||Img_tmp_mag->pData[(x-3)%3*ImageIn->numCols+y+j]>=(int)(high_threshold)||Img_tmp_mag->pData[(x-2)%3*ImageIn->numCols+y+j-1]>=(int)(high_threshold) ||Img_tmp_mag->pData[(x-2)%3*ImageIn->numCols+y+j+1]>=(int)(high_threshold)||Img_tmp_mag->pData[(x-1)%3*ImageIn->numCols+y+j]>=(int)(high_threshold) ||Img_tmp_mag->pData[(x-1)%3*ImageIn->numCols+y+j-1]>=(int)(high_threshold))
									{
										vect_out[j] = 0x7FFF;
										continue;
									}
									else
									{
										vect_out[j] = 0;
										continue;
									}
								}
								else
								{
									vect_out[j] = 0x7FFF;
									continue;
								}
							}
							continue;
						//}
							
					}
					else if((angle ) < (0x3E8E))//112 in rad in 2.13
					{
						if(  0 <= Img_tmp_mag->pData[(x-3)%3*ImageIn->numCols+y+j] - mag||  0 <= Img_tmp_mag->pData[(x-1)%3*ImageIn->numCols+y+j] - mag)
						{
							vect_out[j] = 0;
							continue;
						}
						else
						{
							if(mag < high_threshold)
							{
								if (Img_tmp_mag->pData[(x-3)%3*ImageIn->numCols+y+j-1]>=(int)(high_threshold)||Img_tmp_mag->pData[(x-3)%3*ImageIn->numCols+y+j+1]>=(int)(high_threshold) ||Img_tmp_mag->pData[(x-2)%3*ImageIn->numCols+y+j-1]>=(int)(high_threshold) ||Img_tmp_mag->pData[(x-2)%3*ImageIn->numCols+y+j+1]>=(int)(high_threshold) ||Img_tmp_mag->pData[(x-1)%3*ImageIn->numCols+y+j+1]>=(int)(high_threshold)||Img_tmp_mag->pData[(x-1)%3*ImageIn->numCols+y+j-1]>=(int)(high_threshold))
								{
									vect_out[j] = 0x7FFF;
									continue;
								}
								else
								{
									vect_out[j] = 0;
									continue;
								}
							}
							else
							{
								vect_out[j] = 0x7FFF;
								continue;
							}
						}
						continue;
					}
					else if((angle ) < (0x595C))//160 in rad in 2.13
					{
						/*if((grady&0x8000)>>15)
						{
							if( mag <= Img_tmp_mag->pData[(x-1)%3*ImageIn->numCols+y+j-1] || mag <= Img_tmp_mag->pData[(x-3)%3*ImageIn->numCols+y+j+1])
							{
								vect_out[j] = 0;
								continue;
							}
							else
							{
								if(mag < high_threshold)
								{
									if (Img_tmp_mag->pData[(x-3)%3*ImageIn->numCols+y+j-1]>=(int)(high_threshold) ||Img_tmp_mag->pData[(x-3)%3*ImageIn->numCols+y+j]>=(int)(high_threshold)||Img_tmp_mag->pData[(x-2)%3*ImageIn->numCols+y+j-1]>=(int)(high_threshold) ||Img_tmp_mag->pData[(x-2)%3*ImageIn->numCols+y+j+1]>=(int)(high_threshold)||Img_tmp_mag->pData[(x-1)%3*ImageIn->numCols+y+j]>=(int)(high_threshold) ||Img_tmp_mag->pData[(x-1)%3*ImageIn->numCols+y+j-1]>=(int)(high_threshold))
									{
										vect_out[j] = 255;
										continue;
									}
									else
									{
										vect_out[j] = 0;
										continue;
									}
								}
								else
								{
									vect_out[j] = 255;
									continue;
								}
							}
							continue;
						}
						else
						{*/
							if( mag <= Img_tmp_mag->pData[(x-3)%3*ImageIn->numCols+y+j-1] || mag <= Img_tmp_mag->pData[(x-1)%3*ImageIn->numCols+y+j+1])
							{
								vect_out[j] = 0;
								continue;
							}
							else
							{
								if(mag < high_threshold)
								{
									if (Img_tmp_mag->pData[(x-3)%3*ImageIn->numCols+y+j]>=(int)(high_threshold) ||Img_tmp_mag->pData[(x-3)%3*ImageIn->numCols+y+j+1]>=(int)(high_threshold) ||Img_tmp_mag->pData[(x-2)%3*ImageIn->numCols+y+j-1]>=(int)(high_threshold) ||Img_tmp_mag->pData[(x-2)%3*ImageIn->numCols+y+j+1]>=(int)(high_threshold) ||Img_tmp_mag->pData[(x-1)%3*ImageIn->numCols+y+j]>=(int)(high_threshold) ||Img_tmp_mag->pData[(x-1)%3*ImageIn->numCols+y+j-1]>=(int)(high_threshold))
									{
										vect_out[j] = 0x7FFF;
										continue;
									}
									else
									{
										vect_out[j] = 0;
										continue;
									}
								}
								else
								{
									vect_out[j] = 0x7FFF;
									continue;
								}
							}
							continue;
						//}	
					}
					else//case tan between 135+22.5 and 180
					{
						if( mag <= Img_tmp_mag->pData[(x-2)%3*ImageIn->numCols+y+j-1] || mag <= Img_tmp_mag->pData[(x-2)%3*ImageIn->numCols+y+j+1])
						{
							vect_out[j] = 0;
							continue;
						}
						else
						{
							if(mag < high_threshold)
							{
								if(Img_tmp_mag->pData[(x-3)%3*ImageIn->numCols+y+j-1]>=(int)(high_threshold) ||Img_tmp_mag->pData[(x-3)%3*ImageIn->numCols+y+j]>=(int)(high_threshold) ||Img_tmp_mag->pData[(x-3)%3*ImageIn->numCols+y+j+1]>=(int)(high_threshold)||Img_tmp_mag->pData[(x-1)%3*ImageIn->numCols+y+j+1]>=(int)(high_threshold) ||Img_tmp_mag->pData[(x-1)%3*ImageIn->numCols+y+j]>=(int)(high_threshold) ||Img_tmp_mag->pData[(x-1)%3*ImageIn->numCols+y+j-1]>=(int)(high_threshold))
								{
									vect_out[j] = 0x7FFF;
									continue;
								}
								else
								{
									vect_out[j] = 0;
									continue;
								}
							}
							else
							{
								vect_out[j] = 0x7FFF;
								continue;
							}
						}
						continue;
					}
				}
            }
			vst1q(&ImageOut->pData[indice], vect_out);
        }
		//This is the tail of the computation of the output value
		for(int y= ((ImageIn-> numCols-2)&0xFFF8); y < ImageIn-> numCols; y++)
        {
			int indice = (x-2)*ImageIn->numCols +y;
			int indicepcent = (x-2)%3*ImageIn->numCols+y;
            int mag = Img_tmp_mag->pData[((x-2)%3) * (ImageIn->numCols)+y];
            if(mag != 0)
			{
				if(mag < low_threshold)
				{
					ImageOut->pData[indice] = 0;
					continue;
				}
				else
				{
					q15_t angle;
					divergence_q15_t grad = Img_tmp_grad->pData[((x-2)%3)*ImageIn->numCols+y];
					arm_atan2_q15(grad.x, grad.y, &angle);
					arm_abs_q15( &angle, &angle, 1);
					//int grady = grad.x;
					if((angle ) < (0xC4A))//22 in rad in 2.13
					{
				        	if( mag <= Img_tmp_mag->pData[((x-2)%3)*ImageIn->numCols+y-1] || mag <= Img_tmp_mag->pData[((x-2)%3)*ImageIn->numCols+y+1])
                            {
                                ImageOut->pData[indice] = 0;
                            	continue;
							}
                            else
                            {
                                if(mag < high_threshold)
                                {
                                    if(Img_tmp_mag->pData[(x-3)%3*ImageIn->numCols+y-1]>=(int)(high_threshold) ||Img_tmp_mag->pData[(x-3)%3*ImageIn->numCols+y]>=(int)(high_threshold) ||Img_tmp_mag->pData[(x-3)%3*ImageIn->numCols+y+1]>=(int)(high_threshold)||Img_tmp_mag->pData[(x-1)%3*ImageIn->numCols+y+1]>=(int)(high_threshold) ||Img_tmp_mag->pData[(x-1)%3*ImageIn->numCols+y]>=(int)(high_threshold) ||Img_tmp_mag->pData[(x-1)%3*ImageIn->numCols+y-1]>=(int)(high_threshold))
                                    {
                                        ImageOut->pData[indice] = 0x7FFF;
                                    	continue;
									}
                                    else
                                    {
                                        ImageOut->pData[indice] = 0;
                                    	continue;
									}
								}
                                else
                                {
                                    ImageOut->pData[indice] = 0x7FFF;
                                	continue;
								}
                            }
				        	continue;
					}
					else if((angle ) < (0x256C))//67 in rad in 2.13
					{
							/*if((grady&0x8000)>>15)
				        	{
				        		if( mag <= Img_tmp_mag->pData[(x-3)%3*ImageIn->numCols+y-1] || mag <= Img_tmp_mag->pData[(x-1)%3*ImageIn->numCols+y+1])
                                {
                                    ImageOut->pData[indice] = 0;
                                	continue;
								}
                                else
                                {
                                    if(mag < high_threshold)
                                    {
                                        if (Img_tmp_mag->pData[(x-3)%3*ImageIn->numCols+y]>=(int)(high_threshold) ||Img_tmp_mag->pData[(x-3)%3*ImageIn->numCols+y+1]>=(int)(high_threshold) ||Img_tmp_mag->pData[indicepcent-1]>=(int)(high_threshold) ||Img_tmp_mag->pData[indicepcent+1]>=(int)(high_threshold) ||Img_tmp_mag->pData[(x-1)%3*ImageIn->numCols+y]>=(int)(high_threshold) ||Img_tmp_mag->pData[(x-1)%3*ImageIn->numCols+y-1]>=(int)(high_threshold))
                                        {
                                            ImageOut->pData[indice] = 255;
											continue;
										}
                                        else
                                        {
                                            ImageOut->pData[indice] = 0;
                                        	continue;
										}
                                    }
                                    else
                                    {
                                        ImageOut->pData[indice] = 255;
                                    	continue;
									}
                                }
				        		continue;
				        	}
				        	else
				        	{*/
				        		if( mag <= Img_tmp_mag->pData[(x-1)%3*ImageIn->numCols+y-1] || mag <= Img_tmp_mag->pData[(x-3)%3*ImageIn->numCols+y+1])
                                {
                                    ImageOut->pData[indice] = 0;
                                	continue;
								}
                                else
                                {
                                    if(mag < high_threshold)
                                    {
                                        if (Img_tmp_mag->pData[(x-3)%3*ImageIn->numCols+y-1]>=(int)(high_threshold) ||Img_tmp_mag->pData[(x-3)%3*ImageIn->numCols+y]>=(int)(high_threshold)||Img_tmp_mag->pData[indicepcent-1]>=(int)(high_threshold) ||Img_tmp_mag->pData[indicepcent+1]>=(int)(high_threshold)||Img_tmp_mag->pData[(x-1)%3*ImageIn->numCols+y]>=(int)(high_threshold) ||Img_tmp_mag->pData[(x-1)%3*ImageIn->numCols+y-1]>=(int)(high_threshold))
                                        {
                                            ImageOut->pData[indice] = 0x7FFF;
                                        	continue;
										}
                                        else
                                        {
                                            ImageOut->pData[indice] = 0;
                                        	continue;
										}
                                    }
                                    else
                                    {
                                        ImageOut->pData[indice] = 0x7FFF;
                                    	continue;
									}
                                }
				        		continue;
				        	//}	
					}
					else if((angle ) < (0x3E8E))//112 in rad in 2.13
					{
				        	if(  0 <= Img_tmp_mag->pData[(x-3)%3*ImageIn->numCols+y] - mag||  0 <= Img_tmp_mag->pData[(x-1)%3*ImageIn->numCols+y] - mag)
                            {
                                ImageOut->pData[indice] = 0;
                            	continue;
							}
                            else
                            {
                                if(mag < high_threshold)
                                {
                                    if (Img_tmp_mag->pData[(x-3)%3*ImageIn->numCols+y-1]>=(int)(high_threshold)||Img_tmp_mag->pData[(x-3)%3*ImageIn->numCols+y+1]>=(int)(high_threshold) ||Img_tmp_mag->pData[indicepcent-1]>=(int)(high_threshold) ||Img_tmp_mag->pData[indicepcent+1]>=(int)(high_threshold) ||Img_tmp_mag->pData[(x-1)%3*ImageIn->numCols+y+1]>=(int)(high_threshold)||Img_tmp_mag->pData[(x-1)%3*ImageIn->numCols+y-1]>=(int)(high_threshold))
                                    {
                                        ImageOut->pData[indice] = 0x7FFF;
										continue;
									}
                                    else
                                    {
                                        ImageOut->pData[indice] = 0;
										continue;
									}
                                }
                                else
                                {
                                    ImageOut->pData[indice] = 0x7FFF;
									continue;
								}
                            }
            	        	continue;
					}
					else if((angle ) < (0x595C))//160 in rad in 2.13
					{
							/*if((grady&0x8000)>>15)
				        	{
				        		if( mag <= Img_tmp_mag->pData[(x-1)%3*ImageIn->numCols+y-1] || mag <= Img_tmp_mag->pData[(x-3)%3*ImageIn->numCols+y+1])
                                {
                                    ImageOut->pData[indice] = 0;
                                	continue;
								}
                                else
                                {
                                    if(mag < high_threshold)
                                    {
                                        if (Img_tmp_mag->pData[(x-3)%3*ImageIn->numCols+y-1]>=(int)(high_threshold) ||Img_tmp_mag->pData[(x-3)%3*ImageIn->numCols+y]>=(int)(high_threshold)||Img_tmp_mag->pData[indicepcent-1]>=(int)(high_threshold) ||Img_tmp_mag->pData[indicepcent+1]>=(int)(high_threshold)||Img_tmp_mag->pData[(x-1)%3*ImageIn->numCols+y]>=(int)(high_threshold) ||Img_tmp_mag->pData[(x-1)%3*ImageIn->numCols+y-1]>=(int)(high_threshold))
                                        {
                                            ImageOut->pData[indice] = 255;
                                        	continue;
										}
                                        else
                                        {
                                            ImageOut->pData[indice] = 0;
                                        	continue;
										}
                                    }
                                    else
                                    {
                                        ImageOut->pData[indice] = 255;
                                    	continue;
									}
                                }
				        		continue;
				        	}
				        	else
				        	{*/
				        		if( mag <= Img_tmp_mag->pData[(x-3)%3*ImageIn->numCols+y-1] || mag <= Img_tmp_mag->pData[(x-1)%3*ImageIn->numCols+y+1])
                                {
                                    ImageOut->pData[indice] = 0;
                                	continue;
								}
                                else
                                {
                                    if(mag < high_threshold)
                                    {
                                        if (Img_tmp_mag->pData[(x-3)%3*ImageIn->numCols+y]>=(int)(high_threshold) ||Img_tmp_mag->pData[(x-3)%3*ImageIn->numCols+y+1]>=(int)(high_threshold) ||Img_tmp_mag->pData[indicepcent-1]>=(int)(high_threshold) ||Img_tmp_mag->pData[indicepcent+1]>=(int)(high_threshold) ||Img_tmp_mag->pData[(x-1)%3*ImageIn->numCols+y]>=(int)(high_threshold) ||Img_tmp_mag->pData[(x-1)%3*ImageIn->numCols+y-1]>=(int)(high_threshold))
                                        {
                                            ImageOut->pData[indice] = 0x7FFF;
                                        	continue;
										}
                                        else
                                        {
                                            ImageOut->pData[indice] = 0;
                                        	continue;
										}
                                    }
                                    else
                                    {
                                        ImageOut->pData[indice] = 0x7FFF;
                                    	continue;
									}
                                }
				        		continue;
				        	//}
					}
					else//case tan between 135+22.5 and 180
					{
							if( mag <= Img_tmp_mag->pData[indicepcent-1] || mag <= Img_tmp_mag->pData[indicepcent+1])
                            {
                                ImageOut->pData[indice] = 0;
                            	continue;
							}
                            else
                            {
                                if(mag < high_threshold)
                                {
                                    if(Img_tmp_mag->pData[(x-3)%3*ImageIn->numCols+y-1]>=(int)(high_threshold) ||Img_tmp_mag->pData[(x-3)%3*ImageIn->numCols+y]>=(int)(high_threshold) ||Img_tmp_mag->pData[(x-3)%3*ImageIn->numCols+y+1]>=(int)(high_threshold)||Img_tmp_mag->pData[(x-1)%3*ImageIn->numCols+y+1]>=(int)(high_threshold) ||Img_tmp_mag->pData[(x-1)%3*ImageIn->numCols+y]>=(int)(high_threshold) ||Img_tmp_mag->pData[(x-1)%3*ImageIn->numCols+y-1]>=(int)(high_threshold))
                                    {
                                        ImageOut->pData[indice] = 0x7FFF;
                                    	continue;
									}
                                    else
                                    {
                                        ImageOut->pData[indice] = 0;
                                    	continue;
									}
                                }
                                else
                                {
                                    ImageOut->pData[indice] = 0x7FFF;
                                	continue;
								}
                            }
            	        	continue;
					}
				}
			}
			else
			{

				ImageOut->pData[indice] = 0;
			}
		}
	}
	//This is the loop to do the last lines
	//For the whole process, the line processeds where using different indice and now we catch up(nee to check for the before last one that the las line of magnitude read is 0 due to the border)
	//only one line to catch
	#if 0
	for(int x = ImageIn->numRows; x <ImageIn->numRows+1 ;x++)
	{
		//same proces as previously running on all the line minus the tail
		//to compute the output value
		for(int y= 0; y < ((ImageIn-> numCols-2)&0xFFF8); y+=8)
		{
			int indice = (x-2)*ImageIn->numCols +y;
			int indicepcent = (x-2)%3*ImageIn->numCols+y;
			vect_mag = vld1q(&Img_tmp_mag->pData[((x-2)%3) * (ImageIn->numCols)+y]);
			q15x8_t vect_out;
			int16x8x2_t vect_grad = vld2q_s16(&Img_tmp_grad->pData[((x-2)%3)*ImageIn->numCols+y].x);
			for(int j =0; j< 8; j++)
			{
				int mag = vect_mag[j];
				if(mag != 0)
				{
					if(mag < low_threshold)
					{
						vect_out[j] = 0;
						
						continue;
					}
					else
					{
						q15_t angle;
						arm_atan2_q15(vect_grad.val[0][j],  vect_grad.val[1][j], &angle);
						arm_abs_q15( &angle, &angle, 1);
						//int grady = vect_grad.val[0][j];
						if((angle ) < (0xC4A))//22 in rad in 2.13
						{
				        	if( mag <= Img_tmp_mag->pData[((x-2)%3)*ImageIn->numCols+y+j-1] || mag <= Img_tmp_mag->pData[((x-2)%3)*ImageIn->numCols+y+j+1])
                            {
                                vect_out[j] = 0;
                            	continue;
							}
                            else
                            {
                                if(mag < high_threshold)
                                {
                                    if(Img_tmp_mag->pData[(x-3)%3*ImageIn->numCols+y-1+j]>=(int)(high_threshold) ||Img_tmp_mag->pData[(x-3)%3*ImageIn->numCols+y+j]>=(int)(high_threshold) ||Img_tmp_mag->pData[(x-3)%3*ImageIn->numCols+y+1+j]>=(int)(high_threshold)||Img_tmp_mag->pData[(x-1)%3*ImageIn->numCols+y+j+1]>=(int)(high_threshold) ||Img_tmp_mag->pData[(x-1)%3*ImageIn->numCols+y+j]>=(int)(high_threshold) ||Img_tmp_mag->pData[(x-1)%3*ImageIn->numCols+y+j-1]>=(int)(high_threshold))
                                    {
                                        vect_out[j] = 0x7FFF;
                                    	continue;
									}
                                    else
                                    {
                                        vect_out[j] = 0;
                                    	continue;
									}
								}
                                else
                                {
                                    vect_out[j] = 0x7FFF;
                                	continue;
								}
                            }
				        	continue;
						}
						else if((angle ) < (0x256C))//67 in rad in 2.13
						{
            	            /*if((grady&0x8000)>>15)
				        	{
				        		if( mag <= Img_tmp_mag->pData[(x-3)%3*ImageIn->numCols+y+j-1] || mag <= Img_tmp_mag->pData[(x-1)%3*ImageIn->numCols+y+j+1])
                                {
                                    vect_out[j] = 0;
                                	continue;
								}
                                else
                                {
                                    if(mag < high_threshold)
                                    {
                                        if (Img_tmp_mag->pData[(x-3)%3*ImageIn->numCols+y+j]>=(int)(high_threshold) ||Img_tmp_mag->pData[(x-3)%3*ImageIn->numCols+y+j+1]>=(int)(high_threshold) ||Img_tmp_mag->pData[indicepcent+j-1]>=(int)(high_threshold) ||Img_tmp_mag->pData[indicepcent+j+1]>=(int)(high_threshold) ||Img_tmp_mag->pData[(x-1)%3*ImageIn->numCols+y+j]>=(int)(high_threshold) ||Img_tmp_mag->pData[(x-1)%3*ImageIn->numCols+y+j-1]>=(int)(high_threshold))
                                        {
                                            vect_out[j] = 255;
											continue;
										}
                                        else
                                        {
                                            vect_out[j] = 0;
                                        	continue;
										}
                                    }
                                    else
                                    {
                                        vect_out[j] = 255;
                                    	continue;
									}
                                }
				        		continue;
				        	}
				        	else
				        	{*/
				        		if( mag <= Img_tmp_mag->pData[(x-1)%3*ImageIn->numCols+y+j-1] || mag <= Img_tmp_mag->pData[(x-3)%3*ImageIn->numCols+y+j+1])
                                {
                                    vect_out[j] = 0;
                                	continue;
								}
                                else
                                {
                                    if(mag < high_threshold)
                                    {
                                        if (Img_tmp_mag->pData[(x-3)%3*ImageIn->numCols+y+j-1]>=(int)(high_threshold) ||Img_tmp_mag->pData[(x-3)%3*ImageIn->numCols+y+j]>=(int)(high_threshold)||Img_tmp_mag->pData[indicepcent+j-1]>=(int)(high_threshold) ||Img_tmp_mag->pData[indicepcent+j+1]>=(int)(high_threshold)||Img_tmp_mag->pData[(x-1)%3*ImageIn->numCols+y+j]>=(int)(high_threshold) ||Img_tmp_mag->pData[(x-1)%3*ImageIn->numCols+y+j-1]>=(int)(high_threshold))
                                        {
                                            vect_out[j] = 0x7FFF;
                                        	continue;
										}
                                        else
                                        {
                                            vect_out[j] = 0;
                                        	continue;
										}
                                    }
                                    else
                                    {
                                        vect_out[j] = 0x7FFF;
                                    	continue;
									}
                                }
				        		continue;
				        	//}
						}
						else if((angle ) < (0x3E8E))//112 in rad in 2.13
						{
				        	if(  0 <= Img_tmp_mag->pData[(x-3)%3*ImageIn->numCols+y+j] - mag||  0 <= Img_tmp_mag->pData[(x-1)%3*ImageIn->numCols+y+j] - mag)
                            {
                                vect_out[j] = 0;
                            	continue;
							}
                            else
                            {
                                if(mag < high_threshold)
                                {
                                    if (Img_tmp_mag->pData[(x-3)%3*ImageIn->numCols+y+j-1]>=(int)(high_threshold)||Img_tmp_mag->pData[(x-3)%3*ImageIn->numCols+y+j+1]>=(int)(high_threshold) ||Img_tmp_mag->pData[indicepcent+j-1]>=(int)(high_threshold) ||Img_tmp_mag->pData[indicepcent+j+1]>=(int)(high_threshold) ||Img_tmp_mag->pData[(x-1)%3*ImageIn->numCols+y+j+1]>=(int)(high_threshold)||Img_tmp_mag->pData[(x-1)%3*ImageIn->numCols+y+j-1]>=(int)(high_threshold))
                                    {
                                        vect_out[j] = 0x7FFF;
                                    	continue;
									}
                                    else
                                    {
                                        vect_out[j] = 0;
                                    	continue;
									}
                                }
                                else
                                {
                                    vect_out[j] = 0x7FFF;
                                	continue;
								}
                            }   
            	        	continue;
						}
						else if((angle ) < (0x595C))//160 in rad in 2.13
						{
				        	/*if((grady&0x8000)>>15)
				        	{
				        		if( mag <= Img_tmp_mag->pData[(x-1)%3*ImageIn->numCols+y+j-1] || mag <= Img_tmp_mag->pData[(x-3)%3*ImageIn->numCols+y+j+1])
                                {
                                    vect_out[j] = 0;
                                	continue;
								}
                                else
                                {
                                    if(mag < high_threshold)
                                    {
                                        if (Img_tmp_mag->pData[(x-3)%3*ImageIn->numCols+y+j-1]>=(int)(high_threshold) ||Img_tmp_mag->pData[(x-3)%3*ImageIn->numCols+y+j]>=(int)(high_threshold)||Img_tmp_mag->pData[indicepcent+j-1]>=(int)(high_threshold) ||Img_tmp_mag->pData[indicepcent+j+1]>=(int)(high_threshold)||Img_tmp_mag->pData[(x-1)%3*ImageIn->numCols+y+j]>=(int)(high_threshold) ||Img_tmp_mag->pData[(x-1)%3*ImageIn->numCols+y+j-1]>=(int)(high_threshold))
                                        {
                                            vect_out[j] = 255;
                                        	continue;
										}
                                        else
                                        {
                                            vect_out[j] = 0;
                                        	continue;
										}
                                    }
                                    else
                                    {
                                        vect_out[j] = 255;
                                    	continue;
									}
                                }
				        		continue;
				        	}
				        	else
				        	{*/
				        		if( mag <= Img_tmp_mag->pData[(x-3)%3*ImageIn->numCols+y+j-1] || mag <= Img_tmp_mag->pData[(x-1)%3*ImageIn->numCols+y+j+1])
                                {
                                    vect_out[j] = 0;
                                	continue;
								}
                                else
                                {
                                    if(mag < high_threshold)
                                    {
                                        if (Img_tmp_mag->pData[(x-3)%3*ImageIn->numCols+y+j]>=(int)(high_threshold) ||Img_tmp_mag->pData[(x-3)%3*ImageIn->numCols+y+j+1]>=(int)(high_threshold) ||Img_tmp_mag->pData[indicepcent+j-1]>=(int)(high_threshold) ||Img_tmp_mag->pData[indicepcent+j+1]>=(int)(high_threshold) ||Img_tmp_mag->pData[(x-1)%3*ImageIn->numCols+y+j]>=(int)(high_threshold) ||Img_tmp_mag->pData[(x-1)%3*ImageIn->numCols+y+j-1]>=(int)(high_threshold))
                                        {
                                            vect_out[j] = 0x7FFF;
                                        	continue;
										}
                                        else
                                        {
                                            vect_out[j] = 0;
                                        	continue;
										}
                                    }
                                    else
                                    {
                                        vect_out[j] = 0x7FFF;
                                    	continue;
									}
                                }
				        		continue;
				        	//}
						}
						else//case tan between 135+22.5 and 180
						{
				        	if( mag <= Img_tmp_mag->pData[indicepcent+j-1] || mag <= Img_tmp_mag->pData[indicepcent+j+1])
                            {
                                vect_out[j] = 0;
                            	continue;
							}
                            else
                            {
                                if(mag < high_threshold)
                                {
                                    if(Img_tmp_mag->pData[(x-3)%3*ImageIn->numCols+y+j-1]>=(int)(high_threshold) ||Img_tmp_mag->pData[(x-3)%3*ImageIn->numCols+y+j]>=(int)(high_threshold) ||Img_tmp_mag->pData[(x-3)%3*ImageIn->numCols+y+j+1]>=(int)(high_threshold)||Img_tmp_mag->pData[(x-1)%3*ImageIn->numCols+y+j+1]>=(int)(high_threshold) ||Img_tmp_mag->pData[(x-1)%3*ImageIn->numCols+y+j]>=(int)(high_threshold) ||Img_tmp_mag->pData[(x-1)%3*ImageIn->numCols+y+j-1]>=(int)(high_threshold))
                                    {
                                        vect_out[j] = 0x7FFF;
                                    	continue;
									}
                                    else
                                    {
                                        vect_out[j] = 0;
                                    	continue;
									}
                                }
                                else
                                {
                                    vect_out[j] = 0x7FFF;
                                	continue;
								}
                            }   
            	        	continue;
						}
					}
				}
				else
				{
					vect_out[j] = 0;
				}
			}
			vst1q(&ImageOut->pData[indice], vect_out);
        }
		//tail computation
		for(int y= ((ImageIn-> numCols-2)&0xFFF8); y < ImageIn-> numCols; y++)
		{
			int indice = (x-2)*ImageIn->numCols +y;
			int indicepcent = (x-2)%3*ImageIn->numCols+y;
			int mag = Img_tmp_mag->pData[((x-2)%3) * (ImageIn->numCols)+y];
			if(mag != 0)
			{
				if(mag < low_threshold)
				{
					ImageOut->pData[indice] = 0;
					continue;
				}
				else
				{
					q15_t angle;
					divergence_q15_t grad = Img_tmp_grad->pData[((x-2)%3)*ImageIn->numCols+y];
					arm_atan2_q15(grad.x, grad.y, &angle);
					arm_abs_q15( &angle, &angle, 1);
					//int grady = grad.x;
					if((angle ) < (0xC4A))//22 in rad in 2.13
					{
				        	if( mag <= Img_tmp_mag->pData[((x-2)%3)*ImageIn->numCols+y-1] || mag <= Img_tmp_mag->pData[((x-2)%3)*ImageIn->numCols+y+1])
                            {
                                ImageOut->pData[indice] = 0;
                            	continue;
							}
                            else
                            {
                                if(mag < high_threshold)
                                {
                                    if(Img_tmp_mag->pData[(x-3)%3*ImageIn->numCols+y-1]>=(int)(high_threshold) ||Img_tmp_mag->pData[(x-3)%3*ImageIn->numCols+y]>=(int)(high_threshold) ||Img_tmp_mag->pData[(x-3)%3*ImageIn->numCols+y+1]>=(int)(high_threshold)||Img_tmp_mag->pData[(x-1)%3*ImageIn->numCols+y+1]>=(int)(high_threshold) ||Img_tmp_mag->pData[(x-1)%3*ImageIn->numCols+y]>=(int)(high_threshold) ||Img_tmp_mag->pData[(x-1)%3*ImageIn->numCols+y-1]>=(int)(high_threshold))
                                    {
                                        ImageOut->pData[indice] = 0x7FFF;
                                    	continue;
									}
                                    else
                                    {
                                        ImageOut->pData[indice] = 0;
                                    	continue;
									}
								}
                                else
                                {
                                    ImageOut->pData[indice] = 0x7FFF;
                                	continue;
								}
                            }
				        	continue;
					}
					else if((angle ) < (0x256C))//67 in rad in 2.13
					{
							/*if((grady&0x8000)>>15)
				        	{
				        		if( mag <= Img_tmp_mag->pData[(x-3)%3*ImageIn->numCols+y-1] || mag <= Img_tmp_mag->pData[(x-1)%3*ImageIn->numCols+y+1])
                                {
                                    ImageOut->pData[indice] = 0;
                                	continue;
								}
                                else
                                {
                                    if(mag < high_threshold)
                                    {
                                        if (Img_tmp_mag->pData[(x-3)%3*ImageIn->numCols+y]>=(int)(high_threshold) ||Img_tmp_mag->pData[(x-3)%3*ImageIn->numCols+y+1]>=(int)(high_threshold) ||Img_tmp_mag->pData[indicepcent-1]>=(int)(high_threshold) ||Img_tmp_mag->pData[indicepcent+1]>=(int)(high_threshold) ||Img_tmp_mag->pData[(x-1)%3*ImageIn->numCols+y]>=(int)(high_threshold) ||Img_tmp_mag->pData[(x-1)%3*ImageIn->numCols+y-1]>=(int)(high_threshold))
                                        {
                                            ImageOut->pData[indice] = 255;
											continue;
										}
                                        else
                                        {
                                            ImageOut->pData[indice] = 0;
                                        	continue;
										}
                                    }
                                    else
                                    {
                                        ImageOut->pData[indice] = 255;
                                    	continue;
									}
                                }
				        		continue;
				        	}
				        	else
				        	{*/
				        		if( mag <= Img_tmp_mag->pData[(x-1)%3*ImageIn->numCols+y-1] || mag <= Img_tmp_mag->pData[(x-3)%3*ImageIn->numCols+y+1])
                                {
                                    ImageOut->pData[indice] = 0;
                                	continue;
								}
                                else
                                {
                                    if(mag < high_threshold)
                                    {
                                        if (Img_tmp_mag->pData[(x-3)%3*ImageIn->numCols+y-1]>=(int)(high_threshold) ||Img_tmp_mag->pData[(x-3)%3*ImageIn->numCols+y]>=(int)(high_threshold)||Img_tmp_mag->pData[indicepcent-1]>=(int)(high_threshold) ||Img_tmp_mag->pData[indicepcent+1]>=(int)(high_threshold)||Img_tmp_mag->pData[(x-1)%3*ImageIn->numCols+y]>=(int)(high_threshold) ||Img_tmp_mag->pData[(x-1)%3*ImageIn->numCols+y-1]>=(int)(high_threshold))
                                        {
                                            ImageOut->pData[indice] = 0x7FFF;
                                        	continue;
										}
                                        else
                                        {
                                            ImageOut->pData[indice] = 0;
                                        	continue;
										}
                                    }
                                    else
                                    {
                                        ImageOut->pData[indice] = 0x7FFF;
                                    	continue;
									}
                                }
				        		continue;
				        	//}
							
					}
					else if((angle ) < (0x3E8E))//112 in rad in 2.13
					{
				        	if(  0 <= Img_tmp_mag->pData[(x-3)%3*ImageIn->numCols+y] - mag||  0 <= Img_tmp_mag->pData[(x-1)%3*ImageIn->numCols+y] - mag)
                            {
                                ImageOut->pData[indice] = 0;
                            	continue;
							}
                            else
                            {
                                if(mag < high_threshold)
                                {
                                    if (Img_tmp_mag->pData[(x-3)%3*ImageIn->numCols+y-1]>=(int)(high_threshold)||Img_tmp_mag->pData[(x-3)%3*ImageIn->numCols+y+1]>=(int)(high_threshold) ||Img_tmp_mag->pData[indicepcent-1]>=(int)(high_threshold) ||Img_tmp_mag->pData[indicepcent+1]>=(int)(high_threshold) ||Img_tmp_mag->pData[(x-1)%3*ImageIn->numCols+y+1]>=(int)(high_threshold)||Img_tmp_mag->pData[(x-1)%3*ImageIn->numCols+y-1]>=(int)(high_threshold))
                                    {
                                        ImageOut->pData[indice] = 0x7FFF;
										continue;
									}
                                    else
                                    {
                                        ImageOut->pData[indice] = 0;
										continue;
									}
                                }
                                else
                                {
                                    ImageOut->pData[indice] = 0x7FFF;
									continue;
								}
                            }   
            	        	continue;
					}
					else if((angle ) < (0x595C))//160 in rad in 2.13
					{
							/*if((grady&0x8000)>>15)
				        	{
				        		if( mag <= Img_tmp_mag->pData[(x-1)%3*ImageIn->numCols+y-1] || mag <= Img_tmp_mag->pData[(x-3)%3*ImageIn->numCols+y+1])
                                {
                                    ImageOut->pData[indice] = 0;
                                	continue;
								}
                                else
                                {
                                    if(mag < high_threshold)
                                    {
                                        if (Img_tmp_mag->pData[(x-3)%3*ImageIn->numCols+y-1]>=(int)(high_threshold) ||Img_tmp_mag->pData[(x-3)%3*ImageIn->numCols+y]>=(int)(high_threshold)||Img_tmp_mag->pData[indicepcent-1]>=(int)(high_threshold) ||Img_tmp_mag->pData[indicepcent+1]>=(int)(high_threshold)||Img_tmp_mag->pData[(x-1)%3*ImageIn->numCols+y]>=(int)(high_threshold) ||Img_tmp_mag->pData[(x-1)%3*ImageIn->numCols+y-1]>=(int)(high_threshold))
                                        {
                                            ImageOut->pData[indice] = 255;
                                        	continue;
										}
                                        else
                                        {
                                            ImageOut->pData[indice] = 0;
                                        	continue;
										}
                                    }
                                    else
                                    {
                                        ImageOut->pData[indice] = 255;
                                    	continue;
									}
                                }
				        		continue;
				        	}
				        	else
				        	{*/
				        		if( mag <= Img_tmp_mag->pData[(x-3)%3*ImageIn->numCols+y-1] || mag <= Img_tmp_mag->pData[(x-1)%3*ImageIn->numCols+y+1])
                                {
                                    ImageOut->pData[indice] = 0;
                                	continue;
								}
                                else
                                {
                                    if(mag < high_threshold)
                                    {
                                        if (Img_tmp_mag->pData[(x-3)%3*ImageIn->numCols+y]>=(int)(high_threshold) ||Img_tmp_mag->pData[(x-3)%3*ImageIn->numCols+y+1]>=(int)(high_threshold) ||Img_tmp_mag->pData[indicepcent-1]>=(int)(high_threshold) ||Img_tmp_mag->pData[indicepcent+1]>=(int)(high_threshold) ||Img_tmp_mag->pData[(x-1)%3*ImageIn->numCols+y]>=(int)(high_threshold) ||Img_tmp_mag->pData[(x-1)%3*ImageIn->numCols+y-1]>=(int)(high_threshold))
                                        {
                                            ImageOut->pData[indice] = 0x7FFF;
                                        	continue;
										}
                                        else
                                        {
                                            ImageOut->pData[indice] = 0;
                                        	continue;
										}
                                    }
                                    else
                                    {
                                        ImageOut->pData[indice] = 0x7FFF;
                                    	continue;
									}
                                }
				        		continue;
				        	//}
				        	
					}
					else//case tan between 135+22.5 and 180
					{
							if( mag <= Img_tmp_mag->pData[indicepcent-1] || mag <= Img_tmp_mag->pData[indicepcent+1])
                            {
                                ImageOut->pData[indice] = 0;
                            	continue;
							}
                            else
                            {
                                if(mag < high_threshold)
                                {
                                    if(Img_tmp_mag->pData[(x-3)%3*ImageIn->numCols+y-1]>=(int)(high_threshold) ||Img_tmp_mag->pData[(x-3)%3*ImageIn->numCols+y]>=(int)(high_threshold) ||Img_tmp_mag->pData[(x-3)%3*ImageIn->numCols+y+1]>=(int)(high_threshold)||Img_tmp_mag->pData[(x-1)%3*ImageIn->numCols+y+1]>=(int)(high_threshold) ||Img_tmp_mag->pData[(x-1)%3*ImageIn->numCols+y]>=(int)(high_threshold) ||Img_tmp_mag->pData[(x-1)%3*ImageIn->numCols+y-1]>=(int)(high_threshold))
                                    {
                                        ImageOut->pData[indice] = 0x7FFF;
                                    	continue;
									}
                                    else
                                    {
                                        ImageOut->pData[indice] = 0;
                                    	continue;
									}
                                }
                                else
                                {
                                    ImageOut->pData[indice] = 0x7FFF;
                                	continue;
								}
                            }   
            	        	continue;
					}
				}
			}
			else
			{
				ImageOut->pData[indice] = 0;
			}
		}
	}
	#endif
}   
#endif
