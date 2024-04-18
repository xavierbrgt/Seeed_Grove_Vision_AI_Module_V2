#ifndef _CANNY_EDGE_H_
#define _CANNY_EDGE_H_
#include "arm_math.h"
#include "arm_type_cvl.h"

#ifdef   __cplusplus
extern "C"
{
#endif

/**
 * @brief        Thining operation in q15 on a sobel image outputing a classification of the pixels for output
 * @param[in]    ImageIn    Pointer to the structure of the input image, a sobel image with two information for each pixel angle and magnitude
 * @param[in]    ImageOut   Pointer to the structure of the ouput image, containing a classification of the pixel for each pixel
 * @return 
 *
 */
extern void arm_thining_angle_in_out_q15_u8_proc_q15(const arm_image_sobel_q15_t* ImageIn, arm_image_u8_t* ImageWeakStrong);
/**
 * @brief        Hysteresis operation in q15 operating on a sobel image and a classification in order to found the canny edge image
 * @param[in]    ImageIn    Pointer to the structure of the input image, a sobel image containing angle and magnitude for each pixel
 * @param[in]    ImageOut   Pointer to the structure of the ouput image
 * @param[in]    ImageWeakStrong    Pointer to the structure of the second input image containing the classification of the pixels
 * @return 
 *
 */
extern void arm_hysteresis_in_q15_out_q15_proc_q15(const arm_image_sobel_q15_t* ImageIn, arm_image_gray_q15_t* ImageOut, arm_image_u8_t* ImageWeakStrong);

/**
 * @brief        Thining operation in float16 on a sobel image outputing a classification of the pixels for output
 * @param[in]    ImageIn    Pointer to the structure of the input image, a sobel image with two information for each pixel angle and magnitude
 * @param[in]    ImageOut   Pointer to the structure of the ouput image, containing a classification of the pixel for each pixel
 * @return 
 *
 */
extern void arm_thining_angle_in_f16_out_u8_proc_f16(const arm_image_sobel_f16_t* ImageIn, arm_image_u8_t* ImageWeakStrong);
/**
 * @brief        Hysteresis operation in float16 operating on a sobel image and a classification in order to found the canny edge image
 * @param[in]    ImageIn    Pointer to the structure of the input image, a sobel image containing angle and magnitude for each pixel
 * @param[in]    ImageOut   Pointer to the structure of the ouput image
 * @param[in]    ImageWeakStrong    Pointer to the structure of the second input image containing the classification of the pixels
 * @return 
 *
 */
extern void arm_hysteresis_in_f16_out_f16_proc_f16(const arm_image_sobel_f16_t* ImageIn, arm_image_gray_u8_t* ImageOut, arm_image_u8_t* ImageWeakStrong);

/**
 * @brief        Merge function of hysteresis and thining, operationg on q15
 * @param[in]    ImageIn    Pointer to the structure of the input image, a sobel image containing angle and magnitude for each pixel
 * @param[in]    ImageOut   Pointer to the structure of the ouput image
 * @return 
 *
 */
extern void arm_canny_edge_in_out_q15_u8_proc_q15(const arm_image_sobel_q15_t* ImageIn, arm_image_gray_q15_t* ImageOut);
/**
 * @brief        Merge function of hysteresis and thining, operationg on float16
 * @param[in]    ImageIn    Pointer to the structure of the input image, a sobel image containing angle and magnitude for each pixel
 * @param[in]    ImageOut   Pointer to the structure of the ouput image
 * @return 
 *
 */
extern void arm_canny_edge_in_f16_out_u8_proc_f16(const arm_image_sobel_f16_t* ImageIn, arm_image_gray_u8_t* ImageOut);

/**
 * @brief        Function combining the sobel operation and the canny edge last steps in a tentative to increase the speed
 * @param[in]    ImageIn    Pointer to the structure of the input image, a sobel image containing angle and magnitude for each pixel
 * @param[in]    ImageOut   Pointer to the structure of the ouput image
 * @param[in]    Img_tmp_temporary  Pointer to the structure of the first buffer used in the function storing elements in q15
 * @param[in]    Img_tmp_grad       Pointer to the structure of the second buffer used for storing gradient, with two component for each element inq15
 * @param[in]    Img_tmp_mag        Pointer to the structure of the third buffer used for storing magnitude, with on component on each element in q15
 * @return 
 *
 */
extern void arm_canny_edge_sobel_in_q15_out_u8_proc_q15(const arm_image_gray_q15_t* ImageIn, 
                                                 arm_image_gray_q15_t* ImageOut, 
                                                 arm_buffer_2_q15_t* Img_tmp_grad/*4*/, 
                                                 arm_image_gray_q15_t* Img_tmp_mag/*3*/, 
                                                 arm_buffer_2_q15_t* Img_tmp_temporary/*3*/);

#ifdef   __cplusplus
}
#endif

#endif
