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

#define INV_A 100

inline int mod(int a, int b)
{
    while (a<0)
        a+=b;
    return a%b;
}

#endif // CONSTANTS_H
