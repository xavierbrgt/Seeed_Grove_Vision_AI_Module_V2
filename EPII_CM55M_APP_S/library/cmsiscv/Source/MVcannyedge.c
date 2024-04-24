/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2013-2021 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2013-2021 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * Edge Detection.
 */
#include <stdio.h>
#include <math.h>
#include <string.h>
//#include "imlib.h"
//#include "fb_alloc.h"
#include "MV.h"


int8_t kernel_gauss_3[9] = {  1, 2, 1,
                                    2, 4, 2,
                                    1, 2, 1};

#define PIXFORMAT_STRUCT            \
struct {                            \
  union {                           \
    struct {                        \
        uint32_t /*reserved*/   :3; \
        uint32_t is_yuv         :1; \
        uint32_t is_mutable     :1; \
        uint32_t is_color       :1; \
        uint32_t is_compressed  :1; \
        uint32_t is_bayer       :1; \
        uint32_t pixfmt_id      :8; \
        uint32_t subfmt_id      :8; \
        uint32_t bpp            :8; \
    };                              \
    uint32_t pixfmt;                \
  };                                \
  uint32_t size; /* for compressed images */ \
}
typedef struct image {
    int32_t w;
    int32_t h;
    PIXFORMAT_STRUCT;
    union {
        uint8_t *pixels;
        uint8_t *data;
    };
} image_t;

#define M_PI      3.14159265358979323846
/*void imlib_edge_simple(image_t *src, rectangle_t *roi, int low_thresh, int high_thresh) {
    imlib_morph(src, 1, kernel_high_pass_3, 1.0f, 0.0f, false, 0, false, NULL);
    list_t thresholds;
    list_init(&thresholds, sizeof(color_thresholds_list_lnk_data_t));
    color_thresholds_list_lnk_data_t lnk_data;
    lnk_data.LMin = low_thresh;
    lnk_data.LMax = high_thresh;
    list_push_back(&thresholds, &lnk_data);
    imlib_binary(src, src, &thresholds, false, false, NULL);
    list_free(&thresholds);
    imlib_erode(src, 1, 2, NULL);
}*/

void imlib_sepconv3(arm_matrix_instance_q15* img, const int8_t *krn, const float m, const int b, int *buffer) {
    int ksize = 3;
    // TODO: Support RGB
    //int *buffer = fb_alloc(img->numCols * sizeof(*buffer) * 2, FB_ALLOC_NO_HINT);

    // NOTE: This doesn't deal with borders right now. Adding if
    // statements in the inner loop will slow it down significantly.
    for (int y = 0; y < img->numRows - ksize; y++) {
        for (int x = 0; x < img->numCols; x++) {
            int acc = 0;
            //if (IM_X_INSIDE(img, x+k) && IM_Y_INSIDE(img, y+j))
            acc = __SMLAD(krn[0], img->pData[(y+0)*img->numCols + x], acc);
            acc = __SMLAD(krn[1], img->pData[(y+1)*img->numCols + x], acc);
            acc = __SMLAD(krn[2], img->pData[(y+2)*img->numCols + x], acc);//
            buffer[((y % 2) * img->numCols) + x] = acc;
        }
        if (y > 0) {
            // flush buffer
            for (int x = 0; x < img->numCols - ksize; x++) {
                int acc = 0;
                acc = __SMLAD(krn[0], buffer[((y - 1) % 2) * img->numCols + x + 0], acc);
                acc = __SMLAD(krn[1], buffer[((y - 1) % 2) * img->numCols + x + 1], acc);
                acc = __SMLAD(krn[2], buffer[((y - 1) % 2) * img->numCols + x + 2], acc);
                acc = (acc * m) + b; // scale, offset, and clamp
                if(acc<IM_MAX_GS)
                {
                    if(0>acc)
                    {
                        acc = 0;
                    }
                }
                else
                {
                    acc = IM_MAX_GS;
                }
                //acc = IM_MAX(IM_MIN(acc, IM_MAX_GS), 0);
                //IM_SET_GS_PIXEL(img, (x + 1), (y), acc);
                img->pData[(y)*img->numCols + (x+1)] = acc;
            }
        }
    }
    //fb_free();
}


void imlib_edge_canny(arm_matrix_instance_q15* src, rectangle_t *roi, int low_thresh, int high_thresh, gvec_t *gm, int* buffer) {
    int w = src->numCols;

    //gvec_t *gm = fb_alloc0(roi->numCols * roi->numRows * sizeof *gm, FB_ALLOC_NO_HINT);

    //1. Noise Reduction with a Gaussian filter
    imlib_sepconv3(src, &kernel_gauss_3[0], 1.0f / 16.0f, 0.0f, buffer);

    //2. Finding Image Gradients
    for (int gy = 1, y = roi->y + 1; y < roi->y + roi->h - 1; y++, gy++) {
        for (int gx = 1, x = roi->x + 1; x < roi->x + roi->w - 1; x++, gx++) {
            int vx = 0, vy = 0;
            // sobel kernel in the horizontal direction
            vx = src->pData [(y - 1) * w + x - 1]
                 - src->pData [(y - 1) * w + x + 1]
                 + (src->pData[(y + 0) * w + x - 1] << 1)
                 - (src->pData[(y + 0) * w + x + 1] << 1)
                 + src->pData [(y + 1) * w + x - 1]
                 - src->pData [(y + 1) * w + x + 1];

            // sobel kernel in the vertical direction
            vy = src->pData [(y - 1) * w + x - 1]
                 + (src->pData[(y - 1) * w + x + 0] << 1)
                 + src->pData [(y - 1) * w + x + 1]
                 - src->pData [(y + 1) * w + x - 1]
                 - (src->pData[(y + 1) * w + x + 0] << 1)
                 - src->pData [(y + 1) * w + x + 1];

            // Find magnitude
            int g = (int) sqrtf(vx * vx + vy * vy);
            // Find the direction and round angle to 0, 45, 90 or 135
            int t = (int) fabs((atan2(vy, vx) * 180.0 / M_PI));
            /*#ifdef PRINTS
            if(gx == 173 && gy == 44)
            {
                printf("IMLIB g %d, t %d\n", g, t);
            }
            #endif*/
            /*if(gx == 218 && gy == 37)
            {
                printf("IMLIB y %d, x %d, magnin %d, anglein %d, vx %d, vy %d", gy, gx, g, t, vx, vy);
            }*/
            if (t < 22) {
                t = 0;
            } else if (t < 67) {
                t = 45;
            } else if (t < 112) {
                t = 90;
            } else if (t < 160) {
                t = 135;
            } else if (t <= 180) {
                t = 0;
            }

            gm[gy * roi->w + gx].t = t;
            gm[gy * roi->w + gx].g = g;
            /*#ifdef PRINTS
            if(gx == 284 && gy == 1)
            {
                printf("IMLIB y %d, x %d, magnin %d, anglein %d\n", gy, gx, gm[gy * roi->w + gx].g, gm[gy * roi->w + gx].t);
            }
            #endif*/
        }
    }
    /*#ifdef PRINTS
    printf("IMLIN MAG %d, angle %d\n", gm[44 * roi->w + 173].t , gm[44 * roi->w + 173].g);
    #endif*/
    // 3. Hysteresis Thresholding
    // 4. Non-maximum Suppression and output
    for (int gy = 0, y = roi->y; y < roi->y + roi->h; y++, gy++) {
        for (int gx = 0, x = roi->x; x < roi->x + roi->w; x++, gx++) {
            int i = y * w + x;
            gvec_t *va = NULL, *vb = NULL, *vc = &gm[gy * roi->w + gx];

            // Clear the borders
            if (y == (roi->y) || y == (roi->y + roi->h - 1) ||
                x == (roi->x) || x == (roi->x + roi->w - 1)) {
                src->pData[i] = 0;
                continue;
            }

            if (vc->g < low_thresh) {
                // Not an edge
                src->pData[i] = 0;
                continue;
                // Check if strong or weak edge
            } else if (vc->g >= high_thresh ||
                       gm[(gy - 1) * roi->w + (gx - 1)].g >= high_thresh ||
                       gm[(gy - 1) * roi->w + (gx + 0)].g >= high_thresh ||
                       gm[(gy - 1) * roi->w + (gx + 1)].g >= high_thresh ||
                       gm[(gy + 0) * roi->w + (gx - 1)].g >= high_thresh ||
                       gm[(gy + 0) * roi->w + (gx + 1)].g >= high_thresh ||
                       gm[(gy + 1) * roi->w + (gx - 1)].g >= high_thresh ||
                       gm[(gy + 1) * roi->w + (gx + 0)].g >= high_thresh ||
                       gm[(gy + 1) * roi->w + (gx + 1)].g >= high_thresh) {
                        /*#ifdef PRINTS
                        if(gx == 163 && gy == 17)
            {
                printf(" %d, %d, %d, %d, %d, %d, %d, %d", gm[(gy - 1) * roi->w + (gx - 1)].g  ,
                       gm[(gy - 1) * roi->w + (gx + 0)].g  ,
                       gm[(gy - 1) * roi->w + (gx + 1)].g  ,
                       gm[(gy + 0) * roi->w + (gx - 1)].g  ,
                       gm[(gy + 0) * roi->w + (gx + 1)].g  ,
                       gm[(gy + 1) * roi->w + (gx - 1)].g  ,
                       gm[(gy + 1) * roi->w + (gx + 0)].g  ,
                       gm[(gy + 1) * roi->w + (gx + 1)].g );
            }
            #endif*/
                vc->g = vc->g;
            } else {
                // Not an edge
                src->pData[i] = 0;
                continue;
            }

            switch (vc->t) {
                case 0: {
                    va = &gm[(gy + 0) * roi->w + (gx - 1)];
                    vb = &gm[(gy + 0) * roi->w + (gx + 1)];
                    break;
                }

                case 45: {
                    va = &gm[(gy + 1) * roi->w + (gx - 1)];
                    vb = &gm[(gy - 1) * roi->w + (gx + 1)];
                    break;
                }

                case 90: {
                    va = &gm[(gy + 1) * roi->w + (gx + 0)];
                    vb = &gm[(gy - 1) * roi->w + (gx + 0)];
                    break;
                }

                case 135: {
                    va = &gm[(gy + 1) * roi->w + (gx + 1)];
                    vb = &gm[(gy - 1) * roi->w + (gx - 1)];
                    break;
                }
            }
            /*#ifdef PRINTS
            if(gx == 218 && gy == 37)
            {
                printf("\nIMLIB va %d, vb %d, vc->g %d\n", va->g, vb->g, vc->g);
            }
            #endif*/
            if (!(vc->g > va->g && vc->g > vb->g)) {
                src->pData[i] = 0;
            } else {
                src->pData[i] = 0x7FFF;
            }
        }
    }

    //fb_free();
}

void imlib_sobel(arm_matrix_instance_q15* src, rectangle_t *roi, gvec_t *gm)
{
    int w = src->numCols;

    for (int gy = 1, y = roi->y + 1; y < roi->y + roi->h - 1; y++, gy++) {
        for (int gx = 1, x = roi->x + 1; x < roi->x + roi->w - 1; x++, gx++) {
            int vx = 0, vy = 0;
            // sobel kernel in the horizontal direction
            vx = src->pData [(y - 1) * w + x - 1]
                 - src->pData [(y - 1) * w + x + 1]
                 + (src->pData[(y + 0) * w + x - 1] << 1)
                 - (src->pData[(y + 0) * w + x + 1] << 1)
                 + src->pData [(y + 1) * w + x - 1]
                 - src->pData [(y + 1) * w + x + 1];
            // sobel kernel in the vertical direction
            vy = src->pData [(y - 1) * w + x - 1]
                 + (src->pData[(y - 1) * w + x + 0] << 1)
                 + src->pData [(y - 1) * w + x + 1]
                 - src->pData [(y + 1) * w + x - 1]
                 - (src->pData[(y + 1) * w + x + 0] << 1)
                 - src->pData [(y + 1) * w + x + 1];
                 /*#ifdef PRINTS
                 if(gy ==1 &&gx==1)
                 {
                    printf("gradx IMLIB %d, y %d\n", vx, vy);
                 }
                 #endif*/
            // Find magnitude
            int g = (int) sqrtf(vx * vx + vy * vy);
            // Find the direction and round angle to 0, 45, 90 or 135
            int t = (int) fabs((atan2(vy, vx) * 180.0 / M_PI));
            if (t < 22) {
                t = 0;
            } else if (t < 67) {
                t = 45;
            } else if (t < 112) {
                t = 90;
            } else if (t < 160) {
                t = 135;
            } else if (t <= 180) {
                t = 0;
            }
            /*if(gy==15&& gx==174)
            {
                printf("T == %d, %d, vx %d, vy %d, g %d\n", t, (int) fabs((atan2(vy, vx) * 180.0 / M_PI)), vx, vy, g);
            }*/
            /*if(gy == 3 &&(gx==5||gx==4||gx == 3))
            {
                printf("x = %d, y = %d, gradx = %d, grady = %d, g = %d\n", gy, gx, vx, vy, g);
                for(int x =0; x<3; x++)
                {
                    for(int y = 0; y<3; y++)
                    {
                        printf("value in %d", src->pData[(x+1)*w+y+gx-1]);
                    }
                    printf("\n");
                }
            }*/
            gm[gy * roi->w + gx].t = t;
            gm[gy * roi->w + gx].g = g;
            
        }
    }
    /*for(int x =0; x<3; x++)
    {
        for(int y = 0; y<320; y++)
        {
            printf("%d\t", gm[x*w+y].t);
        }
        printf("\n");
    }*/
    for (int gy = 0, y = roi->y; y < roi->y + roi->h; y++, gy++) {
        for (int gx = 0, x = roi->x; x < roi->x + roi->w; x++, gx++) {
            if (y == (roi->y) || y == (roi->y + roi->h - 1) ||
                x == (roi->x) || x == (roi->x + roi->w - 1)) {
                src->pData[gy * roi->w + gx] = 0;
                continue;
            }
            src->pData [gy * roi->w + gx] = gm[gy * roi->w + gx].g;
            /*if(gy == 74 && gx == 235)
            {
                printf("Imlib value out %d\n", src->pData [gy * roi->w + gx]);
            }*/
        }
    }
}