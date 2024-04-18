#include "gaussian.h"
#include <stdio.h>
//#include "defines.h"
q15_t grad[9] = {   0x0800, 0x1000, 0x0800,
                    0x1000, 0x2000, 0x1000,
                    0x0800, 0x1000, 0x0800
                };
//the kernel is sized by 1/16 in order for the dot product of the gaussian filter to not saturate 
void arm_gaussian_filter_fixed_q15(arm_matrix_instance_q15* ImageIn, arm_matrix_instance_q15* ImageOut)
{
    int w =ImageIn->numCols;
    int indice;
    //main loop computing the value
    for( int y = 1; y< ImageIn -> numRows- 2; y++)//-1 for complete coverage, -2 to match OpenMV + the lign specified below
    {
        for( int x = 1; x< ImageIn-> numCols- 2; x++)
        {
            indice = y*ImageIn->numCols+x;
            q63_t res;
            q15_t matrix_in[9] = { ImageIn->pData[indice-w-1], ImageIn->pData[indice-w], ImageIn->pData[indice-w+1], ImageIn->pData[indice - 1], ImageIn->pData[indice], ImageIn->pData[indice + 1], ImageIn->pData[indice +w -1], ImageIn->pData[ indice + w], ImageIn->pData[indice + w+1]};
            //this function of dotprodict is very ineffiscent in term of performance due to the overhead of calling a function in C
            arm_dot_prod_q15(&grad[0], &matrix_in[0], 9, &res);
            res = res>>15;
            ImageOut->pData[indice] =(q15_t)(res);
        }
        //need for matching OpenMV
        //the two last column are not treated by the OpenMV function
        indice++;
        ImageOut->pData[indice] = ImageIn->pData[indice];
        //
    }
    //need for matching OpenMV
    //the two last line are not treated by OpenMV
    for(int x = 1; x< ImageIn -> numCols- 2; x++)
    {
        indice = (ImageIn->numRows- 3)*ImageIn->numCols+ x;
        ImageOut->pData[indice] = ImageIn->pData[indice];
    }
    for(int x = 1; x< ImageIn -> numCols- 1; x++)
    {
        indice = (ImageIn->numRows- 2)*ImageIn->numCols+ x;
        ImageOut->pData[indice] = ImageIn->pData[indice];
    }
    //

    //Those line are to ensure that the border remain unchanged between the input and the output image
    int x = 0;
    int y = 0;
    for( int y =0; y < 320; y++)
	{
		ImageOut->pData[x*ImageOut->numCols +y] = ImageIn->pData[x*ImageOut->numCols +y];
	}
	x = 240-1;
	for( int y =0; y < 320; y++)
	{
		ImageOut->pData[x*ImageOut->numCols +y] = ImageIn->pData[x*ImageOut->numCols +y];
	}
	y=0;
	for( int x =1; x < 240; x++)
	{
		ImageOut->pData[x*ImageOut->numCols +y] = ImageIn->pData[x*ImageOut->numCols +y];
	}
    y = 320-1;
	for( int x =1; x < 240; x++)
	{
		ImageOut->pData[x*ImageOut->numCols +y] = ImageIn->pData[x*ImageOut->numCols +y];
	}
}


void arm_gaussian_filter_3x3_gray_in_u8_out_u8_proc_f16(const arm_image_gray_u8_t* ImageIn, arm_image_gray_u8_t* ImageOut)//sould output the same result
{
    int w =ImageIn->numCols;
    int indice = ImageIn->numCols+1;
    for( int y = 1; y< ImageIn -> numRows- 1; y++)//put a -1 if you want to treat the entirety of the roi//-2 if you try to match OpenMV with the ligne further bellow
    {
        for( int x = 1; x< ImageIn-> numCols- 1; x++)//put a -1 if you want to treat the entirety of the roi//-2 if you try to match OpenMV with the ligne further bellow
        {
            indice = y*ImageIn->numCols+x;
            ImageOut->pData[indice] =(uint8_t)((ImageIn->pData[indice-w-1] + (ImageIn->pData[indice-w]<<1) + ImageIn->pData[indice-w+1]+(ImageIn->pData[indice - 1]<<1)+ (ImageIn->pData[indice]<<2) +(ImageIn->pData[indice + 1]<<1) + ImageIn->pData[indice +w -1] + (ImageIn->pData[ indice + w]<<1) + ImageIn->pData[indice + w+1])>>4);
        }
        //need for matching OpenMV
        //indice++;
        //ImageOut->pData[indice] = ImageIn->pData[indice];
        //
    }
    //need for matching OpenMV
    /*for(int x = 1; x< ImageIn -> numCols- 2; x++)
    {
        indice = (ImageIn->numRows- 3)*ImageIn->numCols+ x;
        ImageOut->pData[indice] = ImageIn->pData[indice];
    }
    for(int x = 1; x< ImageIn -> numCols- 1; x++)
    {
        indice = (ImageIn->numRows- 2)*ImageIn->numCols+ x;
        ImageOut->pData[indice] = ImageIn->pData[indice];
    }*/
    //
    int x = 0;
    int y = 0;
    for( int y =0; y < 320; y++)
	{
		ImageOut->pData[x*ImageOut->numCols +y] = ImageIn->pData[x*ImageOut->numCols +y];
	}
	x = 240-1;
	for( int y =0; y < 320; y++)
	{
		ImageOut->pData[x*ImageOut->numCols +y] = ImageIn->pData[x*ImageOut->numCols +y];
	}
	y = 320-1;
	for( int x =1; x < 240; x++)
	{
		ImageOut->pData[x*ImageOut->numCols +y] = ImageIn->pData[x*ImageOut->numCols +y];
	}
	y=0;
	for( int x =1; x < 240; x++)
	{
		ImageOut->pData[x*ImageOut->numCols +y] = ImageIn->pData[x*ImageOut->numCols +y];
	}
}

void arm_gaussian_filter_3x3_gray_in_u8_out_q15_proc_q15(const arm_image_gray_u8_t* ImageIn, arm_image_gray_q15_t* ImageOut)
{
    int w =ImageIn->numCols;
    int indice;
    for( int y = 1; y< ImageIn -> numRows- 1; y++)//-1 for complete coverage, -2 to match OpenMV + the lign specified below
    {
        for( int x = 1; x< ImageIn-> numCols- 1; x++)//here too -1 or -2
        {
            indice = y*ImageIn->numCols+x;
            q63_t res;
            q15_t matrix_in[9] = { ImageIn->pData[indice-w-1], ImageIn->pData[indice-w], ImageIn->pData[indice-w+1], ImageIn->pData[indice - 1], ImageIn->pData[indice], ImageIn->pData[indice + 1], ImageIn->pData[indice +w -1], ImageIn->pData[ indice + w], ImageIn->pData[indice + w+1]};
            //this dot product output on 34.30, so a shift of 15 is enought to bring us back into q15 but because we didn't convert our input Image previously, we have to do a shift to the left by 7, this is possible due to the increase of precisionn of the multiplication/dotproduct, so we only need a shif of 15-7 8
            arm_dot_prod_q15(&grad[0], &matrix_in[0], 9, &res);
            res = res>>8;
            ImageOut->pData[indice] =(q15_t)(res);
        }
        //need for matching OpenMV
        //indice++;
        //ImageOut->pData[indice] = ImageIn->pData[indice] <<7;
        
    }
    //need for matching OpenMV
    /*for(int x = 1; x< ImageIn -> numCols- 2; x++)
    {
        indice = (ImageIn->numRows- 3)*ImageIn->numCols+ x;
        ImageOut->pData[indice] = ImageIn->pData[indice] <<7;
    }
    for(int x = 1; x< ImageIn -> numCols- 1; x++)
    {
        indice = (ImageIn->numRows- 2)*ImageIn->numCols+ x;
        ImageOut->pData[indice] = ImageIn->pData[indice] <<7;
    }*/
    //
    int x = 0;
    int y = 0;
    for( int y =0; y < 320; y++)
	{
		ImageOut->pData[x*ImageOut->numCols +y] = ImageIn->pData[x*ImageOut->numCols +y] <<7;
	}
	x = 240-1;
	for( int y =0; y < 320; y++)
	{
		ImageOut->pData[x*ImageOut->numCols +y] = ImageIn->pData[x*ImageOut->numCols +y] <<7;
	}
	y=0;
	for( int x =1; x < 240; x++)
	{
		ImageOut->pData[x*ImageOut->numCols +y] = ImageIn->pData[x*ImageOut->numCols +y] <<7;
	}
    y = 320-1;
	for( int x =1; x < 240; x++)
	{
		ImageOut->pData[x*ImageOut->numCols +y] = ImageIn->pData[x*ImageOut->numCols +y] <<7;
	}
}
//Gaussian filter in float16
#if ((!defined(ARM_MATH_MVEI)) |(defined(FORCE_SCALAR)))
//scalar version of the function
//use a buffer to store the intermediate sum to limitate the repetition of calculation
void arm_gaussian_filter_3x3_gray_in_u8_out_u8_proc_f16_buff(const arm_image_gray_u8_t* ImageIn, arm_image_gray_u8_t* ImageOut, arm_image_buff_f16_t* Img_tmp)
{
    //initialisation of the two first line of the buffer because we need three line of the buffer to compute one line of the output
    //the value inside the buffer are the sum of the three pixel in a line starting from the corresponding indice of the buffer image
    //the the value in the buffer at indice 1 will value 1 + 2 + 3 of the input image 
    for( int x = 0; x <2; x++)
    {
        for(int y = 1; y<ImageIn -> numCols- 1; y++)
        {
            Img_tmp->pData[x*Img_tmp->numCols +y-1] = (ImageIn->pData[x*Img_tmp->numCols +y-1] + (ImageIn->pData[x*Img_tmp->numCols +y]*2) + ImageIn->pData[x*Img_tmp->numCols +y+1]);
        }
    }
    //main loop, computing one value of intermediate sum then the output value unlock by this value
    for( int x =2; x<ImageIn-> numRows; x++)
    {
        //indice of x inside the buffer because we only have three line in the buffer
        int xm = x%3;
        //computation of the first value of the line for the buffer and the output image
        //The border remain unchanged so the output is just a copy of the input
        Img_tmp->pData[xm*Img_tmp->numCols +0] = (ImageIn->pData[x*Img_tmp->numCols +0] + (ImageIn->pData[x*Img_tmp->numCols +1]*2) + ImageIn->pData[x*Img_tmp->numCols +2]);
        ImageOut->pData[(x-1)*Img_tmp->numCols] = ImageIn->pData[(x-1)*Img_tmp->numCols];
        //core of main loop computing a value of buffer and a value of output
        for(int y = 2; y<ImageIn -> numCols; y++)
        {
            Img_tmp->pData[xm*Img_tmp->numCols +y-1] = (ImageIn->pData[x*Img_tmp->numCols +y-1] + (ImageIn->pData[x*Img_tmp->numCols +y]*2) + ImageIn->pData[x*Img_tmp->numCols +y+1]);
            ImageOut->pData[(x-1)*Img_tmp->numCols +y-1] = (uint8_t)(((float)Img_tmp->pData[(x%3)*Img_tmp->numCols +y-2] + ((float)Img_tmp->pData[((x-1)%3)*Img_tmp->numCols +y-2]*2) + (float)Img_tmp->pData[((x-2)%3)*Img_tmp->numCols +y-2])/16);
        }
        //last value of the column is a simple copy
        ImageOut->pData[(x-1)*Img_tmp->numCols +ImageIn -> numCols-1] = ImageIn->pData[(x-1)*Img_tmp->numCols+ImageIn -> numCols-1];
    }
    //the last line is a copy
	int x = 239;
	for( int y =1; y < ImageIn->numCols-1; y++)
	{
		ImageOut->pData[x*ImageOut->numCols +y] = ImageIn->pData[x*ImageOut->numCols +y];
	}
    //first line is a copy too
	x=0;
	for( int y =1; y < ImageIn->numCols-1; y++)
	{
		ImageOut->pData[x*ImageOut->numCols +y] = ImageIn->pData[x*ImageOut->numCols +y];
	}
}
#else
//vector version of the function
void arm_gaussian_filter_3x3_gray_in_u8_out_u8_proc_f16_buff(const arm_image_gray_u8_t* ImageIn, arm_image_gray_u8_t* ImageOut, arm_image_buff_f16_t* Img_tmp)
{
    int w = Img_tmp->numCols;
    //determinig the tail number that will need to be treated after the vector part
    int numtail = (ImageIn->numCols-2-1)&15;
    // computing the first two lines of the buffer
    for( int x = 0; x <2; x++)
    {
        //we don't need the first value of the line so we can start at 1, 
        //we do not use tail predication to treat the end of the loop because even if we overflow, the last colomn that we are not suppose to compute will never be used and the next line wil be computed before usage so ther's no issue in writting in it
        for(int y = 1; y<ImageIn -> numCols- 1; y+=16)
        {
            uint16x8x2_t vect_2x2;
            uint16x8x2_t vect_1x2;
            uint16x8x2_t vect_3x2;
            float16x8x2_t vect_f_2x2;
            float16x8x2_t vect_f_1x2;
            float16x8x2_t vect_f_3x2;
            uint8x16_t vect_1 = vld1q(&ImageIn->pData[x*Img_tmp->numCols+y-1]);
            //we need to do this shift to keep precision but it cause us to split 
            //the vector into two vector cintaining bigger data type
            vect_1x2.val[0] = vshllbq(vect_1,1);
            vect_f_1x2.val[0] = vcvtq(vect_1x2.val[0]);
            uint8x16_t vect_3 = vld1q(&ImageIn->pData[x*Img_tmp->numCols+y+1]);
            vect_3x2.val[0] = vshllbq(vect_3,1);
            vect_f_3x2.val[0] = vcvtq(vect_3x2.val[0]);
            uint8x16_t vect_2 = vld1q(&ImageIn->pData[x*Img_tmp->numCols+y]);
            vect_f_1x2.val[0] = vaddq(vect_f_1x2.val[0], vect_f_3x2.val[0]);
            vect_2x2.val[0] = vshllbq(vect_2,2); 
            vect_f_2x2.val[0] = vcvtq(vect_2x2.val[0]);
            vect_f_2x2.val[0] = vaddq(vect_f_2x2.val[0], vect_f_1x2.val[0]);
            
            vect_1x2.val[1] = vshlltq(vect_1,1);
            vect_f_1x2.val[1] = vcvtq(vect_1x2.val[1]);
            vect_f_2x2.val[0] = vmulq_n_f16( vect_f_2x2.val[0],0.5);
            vect_3x2.val[1] = vshlltq(vect_3,1);
            vect_f_3x2.val[1] = vcvtq(vect_3x2.val[1]);
            vect_f_1x2.val[1] = vaddq(vect_f_1x2.val[1], vect_f_3x2.val[1]);
            vect_2x2.val[1] = vshlltq(vect_2,2);
            vect_f_2x2.val[1] = vcvtq(vect_2x2.val[1]);
            
            vect_f_2x2.val[1] = vaddq(vect_f_2x2.val[1], vect_f_1x2.val[1]);
            vect_f_2x2.val[1] = vmulq_n_f16( vect_f_2x2.val[1],0.5);

            vst2q(&Img_tmp->pData[x*Img_tmp->numCols +y-1], vect_f_2x2);
        }
    }
    //
    for( int x =2; x<ImageIn-> numRows; x++)
    {
        int xm = x%3;
        Img_tmp->pData[xm*Img_tmp->numCols +0] = (ImageIn->pData[x*Img_tmp->numCols +0] + (ImageIn->pData[x*Img_tmp->numCols +1]*2) + ImageIn->pData[x*Img_tmp->numCols +2]);
        Img_tmp->pData[xm*Img_tmp->numCols +1] = (ImageIn->pData[x*Img_tmp->numCols +1] + (ImageIn->pData[x*Img_tmp->numCols +2]*2) + ImageIn->pData[x*Img_tmp->numCols +3]);
        ImageOut->pData[(x-1)*Img_tmp->numCols] = ImageIn->pData[(x-1)*Img_tmp->numCols];
        for(int y = 2; y<ImageIn -> numCols-numtail-1; y+=16)
        {   
            int indice = x*w+y;
            int indicem = xm*w+y;         
            uint16x8x2_t vect_2x2;
            uint16x8x2_t vect_1x2;
            uint16x8x2_t vect_3x2;
            float16x8x2_t vect_f_2x2;
            float16x8x2_t vect_f_1x2;
            float16x8x2_t vect_f_3x2;
            float16x8x2_t vect_2_f;
            float16x8x2_t vect_3_f;
            uint8x16_t vect_u8;
            uint8x16_t vect_1 = vld1q(&ImageIn->pData[indice-2]);
            vect_1x2.val[0] = vshllbq(vect_1,1);
            vect_f_1x2.val[0] = vcvtq(vect_1x2.val[0]);
            
            uint8x16_t vect_3 = vld1q(&ImageIn->pData[x*Img_tmp->numCols+y]);
            vect_3x2.val[0] = vshllbq(vect_3,1);
            vect_f_3x2.val[0] = vcvtq(vect_3x2.val[0]);
            vect_f_1x2.val[0] = vaddq(vect_f_1x2.val[0], vect_f_3x2.val[0]);
            vect_1x2.val[1] = vshlltq(vect_1,1);
            vect_f_1x2.val[1] = vcvtq(vect_1x2.val[1]);
            uint8x16_t vect_2 = vld1q(&ImageIn->pData[x*Img_tmp->numCols+y-1]);
            vect_2x2.val[0] = vshllbq(vect_2,2); 
            vect_f_2x2.val[0] = vcvtq(vect_2x2.val[0]);
            
            vect_f_2x2.val[0] = vaddq(vect_f_2x2.val[0], vect_f_1x2.val[0]);
            vect_3x2.val[1] = vshlltq(vect_3,1);
            vect_f_3x2.val[1] = vcvtq(vect_3x2.val[1]);
            
            vect_f_1x2.val[1] = vaddq(vect_f_1x2.val[1], vect_f_3x2.val[1]);
            vect_2x2.val[1] = vshlltq(vect_2,2);
            vect_f_2x2.val[1] = vcvtq(vect_2x2.val[1]);
            vect_f_2x2.val[0] = vmulq_n_f16( vect_f_2x2.val[0],0.5);
            vect_f_2x2.val[1] = vaddq(vect_f_2x2.val[1], vect_f_1x2.val[1]);
            vect_f_2x2.val[1] = vmulq_n_f16( vect_f_2x2.val[1],0.5);
            vect_2_f= vld2q(&Img_tmp->pData[((x-1)%3)*Img_tmp->numCols +y-2]);
            vect_2_f.val[0]= vmulq_n_f16(vect_2_f.val[0], 2);
            vect_3_f= vld2q(&Img_tmp->pData[((x-2)%3)*Img_tmp->numCols +y-2]);
            vect_3_f.val[0]= vaddq(vect_f_2x2.val[0], vect_3_f.val[0]);
            //i use vst2 in order to have the desinterlive value because i store the value using a method that interlive them
            vst2q(&Img_tmp->pData[indicem-2], vect_f_2x2);
            //So the issue is to do the compute in float and then store in uint8
            //to decrease the precision I use the store narrow
            vect_3_f.val[0]= vaddq(vect_3_f.val[0], vect_2_f.val[0]);
            vect_3_f.val[0]= vmulq_n_f16(vect_3_f.val[0], 0.0625);
            //second part of the compute
            vect_3_f.val[1]= vaddq(vect_f_2x2.val[1], vect_3_f.val[1]);
            //I convert my value float in uint
            uint16x8_t vect_1_u16 = vcvtaq_u16_f16(vect_3_f.val[0]);
            vect_2_f.val[1]= vmulq_n_f16(vect_2_f.val[1], 2);
            //here vect_2_f contain the upper part of the uint 8 vector i need to compute
            vect_3_f.val[1]= vaddq(vect_3_f.val[1], vect_2_f.val[1]);
            //reinterpret the uint16 as an 8, so the upper part of the u8 vector is correct, the bottom is empty
            vect_u8 = vreinterpretq_u8( vect_1_u16);
            vect_3_f.val[1]= vmulq_n_f16(vect_3_f.val[1], 0.0625);
            //here the uint16 contain the low part of the u8 needed
            //cenversion from f16 to u16
            vect_1_u16 = vcvtaq_u16_f16(vect_3_f.val[1]);
            //operation store narrow, the value from the uint8 are putted in the upper part of the u16
            //the u8 contained the upper part and in this place, the u16 was empty because the result of the compute is scaled to u8, so the u16 contain u8 value so the upper part is empty
            vect_u8 = vmovntq( vect_u8,vect_1_u16);
            //now the u16 contain in each value two value u8, on the bottom and upper part for each value
            //Because I am attributing the output value to the u8 vector I don't have to convert it further it's already all good i think
            vst1q(&ImageOut->pData[indice-w-1], vect_u8);
        }
        //need to think about it but maybe it can be acceptable to treat the tail the same as the body since the overflow will go on the next line that will be written after
        
        
        //first part of the tail, if it is possible to trreat it with a vector smaller than the body but still faster than
        //
        if(numtail>7)
        {
            int y = ImageIn->numCols-numtail-1;
            int indice = x*w+y;
            int indicem = xm*w+y;
            //reading only 8 value
            uint16x8_t vect_u16_2 = vldrbq_u16(&ImageIn->pData[indice-1]);
            float16x8_t vect_2 = vcvtq(vect_u16_2);
            //no overflow because the load have widden the vector
            vect_2 = vmulq_n_f16(vect_2, 2);
            uint16x8_t vect_u16_1 = vldrbq_u16(&ImageIn->pData[indice-2]);
            float16x8_t vect_1 = vcvtq(vect_u16_1);
            vect_2 = vaddq(vect_1, vect_2);
            uint16x8_t vect_u16_3 = vldrbq_u16(&ImageIn->pData[indice+0]);
            float16x8_t vect_3 = vcvtq(vect_u16_3);
            vect_2 = vaddq(vect_2, vect_3);
            vst1q(&Img_tmp->pData[indicem-2], vect_2);
            //second part of the compute of the partical sum
            //in vector but with tail predication to ensure no issue will be raise
            y +=8;
            int remains = numtail -7;
            indice +=8;
            indicem +=8;
            vect_u16_2 = vldrbq_u16(&ImageIn->pData[indice-1]);
            vect_2 = vcvtq(vect_u16_2);
            vect_2 = vmulq_n_f16(vect_2, 2);
            vect_u16_1 = vldrbq_u16(&ImageIn->pData[indice-2]);
            vect_1 = vcvtq(vect_u16_1);
            mve_pred16_t vpred = vctp16q(remains);
            vect_2 = vaddq(vect_1, vect_2);
            vect_u16_3 = vldrbq_u16(&ImageIn->pData[indice]);
            vect_3 = vcvtq(vect_u16_3);
            vect_2 = vaddq(vect_2, vect_3);
            vstrhq_p(&Img_tmp->pData[indicem-2], vect_2, vpred);
            //second part of the compte now that the partial sum is computed for all the 
            //tail, I can now compute the output value associated
            //first part in vector and it seems that I didn't care about writting 
            //overflow in the next line so the question of the validity of the tail is still up
            indice -=8;
            y -=8;
            float16x8x2_t vect_2_f;
            float16x8x2_t vect_3_f;
            uint8x16_t vect_u8;
            float16x8x2_t vect_1_f;
            vect_1_f= vld2q(&Img_tmp->pData[(x%3)*Img_tmp->numCols +y-2]);
            vect_2_f= vld2q(&Img_tmp->pData[((x-1)%3)*Img_tmp->numCols +y-2]);
            vect_2_f.val[0]= vmulq_n_f16(vect_2_f.val[0], 2);
            vect_3_f= vld2q(&Img_tmp->pData[((x-2)%3)*Img_tmp->numCols +y-2]);
            vect_1_f.val[0]= vaddq(vect_1_f.val[0], vect_3_f.val[0]);
            vect_1_f.val[0]= vaddq(vect_1_f.val[0], vect_2_f.val[0]);
            vect_1_f.val[0]= vmulq_n_f16(vect_1_f.val[0], 0.0625);
            uint16x8_t vect_1_u16 = vcvtaq_u16_f16(vect_1_f.val[0]);
            vect_u8 = vreinterpretq_u8( vect_1_u16);
            vect_1_f.val[1]= vaddq(vect_1_f.val[1], vect_3_f.val[1]);
            vect_2_f.val[1]= vmulq_n_f16(vect_2_f.val[1], 2);
            vect_1_f.val[1]= vaddq(vect_1_f.val[1], vect_2_f.val[1]);
            vect_1_f.val[1]= vmulq_n_f16(vect_1_f.val[1], 0.0625);
            vect_1_u16 = vcvtaq_u16_f16(vect_1_f.val[1]);
            vect_u8 = vmovntq( vect_u8,vect_1_u16);
            vst1q(&ImageOut->pData[indice-w-1], vect_u8);
        }
        //second part of the tail this time in the case the tail we need to compute is less than 8
        //Tail predication for the temporary sum but still no tp for the output
        //considering that a big enought overflow will it outdated valu in the buffer or worse outside of the buffe
        //there is the possibility of an issue here i think probabli doesn't occur because I have a tail of 7 so overflow doesn't hit outside of buffer
        else
        {
            int y = ImageIn->numCols-numtail-1;
            int indice = x*w+y;
            int indicem = xm*w+y;
            int remains = numtail +1;
            uint16x8_t vect_u16_2 = vldrbq_u16(&ImageIn->pData[indice-1]);
            float16x8_t vect_2 = vcvtq(vect_u16_2);
            vect_2 = vmulq_n_f16(vect_2, 2);
            uint16x8_t vect_u16_1 = vldrbq_u16(&ImageIn->pData[indice-2]);
            float16x8_t vect_1 = vcvtq(vect_u16_1);
            mve_pred16_t vpred = vctp16q(remains);
            vect_2 = vaddq(vect_1, vect_2);
            uint16x8_t vect_u16_3 = vldrbq_u16(&ImageIn->pData[indice]);
            float16x8_t vect_3 = vcvtq(vect_u16_3);
            vect_2 = vaddq(vect_2, vect_3);
            vstrhq_p(&Img_tmp->pData[indicem-2], vect_2, vpred);

            float16x8x2_t vect_2_f;
            float16x8x2_t vect_3_f;
            uint8x16_t vect_u8;
            float16x8x2_t vect_1_f;
            vect_2_f= vld2q(&Img_tmp->pData[((x-1)%3)*Img_tmp->numCols +y-2]);
            vect_2_f.val[0]= vmulq_n_f16(vect_2_f.val[0], 2);
            vect_3_f= vld2q(&Img_tmp->pData[((x-2)%3)*Img_tmp->numCols +y-2]);
            vect_1_f.val[0]= vaddq(vect_1_f.val[0], vect_3_f.val[0]);
            vect_1_f.val[0]= vaddq(vect_1_f.val[0], vect_2_f.val[0]);
            vect_1_f.val[0]= vmulq_n_f16(vect_1_f.val[0], 0.0625);
            uint16x8_t vect_1_u16 = vcvtaq_u16_f16(vect_1_f.val[0]);
            vect_u8 = vreinterpretq_u8( vect_1_u16);
            vect_1_f.val[1]= vaddq(vect_1_f.val[1], vect_3_f.val[1]);
            vect_2_f.val[1]= vmulq_n_f16(vect_2_f.val[1], 2);
            vect_1_f.val[1]= vaddq(vect_1_f.val[1], vect_2_f.val[1]);
            vect_1_f.val[1]= vmulq_n_f16(vect_1_f.val[1], 0.0625);
            vect_1_u16 = vcvtaq_u16_f16(vect_1_f.val[1]);
            vect_u8 = vmovntq( vect_u8,vect_1_u16);
            vst1q(&ImageOut->pData[indice-w-1], vect_u8);
        }
        ImageOut->pData[(x-1)*Img_tmp->numCols +ImageIn -> numCols-1] = ImageIn->pData[(x-1)*Img_tmp->numCols+ImageIn -> numCols-1];
    }
	int x = 239;
    int numtail8 = ImageIn->numCols%16;
	for( int y =0; y < ((ImageIn->numCols>>4)<<4)-2; y+=16)
	{
        uint8x16_t vec_copy = vldrbq_u8(&ImageIn->pData[x*ImageOut->numCols +y]);
        vst1q(&ImageOut->pData[x*ImageOut->numCols +y], vec_copy);
	}
    //tail
    if(numtail8>0)
    {
        for(int y = ((ImageIn->numCols>>4)<<4)-2; y<ImageIn->numCols;y++)
        {
            ImageOut->pData[x*ImageOut->numCols +y] = ImageIn->pData[x*ImageOut->numCols +y];
        }
    }
	x=0;
	for( int y =0; y < (ImageIn->numCols>>4)<<4; y+=16)
	{
        uint8x16_t vec_copy = vldrbq_u8(&ImageIn->pData[x*ImageOut->numCols +y]);
        vst1q(&ImageOut->pData[x*ImageOut->numCols +y], vec_copy);
	}
    //tail
    if(numtail8>0)
    {
        for(int y = (ImageIn->numCols>>4)<<4; y<ImageIn->numCols;y++)
        {
            ImageOut->pData[x*ImageOut->numCols +y] = ImageIn->pData[x*ImageOut->numCols +y];
        }
    }
}
#endif
//gaussian fixed point q15
#if ((!defined(ARM_MATH_MVEI)) |(defined(FORCE_SCALAR)))
//scalar version
void arm_gaussian_filter_3x3_gray_in_u8_out_q15_proc_q15_buff(const arm_image_gray_u8_t* ImageIn, arm_image_gray_q15_t* ImageOut, arm_image_gray_q15_t* Img_tmp)
{
    //first two lines of the partial sum buffer
    for( int x = 0; x <2; x++)
    {
        for(int y = 1; y<ImageIn -> numCols- 1; y++)
        {
            Img_tmp->pData[x*Img_tmp->numCols +y-1] = (ImageIn->pData[x*Img_tmp->numCols +y-1]*0x1000 + (ImageIn->pData[x*Img_tmp->numCols +y]*0x2000) + ImageIn->pData[x*Img_tmp->numCols +y+1]*0x1000)>>12;
        }
    }
    //main loop
    //calculating one line for the buffer and the output image per loop turn
    for( int x =2; x<ImageIn-> numRows; x++)
    {
        int xm = x&3;
        //first two values of the buffer are treated separatly as initialization
        Img_tmp->pData[xm*Img_tmp->numCols +0] = (ImageIn->pData[x*Img_tmp->numCols +0]*0x01000 + (ImageIn->pData[x*Img_tmp->numCols +1]*0x2000) + ImageIn->pData[x*Img_tmp->numCols +2]*0x01000)>>12;
        Img_tmp->pData[xm*Img_tmp->numCols +1] = (ImageIn->pData[x*Img_tmp->numCols +1]*0x01000 + (ImageIn->pData[x*Img_tmp->numCols +2]*0x2000) + ImageIn->pData[x*Img_tmp->numCols +3]*0x01000)>>12;
        //set the first value of the line to zero for the output image
        ImageOut->pData[(x-1)*Img_tmp->numCols] = ImageIn->pData[(x-1)*Img_tmp->numCols]<<7;
        for(int y = 2; y<ImageIn -> numCols; y++)
        {
            //compute on value for the buffer and one value for the output image
            //the value for the output image is one line upper than the one for the buffer because 
            //we need a 3x3 area center around a value to compute it's output value
            //so we need the buffer line under to be done
            Img_tmp->pData[xm*Img_tmp->numCols +y-1] = (ImageIn->pData[x*Img_tmp->numCols +y-1]*0x1000 + (ImageIn->pData[x*Img_tmp->numCols +y]*0x2000) + ImageIn->pData[x*Img_tmp->numCols +y+1]*0x1000)>>12;
            ImageOut->pData[(x-1)*Img_tmp->numCols +y-1] = (((Img_tmp->pData[xm*Img_tmp->numCols +y-2]) + (Img_tmp->pData[((x-1)&3)*Img_tmp->numCols +y-2]<<1) + (Img_tmp->pData[((x-2)&3)*Img_tmp->numCols +y-2]))<<3);
        }
        ImageOut->pData[(x-1)*Img_tmp->numCols +ImageIn -> numCols-1] = ImageIn->pData[(x-1)*Img_tmp->numCols+ImageIn -> numCols-1]<<7;
    }
    //ensure the first and the last lined are set to 0
    int x = 239;
	for( int y =0; y < ImageIn->numCols; y++)
	{
		ImageOut->pData[x*ImageOut->numCols +y] = ImageIn->pData[x*ImageOut->numCols +y]<<7;
	}
	x=0;
	for( int y =0; y < ImageIn->numCols; y++)
	{
		ImageOut->pData[x*ImageOut->numCols +y] = ImageIn->pData[x*ImageOut->numCols +y]<<7;
	}
}
#else
//vector version
void arm_gaussian_filter_3x3_gray_in_u8_out_q15_proc_q15_buff(const arm_image_gray_u8_t* ImageIn, arm_image_gray_q15_t* ImageOut, arm_image_gray_q15_t* Img_tmp)
{   
    //To obtain the tail item number, we use the max value reacheable, numCols-1, and remove 2 because the first and last value of each lane is treated separatly
    int w = Img_tmp->numCols;
    int numtail = (ImageIn->numCols-2-1)&15;
    //initialization by computing thefirst two line of the buffer
    for( int x = 0; x <2; x++)
    {
        //for this the tail in't necessary I think because 
        // not doing it will write on the next lign of the buffer, that we will writte after, so we can just avoid the tail predication 
        for(int y = 1; y< ImageIn -> numCols-/*((ImageIn->numCols-1-1)&15)*/-1; y+=16)
        {
            //we load three vector with an offset of 1 between their values
            //we scale them in order to retain precision, sum then whith the middle ine multiply by two
            //and then rescale them
            uint16x8x2_t vect_2x2;
            uint16x8x2_t vect_1x2;
            uint16x8x2_t vect_3x2;
            uint8x16_t vect_1 = vld1q(&ImageIn->pData[x*Img_tmp->numCols+y-1]);
            vect_1x2.val[0] = vshllbq(vect_1,1);
            uint8x16_t vect_3 = vld1q(&ImageIn->pData[x*Img_tmp->numCols+y+1]);
            vect_3x2.val[0] = vshllbq(vect_3,1);
            vect_1x2.val[0] = vaddq(vect_1x2.val[0], vect_3x2.val[0]);
            vect_1x2.val[1] = vshlltq(vect_1,1);
            uint8x16_t vect_2 = vld1q(&ImageIn->pData[x*Img_tmp->numCols+y]);
            vect_3x2.val[1] = vshlltq(vect_3,1);
            vect_1x2.val[1] = vaddq(vect_1x2.val[1], vect_3x2.val[1]);
            vect_2x2.val[0] = vshllbq(vect_2,2);
            vect_2x2.val[0] = vaddq(vect_2x2.val[0], vect_1x2.val[0]);
            vect_2x2.val[0] = vshrq(vect_2x2.val[0], 1);
            vect_2x2.val[1] = vshlltq(vect_2,2);
            vect_2x2.val[1] = vaddq(vect_2x2.val[1], vect_1x2.val[1]);
            vect_2x2.val[1] = vshrq(vect_2x2.val[1], 1);
            vst2q((uint16_t*)&Img_tmp->pData[x*Img_tmp->numCols +y-1], vect_2x2);
        }
        //here is the tail part....
        /*for(int remains = -1; remains < numtail; remains++)
        {
            int y;
            y = remains+ImageIn->numCols-numtail;
            Img_tmp->pData[x*Img_tmp->numCols +y-2] = (ImageIn->pData[x*Img_tmp->numCols +y-2]*0x1000 + (ImageIn->pData[x*Img_tmp->numCols +y-1]*0x2000) + ImageIn->pData[x*Img_tmp->numCols +y]*0x1000)>>12;//compute element in x y //out
        }*/
    }
    //main loop
    for( int x =2; x<ImageIn-> numRows; x++)
    {
        int xm = x%3;
        //initialization by computing the first value of the buffer and copying the first input value in output
        Img_tmp->pData[xm*Img_tmp->numCols +0] = (ImageIn->pData[x*Img_tmp->numCols +0]*0x01000 + (ImageIn->pData[x*Img_tmp->numCols +1]*0x2000) + ImageIn->pData[x*Img_tmp->numCols +2]*0x01000)>>12;//stay bc 2 not vect
        ImageOut->pData[(x-1)*Img_tmp->numCols] = ImageIn->pData[(x-1)*Img_tmp->numCols]<<7;
        //main process for a line
        //Computationg of a vector of the buffer then a vector of the output
        //for the buffer we do the samw steps as before and for the output we reuse the previously computed line of buffer plus
        //we read the to other required vector in the buffer, scale the vector that requiere it and sum the trhee vector
        //then we just need to shift it in order to be properly in q15
        //note the vector of the buffer all recieve a scaling to keep precision, can be possible if we want maximun of performance to not make this shift
        //and just to sum them scale by the correct factor
        for(int y = 2; y<ImageIn -> numCols-numtail-1; y+=16)
        {
            uint16x8x2_t vect_2x2;
            uint16x8x2_t vect_1x2;
            uint16x8x2_t vect_3x2;
            q15x8x2_t vect_2q;
            q15x8x2_t vect_3q;
            int indice = x*w+y;
            int indicem = xm*w+y;
            uint8x16_t vect_1 = vld1q(&ImageIn->pData[indice-2]);
            vect_1x2.val[0] = vshllbq(vect_1,1);
            uint8x16_t vect_3 = vld1q(&ImageIn->pData[indice]);
            vect_3x2.val[0] = vshllbq(vect_3,1);
            uint8x16_t vect_2 = vld1q(&ImageIn->pData[indice-1]);
            vect_1x2.val[1] = vshlltq(vect_1,1);
            vect_1x2.val[0] = vaddq(vect_1x2.val[0], vect_3x2.val[0]);
            vect_2x2.val[0] = vshllbq(vect_2,2);
            vect_2x2.val[0] = vaddq(vect_2x2.val[0], vect_1x2.val[0]);
            vect_3x2.val[1] = vshlltq(vect_3,1);
            vect_1x2.val[1] = vaddq(vect_1x2.val[1], vect_3x2.val[1]);
            vect_2x2.val[1] = vshlltq(vect_2,2);
            vect_2x2.val[1] = vaddq(vect_2x2.val[1], vect_1x2.val[1]);
            vect_2x2.val[0] = vshrq(vect_2x2.val[0], 1);
            vect_2q = vld2q(&Img_tmp->pData[((x-1)%3)*w +y-2]);
            vect_2x2.val[1] = vshrq(vect_2x2.val[1], 1);
            vect_2q.val[0] = vshlq_n_s16(vect_2q.val[0], 1);
            vect_3q = vld2q(&Img_tmp->pData[((x-2)%3)*w +y-2]);
            vect_2q.val[0] = vaddq_s16(vect_2x2.val[0], vect_2q.val[0]);
            vect_2q.val[1] = vshlq_n_s16(vect_2q.val[1], 1);
            vect_2q.val[0] = vaddq_s16(vect_2q.val[0], vect_3q.val[0]);
            vect_2q.val[0] = vshlq_n_s16(vect_2q.val[0], 3);
            vect_2q.val[1] = vaddq_s16(vect_2x2.val[1], vect_2q.val[1]);
            vst2q((uint16_t*)&Img_tmp->pData[indicem-2], vect_2x2);
            vect_2q.val[1] = vaddq_s16(vect_2q.val[1], vect_3q.val[1]);
            vect_2q.val[1] = vshlq_n_s16(vect_2q.val[1], 3);
            vst2q(&ImageOut->pData[indice-w-1], vect_2q);
        }
        //Tail, if the tail is longer than 8, we need to do two vector of uint16x8, we do not care about overwritting
        //because it will write on the next line that will be writen juste after
        //maybe for performace it can be interesting to determine a thershold for doing the tail in scalar 
        // If we don't care about tp, it means that we can remove this and do one more turn on the previous loop but I am not sure it's the case
        //in the case we were on the last line of the buffer(in fact on all the thre cases), if we try to read one value outside of the buffer it is 
        //undefined, even if this value is written in a place where we do not care it may cause some issue, like if the buffer is at the end of the memory we have the right to access
        //in the case we do the main part of both then the tail of both together we can ignore the tail
        if(numtail > 7)
        {
            int y = ImageIn->numCols-numtail-1;
            int indice = x*w+y;
            int indicem = xm*w+y;
            uint16x8_t vect_2 = vldrbq_u16(&ImageIn->pData[indice-1]);
            vect_2 = vqshlq_n_s16(vect_2, 1);
            uint16x8_t vect_1 = vldrbq_u16(&ImageIn->pData[indice-2]);
             vect_2 = vaddq(vect_1, vect_2);
            uint16x8_t vect_3 = vldrbq_u16(&ImageIn->pData[indice]);
            vect_2 = vaddq(vect_2, vect_3);
            uint16x8_t vect_1q = vld1q(&Img_tmp->pData[((x-1)%3)*w +y-2]);
            vect_1q = vqshlq_n_s16(vect_1q, 1);
            uint16x8_t vect_3q = vld1q(&Img_tmp->pData[((x-2)%3)*w +y-2]);
            vect_1q = vaddq(vect_1q, vect_2);
            vst1q((uint16_t*)&Img_tmp->pData[indicem-2], vect_2);
            vect_1q = vaddq(vect_1q, vect_3q);
            vect_1q = vqshlq_n_s16(vect_1q, 3);
            vst1q((uint16_t*)&ImageOut->pData[indice-w-1],vect_1q);
            y +=8;
            int remains = numtail -7;
            indice +=8;
            indicem +=8;
            vect_2 = vldrbq_u16(&ImageIn->pData[indice-1]);
            vect_2 = vqshlq_n_s16(vect_2, 1);
            vect_1 = vldrbq_u16(&ImageIn->pData[indice-2]);
            mve_pred16_t vpred = vctp16q(remains);
            vect_2 = vaddq(vect_1, vect_2);
            vect_3 = vldrbq_u16(&ImageIn->pData[indice]);
            vect_2 = vaddq(vect_2, vect_3);
            vect_1q = vld1q(&Img_tmp->pData[((x-1)%3)*w +y-2]);
            vect_1q = vqshlq_n_s16(vect_1q, 1);
            vect_3q = vld1q(&Img_tmp->pData[((x-2)%3)*w +y-2]);
            vect_1q = vaddq(vect_1q, vect_2);
            vstrhq_p((uint16_t*)&Img_tmp->pData[indicem-2], vect_2, vpred);
            vect_1q = vaddq(vect_1q, vect_3q);
            vect_1q = vqshlq_n_s16(vect_1q, 3);
            vst1q((uint16_t*)&ImageOut->pData[indice-w-1],vect_1q);
        }
        else
        {
            int y = ImageIn->numCols-numtail-1;
            int remains = numtail -7;
            int indice = x*w+y;
            int indicem = xm*w+y;
            uint16x8_t vect_2 = vldrbq_u16(&ImageIn->pData[indice-1]);
            vect_2 = vqshlq_n_s16(vect_2, 1);
            uint16x8_t vect_1 = vldrbq_u16(&ImageIn->pData[indice-2]);
            mve_pred16_t vpred = vctp16q(remains);
            vect_2 = vaddq(vect_1, vect_2);
            uint16x8_t vect_3 = vldrbq_u16(&ImageIn->pData[indice]);
            vect_2 = vaddq(vect_2, vect_3);
            uint16x8_t vect_1q = vld1q(&Img_tmp->pData[((x-1)%3)*w +y-2]);
            vect_1q = vqshlq_n_s16(vect_1q, 1);
            uint16x8_t vect_3q = vld1q(&Img_tmp->pData[((x-2)%3)*w +y-2]);
            vect_1q = vaddq(vect_1q, vect_2);
            vstrhq_p((uint16_t*)&Img_tmp->pData[indicem-2], vect_2, vpred);
            vect_1q = vaddq(vect_1q, vect_3q);
            vect_1q = vqshlq_n_s16(vect_1q, 3);
            vst1q((uint16_t*)&ImageOut->pData[indice-w-1],vect_1q);
        }
        ImageOut->pData[(x-1)*Img_tmp->numCols +ImageIn -> numCols-1] = ImageIn->pData[(x-1)*Img_tmp->numCols+ImageIn -> numCols-1]<<7;
    }
    int x = 239;
    for(int y = 0; y<ImageIn->numCols;y++)
    {
        ImageOut->pData[x*ImageOut->numCols +y] = ImageIn->pData[x*ImageOut->numCols +y]<<7;
    }
	x=0;
    for(int y = 0; y<ImageIn->numCols;y++)
    {
        ImageOut->pData[x*ImageOut->numCols +y] = ImageIn->pData[x*ImageOut->numCols +y]<<7;
    }
}
#endif
