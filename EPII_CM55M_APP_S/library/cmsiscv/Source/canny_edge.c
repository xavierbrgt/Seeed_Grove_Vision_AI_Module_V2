#include "canny_edge.h"
//Thining function for q15
void arm_thining_angle_in_out_q15_u8_proc_q15(const arm_image_sobel_q15_t* ImageIn, arm_image_u8_t* ImageWeakStrong)
{
    //two value of threshold for decision on pixels, hard coded but should be passed as argument
    int low_threshold = (int)(2528);//0.3*255<<5
    int high_threshold = (int)(5728);//0.7*255<<5  //5664
    //Loop to run thought all the image
    for( int x = 0; x < ImageIn->numRows; x++)
    {
        for( int y =0; y < ImageIn->numCols; y++)
        {
            int indice = x*ImageIn->numCols+y;
            int w = ImageIn->numCols;
            if(ImageIn->pSobelPixel[indice].mag !=0)
            {
                int valuein = ImageIn->pSobelPixel[indice].mag;
                //the angle allow us to determine what are the two magnitude value we need to first compare with
                //then we make a decision on keeping the pixel base on the threshold, if the magnitude value supass the high threshold we keep it
                //else we need to check the 3x3 suqare around the value to see if its an edge or not
                switch (ImageIn->pSobelPixel[indice].angle)
                {
                case Horizontal:
                    if( valuein <= ImageIn->pSobelPixel[indice-1].mag || valuein <= ImageIn->pSobelPixel[indice+1].mag)
                    {
                        ImageWeakStrong->pData[indice] = Low;
                    }
                    else
                    {
                        if( ImageIn->pSobelPixel[indice].mag < low_threshold)
                        {
                            ImageWeakStrong->pData[indice] = Low;
                        }
                        else if(ImageIn->pSobelPixel[indice].mag < high_threshold)
                        {
                            ImageWeakStrong->pData[indice] = Weak;
                        }
                        else
                        {
                            ImageWeakStrong->pData[indice] = Strong;
                        }
                    }
                    break;
                case Diagonal_rl:
                    if( valuein <= ImageIn->pSobelPixel[indice+w-1].mag || valuein <= ImageIn->pSobelPixel[indice-w+1].mag)
                    {
                        ImageWeakStrong->pData[indice] = Low;
                    }
                    else
                    {
                        if( ImageIn->pSobelPixel[indice].mag < low_threshold)
                        {
                            ImageWeakStrong->pData[indice] = Low;
                        }
                        else if(ImageIn->pSobelPixel[indice].mag < high_threshold)
                        {
                            ImageWeakStrong->pData[indice] = Weak;
                        }
                        else
                        {
                            ImageWeakStrong->pData[indice] = Strong;
                        }
                    }
                    break;
                case Vertical:
                    if(  0 <= ImageIn->pSobelPixel[indice-w].mag - valuein||  0 <= ImageIn->pSobelPixel[indice+w].mag - valuein)
                    {
                        ImageWeakStrong->pData[indice] = Low;
                    }
                    else
                    {
                        if( ImageIn->pSobelPixel[indice].mag < low_threshold)
                        {
                            ImageWeakStrong->pData[indice] = Low;
                        }
                        else if(ImageIn->pSobelPixel[indice].mag < high_threshold)
                        {
                            ImageWeakStrong->pData[indice] = Weak;
                        }
                        else
                        {
                            ImageWeakStrong->pData[indice] = Strong;
                        }
                    }
                    break;
                case Diagonal_lr:
                    if( valuein <= ImageIn->pSobelPixel[indice-w-1].mag || valuein <= ImageIn->pSobelPixel[indice+w+1].mag)
                    {
                        ImageWeakStrong->pData[indice] = Low;
                    }
                    else
                    {
                        if( ImageIn->pSobelPixel[indice].mag < low_threshold)//define a macro for this could be usefull
                        {
                            ImageWeakStrong->pData[indice] = Low;
                        }
                        else if(ImageIn->pSobelPixel[indice].mag < high_threshold)
                        {
                            ImageWeakStrong->pData[indice] = Weak;
                        }
                        else
                        {
                            ImageWeakStrong->pData[indice] = Strong;
                        }
                    }
                    break;
                default:
                    break;
                }
            }
            else
            {
                ImageWeakStrong->pData[indice] = Low;
            }
        }
    }
}
//Hysteresis function for q15
void arm_hysteresis_in_q15_out_q15_proc_q15(const arm_image_sobel_q15_t* ImageIn, arm_image_gray_q15_t* ImageOut, arm_image_u8_t* ImageWeakStrong)
{
    int high_threshold = (int)(5728);//0.7*255<<5  //5664
    for( int x = 0; x < ImageIn->numRows; x++)
    {
        for( int y =0; y < ImageIn->numCols ; y++)
        {
            int indice = x*ImageIn->numCols+y;
            int w = ImageIn->numCols;
            //Switch case depending on the result of the comparison with the thresholds
            //if we are lower then the low threshold, we don't keep the value
            //if we are higher than the high threshold the value is kept
            //else the value is only kept if it is connected to a high value
            switch (ImageWeakStrong->pData[indice])
            {
            case Low:
                ImageOut->pData[indice] = 0;
                break;
            case Strong:
                ImageOut->pData[indice] = 255;
            break;
            case Weak:
                if (ImageIn->pSobelPixel[indice-w-1].mag>=(int)(high_threshold) ||ImageIn->pSobelPixel[indice-w].mag>=(int)(high_threshold) ||ImageIn->pSobelPixel[indice-w+1].mag>=(int)(high_threshold) ||ImageIn->pSobelPixel[indice-1].mag>=(int)(high_threshold) ||ImageIn->pSobelPixel[indice+1].mag>=(int)(high_threshold) ||ImageIn->pSobelPixel[indice+w+1].mag>=(int)(high_threshold) ||ImageIn->pSobelPixel[indice+w].mag>=(int)(high_threshold) ||ImageIn->pSobelPixel[indice+w-1].mag>=(int)(high_threshold))
                {
                    ImageOut->pData[indice] = 255;
                }
                else
                {
                    ImageOut->pData[indice] = 0;
                }
            default:
                break;
            }
        }
    }
}
//Thining function for float16
void arm_thining_angle_in_f16_out_u8_proc_f16(const arm_image_sobel_f16_t* ImageIn, arm_image_u8_t* ImageWeakStrong)
{
    //two value of threshold for decision on pixels, hard coded but should be passed as argument
    int low_threshold = (int)(79);
    int high_threshold = (int)(179);
    //Loop to run thought all the image
    for( int x = 0; x < ImageIn->numRows; x++)
    {
        for( int y =0; y < ImageIn->numCols; y++)
        {
            int indice = x*ImageIn->numCols+y;
            int w = ImageIn->numCols;
            if((float)ImageIn->pSobelPixel[indice].mag > low_threshold/*!= (float)0.0*/)
            {
                float valuein = (float)ImageIn->pSobelPixel[indice].mag;
                //the angle allow us to determine what are the two magnitude value we need to first compare with
                //then we make a decision on keeping the pixel base on the threshold, if the magnitude value supass the high threshold we keep it
                //else we need to check the 3x3 suqare around the value to see if its an edge or not
                switch ((int)ImageIn->pSobelPixel[indice].angle)
                {
                case Horizontal:
                    if( valuein <= (float)ImageIn->pSobelPixel[indice-1].mag || valuein <= (float)ImageIn->pSobelPixel[indice+1].mag)
                    {
                        ImageWeakStrong->pData[indice] = Low;

                    }
                    else
                    {
                        /*if( (float)ImageIn->pSobelPixel[indice].mag < low_threshold)
                        {
                            ImageWeakStrong->pData[indice] = Low;
                        }
                        else*/ if((float)ImageIn->pSobelPixel[indice].mag < high_threshold)
                        {
                            ImageWeakStrong->pData[indice] = Weak;
                        }
                        else
                        {
                            ImageWeakStrong->pData[indice] = Strong;
                        }
                    }
                    break;
                case Diagonal_rl:
                    if( valuein <= (float)ImageIn->pSobelPixel[indice+w-1].mag || valuein <= (float)ImageIn->pSobelPixel[indice-w+1].mag)
                    {
                        ImageWeakStrong->pData[indice] = Low;
                    }
                    else
                    {
                        /*if( (float)ImageIn->pSobelPixel[indice].mag < low_threshold)
                        {
                            ImageWeakStrong->pData[indice] = Low;
                        }
                        else*/ if((float)ImageIn->pSobelPixel[indice].mag < high_threshold)
                        {
                            ImageWeakStrong->pData[indice] = Weak;
                        }
                        else
                        {
                            ImageWeakStrong->pData[indice] = Strong;
                        }
                    }
                    break;
                case Vertical:
                    if(  valuein <= (float)ImageIn->pSobelPixel[indice-w].mag ||  valuein <= (float)ImageIn->pSobelPixel[indice+w].mag )
                    {
                        ImageWeakStrong->pData[indice] = Low;
                    }
                    else
                    {
                        /*if( (float)ImageIn->pSobelPixel[indice].mag < low_threshold)
                        {
                            ImageWeakStrong->pData[indice] = Low;
                        }
                        else*/ if((float)ImageIn->pSobelPixel[indice].mag < high_threshold)
                        {
                            ImageWeakStrong->pData[indice] = Weak;
                        }
                        else
                        {
                            ImageWeakStrong->pData[indice] = Strong;
                        }
                    }
                    break;
                case Diagonal_lr:
                    if( valuein <= (float)ImageIn->pSobelPixel[indice-w-1].mag || valuein <= (float)ImageIn->pSobelPixel[indice+w+1].mag)
                    {
                        ImageWeakStrong->pData[indice] = Low;
                    }
                    else
                    {
                        /*if( (float)ImageIn->pSobelPixel[indice].mag < low_threshold)//define a macro for this could be usefull
                        {
                            ImageWeakStrong->pData[indice] = Low;
                        }
                        else*/ if((float)ImageIn->pSobelPixel[indice].mag < high_threshold)
                        {
                            ImageWeakStrong->pData[indice] = Weak;
                        }
                        else
                        {
                            ImageWeakStrong->pData[indice] = Strong;
                        }
                    }
                    break;
                default:
                    break;
                }
            }
            else
            {
                ImageWeakStrong->pData[indice] = Low;
            }
        }
    }
}
//Hysteresis function for float16
void arm_hysteresis_in_f16_out_f16_proc_f16(const arm_image_sobel_f16_t* ImageIn, arm_image_gray_u8_t* ImageOut, arm_image_u8_t* ImageWeakStrong)
{
    int high_threshold = (int)(179);
    for( int x = 0; x < ImageIn->numRows;x++)
    {
        for( int y =0; y < ImageIn->numCols ; y++)
        {
            int indice = x*ImageIn->numCols+y;
            int w = ImageIn->numCols;
            //Switch case depending on the result of the comparison with the thresholds
            //if we are lower then the low threshold, we don't keep the value
            //if we are higher than the high threshold the value is kept
            //else the value is only kept if it is connected to a high value
            switch (ImageWeakStrong->pData[indice])
            {
            case Low:
                ImageOut->pData[indice] = 0;
                break;
            case Strong:
                ImageOut->pData[indice] = 255;
                break;
            case Weak:
                if ((float)ImageIn->pSobelPixel[indice-w-1].mag>=(int)(high_threshold) ||(float)ImageIn->pSobelPixel[indice-w].mag>=(int)(high_threshold) ||(float)ImageIn->pSobelPixel[indice-w+1].mag>=(int)(high_threshold) ||(float)ImageIn->pSobelPixel[indice-1].mag>=(int)(high_threshold) ||(float)ImageIn->pSobelPixel[indice+1].mag>=(int)(high_threshold) ||(float)ImageIn->pSobelPixel[indice+w+1].mag>=(int)(high_threshold) ||(float)ImageIn->pSobelPixel[indice+w].mag>=(int)(high_threshold) ||(float)ImageIn->pSobelPixel[indice+w-1].mag>=(int)(high_threshold))
                {
                    ImageOut->pData[indice] = 255;
                }
                else
                {
                    ImageOut->pData[indice] = 0;
                }
            default:
                break;
            }
        }
    }
}

#if ((!defined(ARM_MATH_MVEI)) |(defined(FORCE_SCALAR)))
//Canny edge function for float16 scalar
void arm_canny_edge_in_f16_out_u8_proc_f16(const arm_image_sobel_f16_t* ImageIn, arm_image_gray_u8_t* ImageOut)
{
    //two value of threshold for decision on pixels, hard coded but should be passed as argument
    int low_threshold = (int)(79);
    int high_threshold = (int)(179);
    //main loop going on all the image
    for( int x = 0; x < ImageIn->numRows; x++)
    {
        for( int y =0; y < ImageIn->numCols; y++)
        {
            int indice = x*ImageIn->numCols+y;
            int w = ImageIn->numCols;
            //first test, in particular for the border and in general for other values, if the magnitude is inferiro to the low threshold, we won't keep it
            if((float)ImageIn->pSobelPixel[indice].mag > low_threshold/*!= (float)(0.0)*/)
            {
                float valuein = (float)ImageIn->pSobelPixel[indice].mag;
                switch ((int)ImageIn->pSobelPixel[indice].angle)
                {
                case Horizontal:
                    if( valuein <= (float)ImageIn->pSobelPixel[indice-1].mag || valuein <= (float)ImageIn->pSobelPixel[indice+1].mag)
                    {
                        ImageOut->pData[indice] = 0;
                    }
                    else
                    {
                        /*if( (float)ImageIn->pSobelPixel[indice].mag < low_threshold)
                        {
                            ImageOut->pData[indice] = 0;
                        }
                        else*/ if((float)ImageIn->pSobelPixel[indice].mag < high_threshold)
                        {
                            if ((float)ImageIn->pSobelPixel[indice-w-1].mag>=(int)(high_threshold) ||(float)ImageIn->pSobelPixel[indice-w].mag>=(int)(high_threshold) ||(float)ImageIn->pSobelPixel[indice-w+1].mag>=(int)(high_threshold) ||(float)ImageIn->pSobelPixel[indice+w+1].mag>=(int)(high_threshold) ||(float)ImageIn->pSobelPixel[indice+w].mag>=(int)(high_threshold) ||(float)ImageIn->pSobelPixel[indice+w-1].mag>=(int)(high_threshold))
                            {
                                ImageOut->pData[indice] = 255;
                            }
                            else
                            {
                                ImageOut->pData[indice] = 0;
                            }
                        }
                        else
                        {
                            ImageOut->pData[indice] = 255;
                        }
                    }
                    break;
                case Diagonal_rl:
                    if( valuein <= (float)ImageIn->pSobelPixel[indice+w-1].mag || valuein <= (float)ImageIn->pSobelPixel[indice-w+1].mag)
                    {
                        ImageOut->pData[indice] = 0;
                    }
                    else
                    {
                        /*if( (float)ImageIn->pSobelPixel[indice].mag < low_threshold)
                        {
                            ImageOut->pData[indice] = 0;
                        }
                        else*/ if((float)ImageIn->pSobelPixel[indice].mag < high_threshold)
                        {
                            if ((float)ImageIn->pSobelPixel[indice-w-1].mag>=(int)(high_threshold) ||(float)ImageIn->pSobelPixel[indice-w].mag>=(int)(high_threshold) ||(float)ImageIn->pSobelPixel[indice-1].mag>=(int)(high_threshold) ||(float)ImageIn->pSobelPixel[indice+1].mag>=(int)(high_threshold) ||(float)ImageIn->pSobelPixel[indice+w].mag>=(int)(high_threshold) ||(float)ImageIn->pSobelPixel[indice+w-1].mag>=(int)(high_threshold))
                            {
                                ImageOut->pData[indice] = 255;
                            }
                            else
                            {
                                ImageOut->pData[indice] = 0;
                            }
                        }
                        else
                        {
                            ImageOut->pData[indice] = 255;
                        }
                    }
                    break;
                case Vertical:
                    if(  valuein <= (float)ImageIn->pSobelPixel[indice-w].mag ||  valuein <= (float)ImageIn->pSobelPixel[indice+w].mag )
                    {
                        ImageOut->pData[indice] = 0;
                    }
                    else
                    {
                        /*if( (float)ImageIn->pSobelPixel[indice].mag < low_threshold)
                        {
                            ImageOut->pData[indice] = 0;
                        }
                        else*/ if((float)ImageIn->pSobelPixel[indice].mag < high_threshold)
                        {
                            if ((float)ImageIn->pSobelPixel[indice-w-1].mag>=(int)(high_threshold) ||(float)ImageIn->pSobelPixel[indice-w+1].mag>=(int)(high_threshold) ||(float)ImageIn->pSobelPixel[indice-1].mag>=(int)(high_threshold) ||(float)ImageIn->pSobelPixel[indice+1].mag>=(int)(high_threshold) ||(float)ImageIn->pSobelPixel[indice+w+1].mag>=(int)(high_threshold) ||(float)ImageIn->pSobelPixel[indice+w-1].mag>=(int)(high_threshold))
                            {
                                ImageOut->pData[indice] = 255;
                            }
                            else
                            {
                                ImageOut->pData[indice] = 0;
                            }
                        }
                        else
                        {
                            ImageOut->pData[indice] = 255;
                        }
                    }
                    break;
                case Diagonal_lr:
                    if( valuein <= (float)ImageIn->pSobelPixel[indice-w-1].mag || valuein <= (float)ImageIn->pSobelPixel[indice+w+1].mag)
                    {
                        ImageOut->pData[indice] = 0;
                    }
                    else
                    {
                        /*if( (float)ImageIn->pSobelPixel[indice].mag < low_threshold)//define a macro for this could be usefull
                        {
                            ImageOut->pData[indice] = 0;
                        }
                        else */if((float)ImageIn->pSobelPixel[indice].mag < high_threshold)
                        {
                            if ((float)ImageIn->pSobelPixel[indice-w].mag>=(int)(high_threshold) ||(float)ImageIn->pSobelPixel[indice-w+1].mag>=(int)(high_threshold) ||(float)ImageIn->pSobelPixel[indice-1].mag>=(int)(high_threshold) ||(float)ImageIn->pSobelPixel[indice+1].mag>=(int)(high_threshold) ||(float)ImageIn->pSobelPixel[indice+w].mag>=(int)(high_threshold) ||(float)ImageIn->pSobelPixel[indice+w-1].mag>=(int)(high_threshold))
                            {
                                ImageOut->pData[indice] = 255;
                            }
                            else
                            {
                                ImageOut->pData[indice] = 0;
                            }
                        }
                        else
                        {
                            ImageOut->pData[indice] = 255;
                        }
                    }
                    break;
                default:
                    break;
                }
            }
            else
            {
                ImageOut->pData[indice] = 0;
            }
        }
    }
}
#else
//Canny edge function for float16 vector
void arm_canny_edge_in_f16_out_u8_proc_f16(const arm_image_sobel_f16_t* ImageIn, arm_image_gray_u8_t* ImageOut)
{
    //two value of threshold for decision on pixels, hard coded but should be passed as argument
    int low_threshold = (int)(79);
    int high_threshold = (int)(179);
    //main loop going on all the image
    for( int x = 0; x < ImageIn->numRows; x++)
    {
        for( int y =0; y < ImageIn->numCols - (ImageIn->numCols&15); y+=16)
        {
            int indice = x*ImageIn->numCols+y;
            int w = ImageIn->numCols;
            //Sobel image containt two information per pixel, so by doing a vld2, we read those two information and split them into separate vectors
            float16x8x2_t vect_input = vld2q_f16(&ImageIn->pSobelPixel[x*ImageIn->numCols+y].mag);
            uint8x16_t vect_out;
            for(int i =0; i < 8;i++)
            {
                float valuein = (float)vect_input.val[0][i];
                //first test, in particular for the border and in general for other values, if the magnitude is inferiro to the low threshold, we won't keep it
                if(valuein > low_threshold/*!= (float)(0.0)*/)
                {
                    //Here is a disjonction of case, 
                    //depending on the angle we have, we compare the value of the magnitude withe the two value in a 3x3 square on the same line 
                    //then depending on this result, we compare the value of the magnitude with the threshold values
                    //one threshold act for ignoring, another for keeping and a last imply to check on the 3x3 square if the pixel is an edge or not to see if we keep it or not
                    switch ((int)vect_input.val[1][i])
                    {
                    case Horizontal:
                        if( valuein <= (float)ImageIn->pSobelPixel[indice+i-1].mag || valuein <= (float)ImageIn->pSobelPixel[indice+i+1].mag)
                        {
                            vect_out[i] = 0;
                        }
                        else
                        {
                            /*if( valuein < low_threshold)
                            {
                                vect_out[i] = 0;
                            }
                            else */if(valuein < high_threshold)
                            {
                                if ((float)ImageIn->pSobelPixel[indice+i-w-1].mag>=(int)(high_threshold) ||(float)ImageIn->pSobelPixel[indice+i-w].mag>=(int)(high_threshold) ||(float)ImageIn->pSobelPixel[indice+i-w+1].mag>=(int)(high_threshold) ||(float)ImageIn->pSobelPixel[indice+i+w+1].mag>=(int)(high_threshold) ||(float)ImageIn->pSobelPixel[indice+i+w].mag>=(int)(high_threshold) ||(float)ImageIn->pSobelPixel[indice+i+w-1].mag>=(int)(high_threshold))
                                {
                                    vect_out[i] = 255;
                                }
                                else
                                {
                                    vect_out[i] = 0;
                                }
                            }
                            else
                            {
                                vect_out[i] = 255;
                            }
                        }
                        break;
                    case Diagonal_rl:
                        if( valuein <= (float)ImageIn->pSobelPixel[indice+i+w-1].mag || valuein <= (float)ImageIn->pSobelPixel[indice+i-w+1].mag)
                        {
                            vect_out[i] = 0;
                        }
                        else
                        {
                            /*if( valuein < low_threshold)
                            {
                                vect_out[i] = 0;
                            }
                            else*/ if(valuein < high_threshold)
                            {
                                if ((float)ImageIn->pSobelPixel[indice+i-w-1].mag>=(int)(high_threshold) ||(float)ImageIn->pSobelPixel[indice+i-w].mag>=(int)(high_threshold) ||(float)ImageIn->pSobelPixel[indice+i-1].mag>=(int)(high_threshold) ||(float)ImageIn->pSobelPixel[indice+i+1].mag>=(int)(high_threshold) ||(float)ImageIn->pSobelPixel[indice+i+w].mag>=(int)(high_threshold) ||(float)ImageIn->pSobelPixel[indice+i+w-1].mag>=(int)(high_threshold))
                                {
                                    vect_out[i] = 255;
                                }
                                else
                                {
                                    vect_out[i] = 0;
                                }
                            }
                            else
                            {
                                vect_out[i] = 255;
                            }
                        }
                        break;
                    case Vertical:
                        if(  valuein <= (float)ImageIn->pSobelPixel[indice+i-w].mag ||  valuein <= (float)ImageIn->pSobelPixel[indice+i+w].mag )
                        {
                            vect_out[i] = 0;
                        }
                        else
                        {
                            /*if( valuein < low_threshold)
                            {
                                vect_out[i] = 0;
                            }
                            else*/ if(valuein < high_threshold)
                            {
                                if ((float)ImageIn->pSobelPixel[indice+i-w-1].mag>=(int)(high_threshold) ||(float)ImageIn->pSobelPixel[indice+i-w+1].mag>=(int)(high_threshold) ||(float)ImageIn->pSobelPixel[indice+i-1].mag>=(int)(high_threshold) ||(float)ImageIn->pSobelPixel[indice+i+1].mag>=(int)(high_threshold) ||(float)ImageIn->pSobelPixel[indice+i+w+1].mag>=(int)(high_threshold) ||(float)ImageIn->pSobelPixel[indice+i+w-1].mag>=(int)(high_threshold))
                                {
                                    vect_out[i] = 255;
                                }
                                else
                                {
                                    vect_out[i] = 0;
                                }
                            }
                            else
                            {
                                vect_out[i] = 255;
                            }
                        }
                        break;
                    case Diagonal_lr:
                        if( (float)valuein <= (float)ImageIn->pSobelPixel[indice+i-w-1].mag || (float)valuein <= (float)ImageIn->pSobelPixel[indice+i+w+1].mag)
                        {
                            vect_out[i] = 0;
                        }
                        else
                        {
                            /*if( valuein < low_threshold)
                            {
                                vect_out[i] = 0;
                            }
                            else*/ if(valuein < high_threshold)
                            {
                                if ((float)ImageIn->pSobelPixel[indice+i-w].mag>=(int)(high_threshold) ||(float)ImageIn->pSobelPixel[indice+i-w+1].mag>=(int)(high_threshold) ||(float)ImageIn->pSobelPixel[indice+i-1].mag>=(int)(high_threshold) ||(float)ImageIn->pSobelPixel[indice+i+1].mag>=(int)(high_threshold) ||(float)ImageIn->pSobelPixel[indice+i+w].mag>=(int)(high_threshold) ||(float)ImageIn->pSobelPixel[indice+i+w-1].mag>=(int)(high_threshold))
                                {
                                    vect_out[i] = 255;
                                }
                                else
                                {
                                    vect_out[i] = 0;
                                }
                            }
                            else
                            {
                                vect_out[i] = 255;
                            }
                        }
                        break;
                    default:
                        break;
                    }
                }
                else
                {
                    vect_out[i] = 0;
                }
            }
            vect_input = vld2q_f16(&ImageIn->pSobelPixel[x*ImageIn->numCols+y+8].mag);
            //tail applyed for each line
            for(int i =0; i < 8;i++)
            {
                //vect_out[i+8]=10;
                float valuein = (float)vect_input.val[0][i];
                //first test, in particular for the border and in general for other values, if the magnitude is inferiro to the low threshold, we won't keep it
                if(valuein > low_threshold/*!= (float)(0.0)*/)
                {
                    //Here is a disjonction of case, 
                    //depending on the angle we have, we compare the value of the magnitude withe the two value in a 3x3 square on the same line 
                    //then depending on this result, we compare the value of the magnitude with the threshold values
                    //one threshold act for ignoring, another for keeping and a last imply to check on the 3x3 square if the pixel is an edge or not to see if we keep it or not
                    switch ((int)vect_input.val[1][i])
                    {
                    case Horizontal:
                        if( valuein <= (float)ImageIn->pSobelPixel[indice+i+8-1].mag || valuein <= (float)ImageIn->pSobelPixel[indice+i+8+1].mag)
                        {
                            vect_out[i+8] = 0;
                        }
                        else
                        {
                            /*if( valuein < low_threshold)
                            {
                                vect_out[i+8] = 0;
                            }
                            else*/ if(valuein < high_threshold)
                            {
                                if ((float)ImageIn->pSobelPixel[indice+i+8-w-1].mag>=(int)(high_threshold) ||(float)ImageIn->pSobelPixel[indice+i+8-w].mag>=(int)(high_threshold) ||(float)ImageIn->pSobelPixel[indice+i+8-w+1].mag>=(int)(high_threshold) ||(float)ImageIn->pSobelPixel[indice+i+8+w+1].mag>=(int)(high_threshold) ||(float)ImageIn->pSobelPixel[indice+i+8+w].mag>=(int)(high_threshold) ||(float)ImageIn->pSobelPixel[indice+i+8+w-1].mag>=(int)(high_threshold))
                                {
                                    vect_out[i+8] = 255;
                                }
                                else
                                {
                                    vect_out[i+8] = 0;
                                }
                            }
                            else
                            {
                                vect_out[i+8] = 255;
                            }
                        }
                        break;
                    case Diagonal_rl:
                        if( valuein <= (float)ImageIn->pSobelPixel[indice+i+8+w-1].mag || valuein <= (float)ImageIn->pSobelPixel[indice+i+8-w+1].mag)
                        {
                            vect_out[i+8] = 0;
                        }
                        else
                        {
                            /*if( valuein < low_threshold)
                            {
                                vect_out[i+8] = 0;
                            }
                            else*/ if(valuein < high_threshold)
                            {
                                if ((float)ImageIn->pSobelPixel[indice+i+8-w-1].mag>=(int)(high_threshold) ||(float)ImageIn->pSobelPixel[indice+i+8-w].mag>=(int)(high_threshold) ||(float)ImageIn->pSobelPixel[indice+i+8-1].mag>=(int)(high_threshold) ||(float)ImageIn->pSobelPixel[indice+i+8+1].mag>=(int)(high_threshold) ||(float)ImageIn->pSobelPixel[indice+i+8+w].mag>=(int)(high_threshold) ||(float)ImageIn->pSobelPixel[indice+i+8+w-1].mag>=(int)(high_threshold))
                                {
                                    vect_out[i+8] = 255;
                                }
                                else
                                {
                                    vect_out[i+8] = 0;
                                }
                            }
                            else
                            {
                                vect_out[i+8] = 255;
                            }
                        }
                        break;
                    case Vertical:
                        if(  valuein <= (float)ImageIn->pSobelPixel[indice+i+8-w].mag ||  valuein <= (float)ImageIn->pSobelPixel[indice+i+8+w].mag )
                        {
                            vect_out[i+8] = 0;
                        }
                        else
                        {
                            /*if( valuein < low_threshold)
                            {
                                vect_out[i+8] = 0;
                            }
                            else*/ if(valuein < high_threshold)
                            {
                                if ((float)ImageIn->pSobelPixel[indice+i+8-w-1].mag>=(int)(high_threshold) ||(float)ImageIn->pSobelPixel[indice+i+8-w+1].mag>=(int)(high_threshold) ||(float)ImageIn->pSobelPixel[indice+i+8-1].mag>=(int)(high_threshold) ||(float)ImageIn->pSobelPixel[indice+i+8+1].mag>=(int)(high_threshold) ||(float)ImageIn->pSobelPixel[indice+i+8+w+1].mag>=(int)(high_threshold) ||(float)ImageIn->pSobelPixel[indice+i+8+w-1].mag>=(int)(high_threshold))
                                {
                                    vect_out[i+8] = 255;
                                }
                                else
                                {
                                    vect_out[i+8] = 0;
                                }
                            }
                            else
                            {
                                vect_out[i+8] = 255;
                            }
                        }
                        break;
                    case Diagonal_lr:
                        if( (float)valuein <= (float)ImageIn->pSobelPixel[indice+i+8-w-1].mag || (float)valuein <= (float)ImageIn->pSobelPixel[indice+i+8+w+1].mag)
                        {
                            vect_out[i+8] = 0;
                        }
                        else
                        {
                            /*if( valuein < low_threshold)
                            {
                                vect_out[i+8] = 0;
                            }
                            else*/ if(valuein < high_threshold)
                            {
                                if ((float)ImageIn->pSobelPixel[indice+i+8-w].mag>=(int)(high_threshold) ||(float)ImageIn->pSobelPixel[indice+i+8-w+1].mag>=(int)(high_threshold) ||(float)ImageIn->pSobelPixel[indice+i+8-1].mag>=(int)(high_threshold) ||(float)ImageIn->pSobelPixel[indice+i+8+1].mag>=(int)(high_threshold) ||(float)ImageIn->pSobelPixel[indice+i+8+w].mag>=(int)(high_threshold) ||(float)ImageIn->pSobelPixel[indice+i+8+w-1].mag>=(int)(high_threshold))
                                {
                                    vect_out[i+8] = 255;
                                }
                                else
                                {
                                    vect_out[i+8] = 0;
                                }
                            }
                            else
                            {
                                vect_out[i+8] = 255;
                            }
                        }
                        break;
                    default:
                        break;
                    }
                }
                else
                {
                    vect_out[i+8] = 0;
                }
            }
            vst1q_u8(&ImageOut->pData[x*ImageIn->numCols+y], vect_out);
        }
        //loop for the tail done in scalar
        for( int y =ImageIn->numCols- (ImageIn->numCols&15); y < ImageIn->numCols; y++)
        {
            int indice = x*ImageIn->numCols+y;
            int w = ImageIn->numCols;
            //first test, in particular for the border and in general for other values, if the magnitude is inferiro to the low threshold, we won't keep it
            if((float)ImageIn->pSobelPixel[indice].mag > low_threshold/*!= (float)(0.0)*/)
            {
                float valuein = (float)ImageIn->pSobelPixel[indice].mag;
                //Here is a disjonction of case, 
                //depending on the angle we have, we compare the value of the magnitude withe the two value in a 3x3 square on the same line 
                //then depending on this result, we compare the value of the magnitude with the threshold values
                //one threshold act for ignoring, another for keeping and a last imply to check on the 3x3 square if the pixel is an edge or not to see if we keep it or not
                switch ((int)ImageIn->pSobelPixel[indice].angle)
                {
                case Horizontal:
                    if( valuein <= (float)ImageIn->pSobelPixel[indice-1].mag || valuein <= (float)ImageIn->pSobelPixel[indice+1].mag)
                    {
                        ImageOut->pData[indice] = 0;
                    }
                    else
                    {
                        /*if( (float)ImageIn->pSobelPixel[indice].mag < low_threshold)
                        {
                            ImageOut->pData[indice] = 0;
                        }
                        else*/ if((float)ImageIn->pSobelPixel[indice].mag < high_threshold)
                        {
                            if ((float)ImageIn->pSobelPixel[indice-w-1].mag>=(int)(high_threshold) ||(float)ImageIn->pSobelPixel[indice-w].mag>=(int)(high_threshold) ||(float)ImageIn->pSobelPixel[indice-w+1].mag>=(int)(high_threshold) ||(float)ImageIn->pSobelPixel[indice+w+1].mag>=(int)(high_threshold) ||(float)ImageIn->pSobelPixel[indice+w].mag>=(int)(high_threshold) ||(float)ImageIn->pSobelPixel[indice+w-1].mag>=(int)(high_threshold))
                            {
                                ImageOut->pData[indice] = 255;
                            }
                            else
                            {
                                ImageOut->pData[indice] = 0;
                            }
                        }
                        else
                        {
                            ImageOut->pData[indice] = 255;
                        }
                    }
                    break;
                case Diagonal_rl:
                    if( valuein <= (float)ImageIn->pSobelPixel[indice+w-1].mag || valuein <= (float)ImageIn->pSobelPixel[indice-w+1].mag)
                    {
                        ImageOut->pData[indice] = 0;
                    }
                    else
                    {
                        /*if( (float)ImageIn->pSobelPixel[indice].mag < low_threshold)
                        {
                            ImageOut->pData[indice] = 0;
                        }
                        else*/ if((float)ImageIn->pSobelPixel[indice].mag < high_threshold)
                        {
                            if ((float)ImageIn->pSobelPixel[indice-w-1].mag>=(int)(high_threshold) ||(float)ImageIn->pSobelPixel[indice-w].mag>=(int)(high_threshold) ||(float)ImageIn->pSobelPixel[indice-1].mag>=(int)(high_threshold) ||(float)ImageIn->pSobelPixel[indice+1].mag>=(int)(high_threshold) ||(float)ImageIn->pSobelPixel[indice+w].mag>=(int)(high_threshold) ||(float)ImageIn->pSobelPixel[indice+w-1].mag>=(int)(high_threshold))
                            {
                                ImageOut->pData[indice] = 255;
                            }
                            else
                            {
                                ImageOut->pData[indice] = 0;
                            }
                        }
                        else
                        {
                            ImageOut->pData[indice] = 255;
                        }
                    }
                    break;
                case Vertical:
                    if(  valuein <= (float)ImageIn->pSobelPixel[indice-w].mag ||  valuein <= (float)ImageIn->pSobelPixel[indice+w].mag )
                    {
                        ImageOut->pData[indice] = 0;
                    }
                    else
                    {
                        /*if( (float)ImageIn->pSobelPixel[indice].mag < low_threshold)
                        {
                            ImageOut->pData[indice] = 0;
                        }
                        else*/ if((float)ImageIn->pSobelPixel[indice].mag < high_threshold)
                        {
                            if ((float)ImageIn->pSobelPixel[indice-w-1].mag>=(int)(high_threshold) ||(float)ImageIn->pSobelPixel[indice-w+1].mag>=(int)(high_threshold) ||(float)ImageIn->pSobelPixel[indice-1].mag>=(int)(high_threshold) ||(float)ImageIn->pSobelPixel[indice+1].mag>=(int)(high_threshold) ||(float)ImageIn->pSobelPixel[indice+w+1].mag>=(int)(high_threshold) ||(float)ImageIn->pSobelPixel[indice+w-1].mag>=(int)(high_threshold))
                            {
                                ImageOut->pData[indice] = 255;
                            }
                            else
                            {
                                ImageOut->pData[indice] = 0;
                            }
                        }
                        else
                        {
                            ImageOut->pData[indice] = 255;
                        }
                    }
                    break;
                case Diagonal_lr:
                    if( valuein <= (float)ImageIn->pSobelPixel[indice-w-1].mag || valuein <= (float)ImageIn->pSobelPixel[indice+w+1].mag)
                    {
                        ImageOut->pData[indice] = 0;
                    }
                    else
                    {
                        /*if( (float)ImageIn->pSobelPixel[indice].mag < low_threshold)
                        {
                            ImageOut->pData[indice] = 0;
                        }
                        else*/ if((float)ImageIn->pSobelPixel[indice].mag < high_threshold)
                        {
                            if ((float)ImageIn->pSobelPixel[indice-w].mag>=(int)(high_threshold) ||(float)ImageIn->pSobelPixel[indice-w+1].mag>=(int)(high_threshold) ||(float)ImageIn->pSobelPixel[indice-1].mag>=(int)(high_threshold) ||(float)ImageIn->pSobelPixel[indice+1].mag>=(int)(high_threshold) ||(float)ImageIn->pSobelPixel[indice+w].mag>=(int)(high_threshold) ||(float)ImageIn->pSobelPixel[indice+w-1].mag>=(int)(high_threshold))
                            {
                                ImageOut->pData[indice] = 255;
                            }
                            else
                            {
                                ImageOut->pData[indice] = 0;
                            }
                        }
                        else
                        {
                            ImageOut->pData[indice] = 255;
                        }
                    }
                    break;
                default:
                    break;
                }
            }
            else
            {
                ImageOut->pData[indice] = 0;
            }
        }
    }
}
#endif
#if ((!defined(ARM_MATH_MVEI)) |(defined(FORCE_SCALAR)))
//Canny edge function for q15 scalar
void arm_canny_edge_in_out_q15_u8_proc_q15(const arm_image_sobel_q15_t* ImageIn, arm_image_gray_q15_t* ImageOut)
{
    //two value of threshold for decision on pixels, hard coded but should be passed as argument
    int low_threshold = (int)(2528);//0.3*255<<5
    int high_threshold = (int)(5728);//0.7*255<<5  //5664
    //main loop going on all the image
    for( int x = 0; x < ImageIn->numRows; x++)
    {
        for( int y =0; y < ImageIn->numCols; y++)
        {
            int indice = x*ImageIn->numCols+y;
            int w = ImageIn->numCols;
            //first test, in particular for the border and in general for other values, if the magnitude is inferiro to the low threshold, we won't keep it
            if(ImageIn->pSobelPixel[indice].mag > low_threshold/*!=0*/)
            {
                int valuein = ImageIn->pSobelPixel[indice].mag;
                //Here is a disjonction of case, 
                //depending on the angle we have, we compare the value of the magnitude withe the two value in a 3x3 square on the same line 
                //then depending on this result, we compare the value of the magnitude with the threshold values
                //one threshold act for ignoring, another for keeping and a last imply to check on the 3x3 square if the pixel is an edge or not to see if we keep it or not
                switch (ImageIn->pSobelPixel[indice].angle)
                {
                case Horizontal:
                    if( valuein <= ImageIn->pSobelPixel[indice-1].mag || valuein <= ImageIn->pSobelPixel[indice+1].mag)
                    {
                        ImageOut->pData[indice] = 0;
                        continue;
                    }
                    else
                    {
                        /*if( ImageIn->pSobelPixel[indice].mag < low_threshold)
                        {
                            ImageOut->pData[indice] = 0;
                            continue;
                        }
                        else */if(ImageIn->pSobelPixel[indice].mag < high_threshold)
                        {
                            if (ImageIn->pSobelPixel[indice-w-1].mag>=(int)(high_threshold) ||ImageIn->pSobelPixel[indice-w].mag>=(int)(high_threshold) ||ImageIn->pSobelPixel[indice-w+1].mag>=(int)(high_threshold)||ImageIn->pSobelPixel[indice+w+1].mag>=(int)(high_threshold) ||ImageIn->pSobelPixel[indice+w].mag>=(int)(high_threshold) ||ImageIn->pSobelPixel[indice+w-1].mag>=(int)(high_threshold))
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
                            break;
                        }
                    }
                    break;
                case Diagonal_rl:
                    if( valuein <= ImageIn->pSobelPixel[indice+w-1].mag || valuein <= ImageIn->pSobelPixel[indice-w+1].mag)
                    {
                        ImageOut->pData[indice] = 0;
                        continue;
                    }
                    else
                    {
                        /*if( ImageIn->pSobelPixel[indice].mag < low_threshold)
                        {
                            ImageOut->pData[indice] = 0;
                            continue;
                        }
                        else */if(ImageIn->pSobelPixel[indice].mag < high_threshold)
                        {
                            if (ImageIn->pSobelPixel[indice-w-1].mag>=(int)(high_threshold) ||ImageIn->pSobelPixel[indice-w].mag>=(int)(high_threshold)||ImageIn->pSobelPixel[indice-1].mag>=(int)(high_threshold) ||ImageIn->pSobelPixel[indice+1].mag>=(int)(high_threshold)||ImageIn->pSobelPixel[indice+w].mag>=(int)(high_threshold) ||ImageIn->pSobelPixel[indice+w-1].mag>=(int)(high_threshold))
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
                    break;
                case Vertical:
                    if(  0 <= ImageIn->pSobelPixel[indice-w].mag - valuein||  0 <= ImageIn->pSobelPixel[indice+w].mag - valuein)
                    {
                        ImageOut->pData[indice] = 0;
                        continue;
                    }
                    else
                    {
                        /*if( ImageIn->pSobelPixel[indice].mag < low_threshold)
                        {
                            ImageOut->pData[indice] = 0;
                            continue;
                        }
                        else */if(ImageIn->pSobelPixel[indice].mag < high_threshold)
                        {
                            if (ImageIn->pSobelPixel[indice-w-1].mag>=(int)(high_threshold)||ImageIn->pSobelPixel[indice-w+1].mag>=(int)(high_threshold) ||ImageIn->pSobelPixel[indice-1].mag>=(int)(high_threshold) ||ImageIn->pSobelPixel[indice+1].mag>=(int)(high_threshold) ||ImageIn->pSobelPixel[indice+w+1].mag>=(int)(high_threshold)||ImageIn->pSobelPixel[indice+w-1].mag>=(int)(high_threshold))
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
                    break;
                case Diagonal_lr:
                    if( valuein <= ImageIn->pSobelPixel[indice-w-1].mag || valuein <= ImageIn->pSobelPixel[indice+w+1].mag)
                    {
                        ImageOut->pData[indice] = 0;
                        continue;
                    }
                    else
                    {
                        /*if( ImageIn->pSobelPixel[indice].mag < low_threshold)//define a macro for this could be usefull
                        {
                            ImageOut->pData[indice] = 0;
                            continue;
                        }
                        else */if(ImageIn->pSobelPixel[indice].mag < high_threshold)
                        {
                            if (ImageIn->pSobelPixel[indice-w].mag>=(int)(high_threshold) ||ImageIn->pSobelPixel[indice-w+1].mag>=(int)(high_threshold) ||ImageIn->pSobelPixel[indice-1].mag>=(int)(high_threshold) ||ImageIn->pSobelPixel[indice+1].mag>=(int)(high_threshold) ||ImageIn->pSobelPixel[indice+w].mag>=(int)(high_threshold) ||ImageIn->pSobelPixel[indice+w-1].mag>=(int)(high_threshold))
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
                    break;
                default:
                    break;
                }
            }
            else
            {
                ImageOut->pData[indice] = 0;
                continue;
            }
        }
    }
}
#else
//Canny edge function for q15 scalar
void arm_canny_edge_in_out_q15_u8_proc_q15(const arm_image_sobel_q15_t* ImageIn, arm_image_gray_q15_t* ImageOut)
{
    //two value of threshold for decision on pixels, hard coded but should be passed as argument
    int low_threshold = (int)(2528);
    int high_threshold = (int)(5728);
    //main loop going on all the image
    for( int x = 0; x < ImageIn->numRows; x++)
    {
        for( int y =0; y < (ImageIn->numCols&0xFFF8); y+=8)
        {
            int w = ImageIn->numCols;
            //Sobel image containt two information per pixel, so by doing a vld2, we read those two information and split them into separate vectors
            q15x8x2_t vect_input = vld2q_s16(&ImageIn->pSobelPixel[x*ImageIn->numCols+y].mag);
            int16x8_t vect_out;
            q15x8_t vect_mag = vect_input.val[0];
            q15x8_t vect_angle = vect_input.val[1];
            //due to the different tests we have to do on magnitude and angle, we have to treat each pixel separatly and no longuer can do all operation on th vector
            for(int j =0; j < 8; j++)
            {
                int indice = x*ImageIn->numCols+y+j;
                int magin = vect_mag[j];
                int anglein = vect_angle[j];
                //first test, in particular for the border and in general for other values, if the magnitude is inferiro to the low threshold, we won't keep it
                if(magin > low_threshold/*!=0*/)
                {
                    //Here is a disjonction of case, 
                    //depending on the angle we have, we compare the value of the magnitude withe the two value in a 3x3 square on the same line 
                    //then depending on this result, we compare the value of the magnitude with the threshold values
                    //one threshold act for ignoring, another for keeping and a last imply to check on the 3x3 square if the pixel is an edge or not to see if we keep it or not
                    switch (anglein)
                    {
                    case Horizontal:
                        if( magin <= ImageIn->pSobelPixel[indice-1].mag || magin <= ImageIn->pSobelPixel[indice+1].mag)
                        {
                            vect_out[j] = 0;
                            continue;
                        }
                        else
                        {
                            /*if( magin < low_threshold)
                            {
                                vect_out[j] = 0;
                                continue;
                            }
                            else*/ if(magin < high_threshold)
                            {
                                if (ImageIn->pSobelPixel[indice-w-1].mag>=(int)(high_threshold) ||ImageIn->pSobelPixel[indice-w].mag>=(int)(high_threshold) ||ImageIn->pSobelPixel[indice-w+1].mag>=(int)(high_threshold)||ImageIn->pSobelPixel[indice+w+1].mag>=(int)(high_threshold) ||ImageIn->pSobelPixel[indice+w].mag>=(int)(high_threshold) ||ImageIn->pSobelPixel[indice+w-1].mag>=(int)(high_threshold))
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
                        break;
                    case Diagonal_rl:
                        if( magin <= ImageIn->pSobelPixel[indice+w-1].mag || magin <= ImageIn->pSobelPixel[indice-w+1].mag)
                        {
                            vect_out[j] = 0;
                            continue;
                        }
                        else
                        {
                            /*if( magin < low_threshold)
                            {
                                vect_out[j] = 0;
                                continue;
                            }
                            else*/ if(magin < high_threshold)
                            {
                                if (ImageIn->pSobelPixel[indice-w-1].mag>=(int)(high_threshold) ||ImageIn->pSobelPixel[indice-w].mag>=(int)(high_threshold)||ImageIn->pSobelPixel[indice-1].mag>=(int)(high_threshold) ||ImageIn->pSobelPixel[indice+1].mag>=(int)(high_threshold)||ImageIn->pSobelPixel[indice+w].mag>=(int)(high_threshold) ||ImageIn->pSobelPixel[indice+w-1].mag>=(int)(high_threshold))
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
                        break;
                    case Vertical:
                        if(  0 <= ImageIn->pSobelPixel[indice-w].mag - magin||  0 <= ImageIn->pSobelPixel[indice+w].mag - magin)
                        {
                            vect_out[j] = 0;
                            continue;
                        }
                        else
                        {
                            /*if( magin < low_threshold)
                            {
                                vect_out[j] = 0;
                            }
                            else*/ if(magin < high_threshold)
                            {
                                if (ImageIn->pSobelPixel[indice-w-1].mag>=(int)(high_threshold)||ImageIn->pSobelPixel[indice-w+1].mag>=(int)(high_threshold) ||ImageIn->pSobelPixel[indice-1].mag>=(int)(high_threshold) ||ImageIn->pSobelPixel[indice+1].mag>=(int)(high_threshold) ||ImageIn->pSobelPixel[indice+w+1].mag>=(int)(high_threshold)||ImageIn->pSobelPixel[indice+w-1].mag>=(int)(high_threshold))
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
                        break;
                    case Diagonal_lr:
                        if( magin <= ImageIn->pSobelPixel[indice-w-1].mag || magin <= ImageIn->pSobelPixel[indice+w+1].mag)
                        {
                            vect_out[j] = 0;
                            continue;
                        }
                        else
                        {
                            /*if( magin < low_threshold)
                            {
                                vect_out[j] = 0;
                                continue;
                            }
                            else*/ if(magin < high_threshold)
                            {
                                if (ImageIn->pSobelPixel[indice-w].mag>=(int)(high_threshold) ||ImageIn->pSobelPixel[indice-w+1].mag>=(int)(high_threshold) ||ImageIn->pSobelPixel[indice-1].mag>=(int)(high_threshold) ||ImageIn->pSobelPixel[indice+1].mag>=(int)(high_threshold) ||ImageIn->pSobelPixel[indice+w].mag>=(int)(high_threshold) ||ImageIn->pSobelPixel[indice+w-1].mag>=(int)(high_threshold))
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
                        break;
                    default:
                        break;
                    }
                }
                else
                {
                    vect_out[j] = 0;
                    continue;
                }
            }
            vst1q_s16(&ImageOut->pData[x*ImageIn->numCols+y], vect_out);
        }
        //the tail is entirely done in scalar 
        for( int y = ImageIn->numCols &0xfff8; y < ImageIn->numCols; y++)
        {
            int indice = x*ImageIn->numCols+y;
            int w = ImageIn->numCols;
            //first test, in particular for the border and in general for other values, if the magnitude is inferiro to the low threshold, we won't keep it
            if(ImageIn->pSobelPixel[indice].mag > low_threshold/*!=0*/)
            {
                int valuein = ImageIn->pSobelPixel[indice].mag;
                //Here is a disjonction of case, 
                //depending on the angle we have, we compare the value of the magnitude withe the two value in a 3x3 square on the same line
                //then depending on this result, we compare the value of the magnitude with the threshold values
                //one threshold act for ignoring, another for keeping and a last imply to check on the 3x3 square if the pixel is an edge or not to see if we keep it or not
                switch (ImageIn->pSobelPixel[indice].angle)
                {
                case Horizontal:
                    if( valuein <= ImageIn->pSobelPixel[indice-1].mag || valuein <= ImageIn->pSobelPixel[indice+1].mag)
                    {
                        ImageOut->pData[indice] = 0;
                    }
                    else
                    {
                        /*if( ImageIn->pSobelPixel[indice].mag < low_threshold)
                        {
                            ImageOut->pData[indice] = 0;
                        }
                        else*/ if(ImageIn->pSobelPixel[indice].mag < high_threshold)
                        {
                            if (ImageIn->pSobelPixel[indice-w-1].mag>=(int)(high_threshold) ||ImageIn->pSobelPixel[indice-w].mag>=(int)(high_threshold) ||ImageIn->pSobelPixel[indice-w+1].mag>=(int)(high_threshold)||ImageIn->pSobelPixel[indice+w+1].mag>=(int)(high_threshold) ||ImageIn->pSobelPixel[indice+w].mag>=(int)(high_threshold) ||ImageIn->pSobelPixel[indice+w-1].mag>=(int)(high_threshold))
                            {
                                ImageOut->pData[indice] = 255;
                            }
                            else
                            {
                                ImageOut->pData[indice] = 0;
                            }
                        }
                        else
                        {
                            ImageOut->pData[indice] = 255;
                        }
                    }
                    break;
                case Diagonal_rl:
                    if( valuein <= ImageIn->pSobelPixel[indice+w-1].mag || valuein <= ImageIn->pSobelPixel[indice-w+1].mag)
                    {
                        ImageOut->pData[indice] = 0;
                    }
                    else
                    {
                        /*if( ImageIn->pSobelPixel[indice].mag < low_threshold)
                        {
                            ImageOut->pData[indice] = 0;
                        }
                        else*/ if(ImageIn->pSobelPixel[indice].mag < high_threshold)
                        {
                            if (ImageIn->pSobelPixel[indice-w-1].mag>=(int)(high_threshold) ||ImageIn->pSobelPixel[indice-w].mag>=(int)(high_threshold)||ImageIn->pSobelPixel[indice-1].mag>=(int)(high_threshold) ||ImageIn->pSobelPixel[indice+1].mag>=(int)(high_threshold)||ImageIn->pSobelPixel[indice+w].mag>=(int)(high_threshold) ||ImageIn->pSobelPixel[indice+w-1].mag>=(int)(high_threshold))
                            {
                                ImageOut->pData[indice] = 255;
                            }
                            else
                            {
                                ImageOut->pData[indice] = 0;
                            }
                        }
                        else
                        {
                            ImageOut->pData[indice] = 255;
                        }
                    }
                    break;
                case Vertical:
                    if(  0 <= ImageIn->pSobelPixel[indice-w].mag - valuein||  0 <= ImageIn->pSobelPixel[indice+w].mag - valuein)
                    {
                        ImageOut->pData[indice] = 0;
                    }
                    else
                    {
                        /*if( ImageIn->pSobelPixel[indice].mag < low_threshold)
                        {
                            ImageOut->pData[indice] = 0;
                        }
                        else*/ if(ImageIn->pSobelPixel[indice].mag < high_threshold)
                        {
                            if (ImageIn->pSobelPixel[indice-w-1].mag>=(int)(high_threshold)||ImageIn->pSobelPixel[indice-w+1].mag>=(int)(high_threshold) ||ImageIn->pSobelPixel[indice-1].mag>=(int)(high_threshold) ||ImageIn->pSobelPixel[indice+1].mag>=(int)(high_threshold) ||ImageIn->pSobelPixel[indice+w+1].mag>=(int)(high_threshold)||ImageIn->pSobelPixel[indice+w-1].mag>=(int)(high_threshold))
                            {
                                ImageOut->pData[indice] = 255;
                            }
                            else
                            {
                                ImageOut->pData[indice] = 0;
                            }
                        }
                        else
                        {
                            ImageOut->pData[indice] = 255;
                        }
                    }
                    break;
                case Diagonal_lr:
                    if( valuein <= ImageIn->pSobelPixel[indice-w-1].mag || valuein <= ImageIn->pSobelPixel[indice+w+1].mag)
                    {
                        ImageOut->pData[indice] = 0;
                    }
                    else
                    {
                        /*if( ImageIn->pSobelPixel[indice].mag < low_threshold)//define a macro for this could be usefull
                        {
                            ImageOut->pData[indice] = 0;
                        }
                        else*/ if(ImageIn->pSobelPixel[indice].mag < high_threshold)
                        {
                            if (ImageIn->pSobelPixel[indice-w].mag>=(int)(high_threshold) ||ImageIn->pSobelPixel[indice-w+1].mag>=(int)(high_threshold) ||ImageIn->pSobelPixel[indice-1].mag>=(int)(high_threshold) ||ImageIn->pSobelPixel[indice+1].mag>=(int)(high_threshold) ||ImageIn->pSobelPixel[indice+w].mag>=(int)(high_threshold) ||ImageIn->pSobelPixel[indice+w-1].mag>=(int)(high_threshold))
                            {
                                ImageOut->pData[indice] = 255;
                            }
                            else
                            {
                                ImageOut->pData[indice] = 0;
                            }
                        }
                        else
                        {
                            ImageOut->pData[indice] = 255;
                        }
                    }
                    break;
                default:
                    break;
                }
            }
            else
            {
                ImageOut->pData[indice] = 0;
            }
        }
    }
}
#endif