#ifndef _MV_H_
#define _MV_H_

#include "arm_math.h"
#define IM_MAX_GS            (255)

typedef struct gvec {
    uint16_t t;
    uint16_t g;
} gvec_t;

typedef struct rectangle {
    int16_t x;
    int16_t y;
    int16_t w;
    int16_t h;
} rectangle_t;


#endif