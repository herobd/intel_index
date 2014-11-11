#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <limits>  
#define INT_POS_INFINITY std::numeric_limits<int>::max()
#define INT_NEG_INFINITY std::numeric_limits<int>::min()
#define DOUBLE_POS_INFINITY std::numeric_limits<double>::max()
#define DOUBLE_NEG_INFINITY std::numeric_limits<double>::min()

#define BLACK 0
#define WHITE 255

#define SPLIT_HORZ 1
#define SPLIT_VERT 2
#define CHOP_TOP 3

#define PI      3.14159265358
#define HALF_PI 1.57079632679
#define SQRT_2  1.41421356237

#define SLOPE_DIF_TOLERANCE .25//.35 //bins, + or -

#define ANCHOR_L 2.4
#define ANCHOR_R 2.4
#define ANCHOR_MICRO 1.8
#define NEW_ANCHOR 80

#define INV_A 100

inline int mod(int a, int b)
{
    while (a<0)
        a+=b;
    return a%b;
}

#define WRITE_SCORES 0
#define USE_3D_CUT 1
#define VERT_BIAS_3D_CUT 0
#define USE_DESC_LOOP_FINDER 0

#define SHOW_VIZ 0
#define SAVE_SUBSECTION 0
#define HORZ_SAVE_INV_DIST_MAP 1
#define NORM_SAVE_ANCHOR 1

#endif // CONSTANTS_H
