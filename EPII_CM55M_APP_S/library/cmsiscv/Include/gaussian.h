#ifndef _GAUSSIAN_H_
#define _GAUSSIAN_H_

//this include contain the fixed point type
#include "arm_math.h"
//this include contain the specific types of the library
#include "arm_type_cvl.h"

#ifdef   __cplusplus
extern "C"
{
#endif


/**
 * @brief        Gaussian filter for grayscale data computing in float16
 * @param[in]    ImageIn    Pointer to the structure of the input image
 * @param[in]    ImageOut   Pointer to the structure of the ouput image
 * @return 
 *
 */
extern void arm_gaussian_filter_3x3_gray_in_u8_out_u8_proc_f16(const arm_image_gray_u8_t* ImageIn, arm_image_gray_u8_t* ImageOut);

/**
 * @brief        Gaussian filter for grayscale data computing in q15
 * @param[in]    ImageIn    Pointer to the structure of the input image
 * @param[in]    ImageOut   Pointer to the structure of the ouput image
 * @return 
 *
 */
 extern void arm_gaussian_filter_3x3_gray_in_u8_out_q15_proc_q15(const arm_image_gray_u8_t* ImageIn, arm_image_gray_q15_t* ImageOut);

/**
 * @brief        Gaussian filter for grayscale data using a buffer to store intermediate values computing in float16
 * @param[in]    ImageIn    Pointer to the structure of the input image
 * @param[in]    ImageOut   Pointer to the structure of the ouput image
 * @param[in]    Img_tmp    Pointer to the temporary buffer used in the function, size must be 3* Number Pixel on a Line
 * @return 
 *
 */
extern void arm_gaussian_filter_3x3_gray_in_u8_out_u8_proc_f16_buff(const arm_image_gray_u8_t* ImageIn, arm_image_gray_u8_t* ImageOut, arm_image_buff_f16_t* Img_tmp);

/**
 * @brief        Gaussian filter for grayscale data using a buffer to store intermediate values computing in q15
 * @param[in]    ImageIn    Pointer to the structure of the input image
 * @param[in]    ImageOut   Pointer to the structure of the ouput image
 * @param[in]    Img_tmp    Pointer to the temporary buffer used in the function, size must be 3* Number Pixel on a Line
 * @return 
 *
 */
extern void arm_gaussian_filter_3x3_gray_in_u8_out_q15_proc_q15_buff(const arm_image_gray_u8_t* ImageIn, arm_image_gray_q15_t* ImageOut, arm_image_gray_q15_t* Img_tmp);

#ifdef   __cplusplus
}
#endif


#endif
