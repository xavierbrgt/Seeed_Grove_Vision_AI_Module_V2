#ifndef _SOBEL_H_
#define _SOBEL_H_

#include "arm_math.h"
#include "arm_type_cvl.h"

#ifdef   __cplusplus
extern "C"
{
#endif

/**
 * @brief        Sobel filter for grayscale data using a buffer to store intermediate values computing in float16
 * @param[in]    ImageIn    Pointer to the structure of the input image
 * @param[in]    ImageOut   Pointer to the structure of the ouput image
 * @return 
 *
 */
extern void arm_sobel_in_q15_out_q15_proc_q15(const arm_image_gray_q15_t* ImageIn, arm_image_sobel_q15_t* ImageOut);

/**
 * @brief        Sobel filter for grayscale data using a buffer to store intermediate values computing in q15
 * @param[in]    ImageIn    Pointer to the structure of the input image
 * @param[in]    ImageOut   Pointer to the structure of the ouput image
 * @return 
 *
 */
extern void arm_sobel_in_u8_out_f16_proc_f16(const arm_image_gray_u8_t* ImageIn, arm_image_sobel_f16_t* ImageOut);

/**
 * @brief        Sobel filter for grayscale data using a buffer to store intermediate values computing in float16
 * @param[in]    ImageIn    Pointer to the structure of the input image
 * @param[in]    ImageOut   Pointer to the structure of the ouput image
 * @param[in]    Img_tmp    Pointer to the temporary buffer used in the function, size must be 3* Number Pixel on a Line
 * @return 
 *
 */
extern void arm_sobel_in_u8_out_f16_proc_f16_buff(const arm_image_gray_u8_t* ImageIn, arm_image_sobel_f16_t* ImageOut, arm_buffer_2_u6_t* Img_tmp);

/**
 * @brief        Sobel filter for grayscale data using a buffer to store intermediate values computing in q15
 * @param[in]    ImageIn    Pointer to the structure of the input image
 * @param[in]    ImageOut   Pointer to the structure of the ouput image
 * @param[in]    Img_tmp    Pointer to the temporary buffer used in the function, size must be 3* Number Pixel on a Line
 * @return 
 *
 */
extern void arm_sobel_in_q15_out_q15_proc_q15_buff( const arm_image_gray_q15_t* ImageIn, arm_image_sobel_q15_t* ImageOut, arm_buffer_2_q15_t* Img_tmp);


#ifdef   __cplusplus
}
#endif

#endif
