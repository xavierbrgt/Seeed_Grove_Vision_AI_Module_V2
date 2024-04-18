#ifndef _ARM_TYPE_CVL_
#define _ARM_TYPE_CVL_
#if !defined(ARM_MATH_MVEI)
typedef float float16_t;
#endif
//inside API

//arm_image_u8
#define  Strong  2
#define  Weak  1
#define  Low 0

#define Horizontal    0
#define Diagonal_rl 1
#define Vertical  2
#define Diagonal_lr 3

/**
 * @brief   Structure for an sobel image grayscale in u8 
 * @param   numCols     number of columns in the image
 * @param   numRows     number of rows in the image
 * @param   pData       pointeur to the array containing the data of the pixels
 * @return 
 *
 */
typedef struct arm_image_u8 {
  uint8_t* pData;
  uint16_t numCols;
  uint16_t numRows;
} arm_image_u8_t;

/**
 * @brief   Structure for a pixel of a sobel image q15
 * @param   mag     magnitude attibuted to the pixel
 * @param   angle   angle antributed to the pixel
 * @return 
 *
 */
typedef struct pixel_sobel_q15 {
  q15_t mag;
  q15_t angle;//no interest to store the angle in fixed point since it can only take 4 values { 0, 45, 90, 135 } but to allow to use Helium we keep the q15 type
} pixel_sobel_q15_t;
/**
 * @brief   Structure for an sobel image in q15 
 * @param   numCols     number of columns in the image
 * @param   numRows     number of rows in the image
 * @param   pSobelPixel pointeur to the array containing the data of the pixels
 * @return 
 *
 */
typedef struct arm_image_sobel_q15 {
  pixel_sobel_q15_t* pSobelPixel;
  uint16_t numCols;
  uint16_t numRows;
} arm_image_sobel_q15_t;
/**
 * @brief   Structure for a pixel of a sobel image float16
 * @param   mag     magnitude attibuted to the pixel
 * @param   angle   angle antributed to the pixel
 * @return 
 *
 */
typedef struct pixel_sobel_f16 {
  float16_t mag;
  float16_t angle;//same reflexion as befor the angle can only take four value but in order to use Helium we keep the float 16
} pixel_sobel_f16_t;
/**
 * @brief   Structure for an sobel image in float16 
 * @param   numCols     number of columns in the image
 * @param   numRows     number of rows in the image
 * @param   pSobelPixel pointeur to the array containing the data of the pixels
 * @return 
 *
 */
typedef struct arm_image_sobel_f16 {
  pixel_sobel_f16_t* pSobelPixel;
  uint16_t numCols;
  uint16_t numRows;
} arm_image_sobel_f16_t;
/**
 * @brief   Structure for a grayscale image in u8 
 * @param   numCols     number of columns in the image
 * @param   numRows     number of rows in the image
 * @param   pData       pointeur to the array containing the data of the pixels
 * @return 
 *
 */
typedef struct arm_image_gray_u8 {
  uint8_t* pData;
  uint16_t numCols;
  uint16_t numRows;
} arm_image_gray_u8_t;
  /**
 * @brief   Structure for an sobel image in float16 
 * @param   numCols     number of columns in the image
 * @param   numRows     number of rows in the image
 * @param   pData       pointeur to the array containing the data of the pixels
 * @return 
 *
 */
typedef struct arm_image_gray_q15 {
  q15_t* pData;
  uint16_t numCols;
  uint16_t numRows;
} arm_image_gray_q15_t;
/**
 * @brief   Structure for an sobel image in float16 
 * @param   numCols     number of columns in the image
 * @param   numRows     number of rows in the image
 * @param   pData       pointeur to the array containing the data of the pixels
 * @return 
 *
 */
typedef struct arm_image_buff_f16 {
  float16_t* pData;
  uint16_t numCols;
  uint16_t numRows;
} arm_image_buff_f16_t;

/**
 * @brief   Structure for a pixel of a buffer with two composant in u16
 * @param   x     first composant of the pixel
 * @param   y     second composant of the pixel
 * @return 
 *
 */
typedef struct divergence {
uint16_t x;
uint16_t y;
} divergence_t;
/**
 * @brief   Structure for an sobel image in float16 
 * @param   numCols     number of columns in the image
 * @param   numRows     number of rows in the image
 * @param   pData       pointeur to the array containing the data of the pixels
 * @return 
 *
 */
typedef struct arm_buffer_2_u6 {
    divergence_t* pData;
    int16_t numCols;
    int16_t numRows;
} arm_buffer_2_u6_t;

/**
 * @brief   Structure for a pixel of a buffer with two composant in q15
 * @param   x     first composant of the pixel
 * @param   y     second composant of the pixel
 * @return 
 *
 */
typedef struct divergence_q15 {
  q15_t x;
  q15_t y;
} divergence_q15_t;
/**
 * @brief   Structure for an sobel image in float16 
 * @param   numCols     number of columns in the image
 * @param   numRows     number of rows in the image
 * @param   pData       pointeur to the array containing the data of the pixels
 * @return 
 *
 */
typedef struct arm_buffer_2_q15 {
  divergence_q15_t* pData;
  int16_t numCols;
  int16_t numRows;
} arm_buffer_2_q15_t;

/**
 * @brief   Structure for a pixel of a buffer with two composant in q31
 * @param   x     first composant of the pixel
 * @param   y     second composant of the pixel
 * @return 
 *
 */
typedef struct divergence_q31{
  q31_t x;
  q31_t y;
} divergence_q31_t;
/**
 * @brief   Structure for an sobel image in float16 
 * @param   numCols     number of columns in the image
 * @param   numRows     number of rows in the image
 * @param   pData       pointeur to the array containing the data of the pixels
 * @return 
 *
 */
typedef struct arm_buffer_2_q31{
  divergence_q31_t* pData;
  int16_t numCols;
  int16_t numRows;
} arm_buffer_2_q31_t;
#endif
